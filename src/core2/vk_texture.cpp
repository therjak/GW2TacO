#include "src/core2/vk_texture.h"
#include <cstring>
#include "src/base/image_decompressor.h"
#include "src/base/logger.h"
#include "src/util/png_decompressor.h"

namespace renderer {

VkTexture2D::VkTexture2D(VulkanDevice* device) : Texture2D(device) {}

VkTexture2D::~VkTexture2D() {
  VulkanDevice* dev = static_cast<VulkanDevice*>(device_);
  if (!dev || dev->GetVkDevice() == VK_NULL_HANDLE) return;
  VkDevice vk_device = dev->GetVkDevice();
  VmaAllocator allocator = dev->GetAllocator();

  if (image_view_ != VK_NULL_HANDLE) {
    vkDestroyImageView(vk_device, image_view_, nullptr);
  }
  if (image_ != VK_NULL_HANDLE) {
    vmaDestroyImage(allocator, image_, image_allocation_);
  }
  if (staging_buffer_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(allocator, staging_buffer_, staging_allocation_);
  }
}

bool VkTexture2D::Create(const int32_t x_res, const int32_t y_res,
                         const uint8_t* data,
                         const char bytes_per_pixel,
                         const Format format,
                         const bool render_target) {
  VulkanDevice* dev = static_cast<VulkanDevice*>(device_);
  VkDevice vk_device = dev->GetVkDevice();
  VmaAllocator allocator = dev->GetAllocator();

  VkFormat vk_format = VK_FORMAT_B8G8R8A8_UNORM;
  if (format == Format::kR32f) {
    vk_format = VK_FORMAT_R32_SFLOAT;
  }

  VkImageCreateInfo image_info{};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent.width = x_res;
  image_info.extent.height = y_res;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.format = vk_format;
  image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if (render_target) {
    image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  if (vmaCreateImage(allocator, &image_info, &alloc_info, &image_, &image_allocation_, nullptr) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create image");
    return false;
  }

  if (data != nullptr) {
    VkDeviceSize image_size = x_res * y_res * bytes_per_pixel;
    
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = image_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    
    VkBuffer staging_buffer;
    VmaAllocation staging_allocation;
    if (vmaCreateBuffer(allocator, &buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr) != VK_SUCCESS) {
      Log_Err("[vk] Failed to create staging buffer");
      return false;
    }

    void* mapped_data;
    vmaMapMemory(allocator, staging_allocation, &mapped_data);
    memcpy(mapped_data, data, static_cast<size_t>(image_size));
    vmaUnmapMemory(allocator, staging_allocation);

    VkCommandBuffer cmd = dev->BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image_;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {static_cast<uint32_t>(x_res), static_cast<uint32_t>(y_res), 1};

    vkCmdCopyBufferToImage(cmd, staging_buffer, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    dev->EndSingleTimeCommands(cmd);

    vmaDestroyBuffer(allocator, staging_buffer, staging_allocation);
  } else {
    VkCommandBuffer cmd = dev->BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image_;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    dev->EndSingleTimeCommands(cmd);
  }

  VkImageViewCreateInfo view_info{};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image_;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = vk_format;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  if (vkCreateImageView(vk_device, &view_info, nullptr, &image_view_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create texture image view");
    return false;
  }

  x_res_ = x_res;
  y_res_ = y_res;
  return true;
}

bool VkTexture2D::Create(const uint8_t* data, int32_t size) {
  if (!data || size <= 0) return false;
  int32_t x_res = 0, y_res = 0;
  auto img = DecompressImage(data, size, &x_res, &y_res);
  if (!img) {
    if (!DecompressPNG(data, size, &img, &x_res, &y_res)) {
      return false;
    } else {
      if (!img) return false;
      ARGBtoABGR(img.get(), x_res, y_res);
    }
  }
  return Create(x_res, y_res, img.get(), 4, Format::kA8R8G8B8, false);
}

bool VkTexture2D::CreateDepthBuffer(const int32_t /*x_res*/,
                                    const int32_t /*y_res*/,
                                    const int32_t /*ms_count*/) {
  return false; // Deprecated or handled differently
}

bool VkTexture2D::Lock(void** result, int32_t& pitch) {
  VulkanDevice* dev = static_cast<VulkanDevice*>(device_);
  VmaAllocator allocator = dev->GetAllocator();

  VkDeviceSize image_size = x_res_ * y_res_ * 4;
  
  if (staging_buffer_ == VK_NULL_HANDLE) {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = image_size;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    
    VmaAllocationCreateInfo staging_alloc_info{};
    staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    
    if (vmaCreateBuffer(allocator, &buffer_info, &staging_alloc_info, &staging_buffer_, &staging_allocation_, nullptr) != VK_SUCCESS) {
      return false;
    }
  }

  vmaMapMemory(allocator, staging_allocation_, (void**)&locked_data_);
  *result = locked_data_;
  pitch = x_res_ * 4;
  locked_pitch_ = pitch;
  return true;
}

bool VkTexture2D::UnLock() {
  if (!locked_data_) return false;
  
  VulkanDevice* dev = static_cast<VulkanDevice*>(device_);
  VmaAllocator allocator = dev->GetAllocator();

  vmaUnmapMemory(allocator, staging_allocation_);
  locked_data_ = nullptr;

  VkCommandBuffer cmd = dev->BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image_;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       0, 0, nullptr, 0, nullptr, 1, &barrier);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {static_cast<uint32_t>(x_res_), static_cast<uint32_t>(y_res_), 1};

  vkCmdCopyBufferToImage(cmd, staging_buffer_, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       0, 0, nullptr, 0, nullptr, 1, &barrier);

  dev->EndSingleTimeCommands(cmd);

  return true;
}

bool VkTexture2D::Update(const uint8_t* data, const int32_t x_res,
                         const int32_t y_res,
                         const char bytes_per_pixel) {
  void* dest;
  int32_t pitch;
  if (!Lock(&dest, pitch)) return false;
  
  if (x_res == x_res_ && y_res == y_res_ && pitch == x_res * bytes_per_pixel) {
      memcpy(dest, data, x_res * y_res * bytes_per_pixel);
  } else {
      for (int i = 0; i < y_res; ++i) {
          memcpy(static_cast<uint8_t*>(dest) + i * pitch, data + i * x_res * bytes_per_pixel, x_res * bytes_per_pixel);
      }
  }
  
  return UnLock();
}

void VkTexture2D::ExportToImage(std::string_view /*filename*/,
                                bool /*clear_alpha*/,
                                ExportImageFormat /*format*/,
                                bool /*degamma*/) {}

bool VkTexture2D::SetToSampler(const Sampler sampler) {
  VulkanDevice* dev = static_cast<VulkanDevice*>(device_);
  
  uint32_t slot = 0;
  if (sampler >= Sampler::kPs0 && sampler <= Sampler::kPs15) {
      slot = static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kPs0);
  } else if (sampler >= Sampler::kVs0 && sampler <= Sampler::kVs3) {
      slot = 16 + static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kVs0);
  } else if (sampler >= Sampler::kGs0 && sampler <= Sampler::kGs3) {
      slot = 20 + static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kGs0);
  } else {
      return false;
  }

  dev->BindTexture(slot, image_view_, VK_NULL_HANDLE);
  return true;
}

}  // namespace renderer
