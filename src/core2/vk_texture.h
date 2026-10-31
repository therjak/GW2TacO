#pragma once
#include <cstdint>
#include <string_view>

#include "src/core2/texture.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkTexture2D : public Texture2D {
 public:
  explicit VkTexture2D(VulkanDevice* device);
  ~VkTexture2D() override;

  bool Create(const int32_t x_res, const int32_t y_res, const uint8_t* data,
              const char bytes_per_pixel = 4,
              const Format format = Format::kA8R8G8B8,
              const bool render_target = false) override;
  bool Create(const uint8_t* data, int32_t size) override;
  bool CreateDepthBuffer(const int32_t x_res, const int32_t y_res,
                         const int32_t ms_count) override;
  bool Lock(void** result, int32_t& pitch) override;
  bool UnLock() override;
  bool Update(const uint8_t* data, const int32_t x_res, const int32_t y_res,
              const char bytes_per_pixel = 4) override;
  void ExportToImage(std::string_view filename, bool clear_alpha,
                     ExportImageFormat format, bool degamma) override;

 private:
  bool SetToSampler(const Sampler sampler) override;

  VkImage image_ = VK_NULL_HANDLE;
  VmaAllocation image_allocation_ = VK_NULL_HANDLE;
  VkImageView image_view_ = VK_NULL_HANDLE;

  VkBuffer staging_buffer_ = VK_NULL_HANDLE;
  VmaAllocation staging_allocation_ = VK_NULL_HANDLE;
  
  uint8_t* locked_data_ = nullptr;
  int32_t locked_pitch_ = 0;
};

}  // namespace renderer
