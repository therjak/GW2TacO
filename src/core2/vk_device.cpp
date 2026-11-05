#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "src/base/logger.h"
#include "src/core2/core2_config.h"
#include "src/core2/vk_constant_buffer.h"
#include "src/core2/vk_device.h"
#include "src/core2/vk_index_buffer.h"
#include "src/core2/vk_render_state.h"
#include "src/core2/vk_shader.h"
#include "src/core2/vk_texture.h"
#include "src/core2/vk_vertex_buffer.h"
#include "src/core2/vk_vertex_format.h"

import math;

namespace renderer {

using math::Rect;

#ifdef ENABLE_CORE_DEBUG_MODE
static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* /*pUserData*/) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    Log_Warn("[vk] Validation: %s", pCallbackData->pMessage);
  } else {
    Log_Nfo("[vk] Validation: %s", pCallbackData->pMessage);
  }
  return VK_FALSE;
}
#endif

VulkanDevice::VulkanDevice() = default;

VulkanDevice::~VulkanDevice() {
  if (vk_device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(vk_device_);
  }

  for (auto& pair : pipeline_cache_) {
    vkDestroyPipeline(vk_device_, pair.second, nullptr);
  }
  pipeline_cache_.clear();

  if (pipeline_layout_ != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(vk_device_, pipeline_layout_, nullptr);
  }
  if (descriptor_pool_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(vk_device_, descriptor_pool_, nullptr);
  }
  if (descriptor_set_layout_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(vk_device_, descriptor_set_layout_, nullptr);
  }

  for (size_t i = 0; i < kMaxFramesInFlight; i++) {
    if (render_finished_semaphores_[i]) {
      vkDestroySemaphore(vk_device_, render_finished_semaphores_[i], nullptr);
    }
    if (image_available_semaphores_[i]) {
      vkDestroySemaphore(vk_device_, image_available_semaphores_[i], nullptr);
    }
    if (in_flight_fences_[i]) {
      vkDestroyFence(vk_device_, in_flight_fences_[i], nullptr);
    }
  }

  if (command_pool_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(vk_device_, command_pool_, nullptr);
  }

  CleanupSwapchain();

  if (vk_device_ != VK_NULL_HANDLE) {
    allocator_.Destroy();
    vkDestroyDevice(vk_device_, nullptr);
  }

#ifdef ENABLE_CORE_DEBUG_MODE
  if (debug_messenger_ != VK_NULL_HANDLE) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance_, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance_, debug_messenger_, nullptr);
    }
  }
#endif

  if (surface_ != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
  }

  if (instance_ != VK_NULL_HANDLE) {
    vkDestroyInstance(instance_, nullptr);
  }
}

void VulkanDevice::ResetPrivateResources() {}

bool VulkanDevice::InitAPI(const HWND window_handle, const bool /*full_screen*/,
                           const int32_t x_res, const int32_t y_res,
                           const int32_t /*sample_count*/,
                           const int32_t /*refresh_rate*/) {
  if (volkInitialize() != VK_SUCCESS) {
    Log_Err("[vk] Failed to initialize volk");
    return false;
  }

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "GW2TacO";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "CoRE2";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                         VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

  std::vector<const char*> layers;

#ifdef ENABLE_CORE_DEBUG_MODE
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  layers.push_back("VK_LAYER_KHRONOS_validation");

  create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
#else
  create_info.enabledLayerCount = 0;
#endif

  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create instance");
    return false;
  }

  volkLoadInstance(instance_);

#ifdef ENABLE_CORE_DEBUG_MODE
  VkDebugUtilsMessengerCreateInfoEXT debug_info{};
  debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debug_info.pfnUserCallback = DebugCallback;

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance_, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance_, &debug_info, nullptr, &debug_messenger_);
  }
#endif

  VkWin32SurfaceCreateInfoKHR surface_info{};
  surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_info.hwnd = window_handle;
  surface_info.hinstance = GetModuleHandle(nullptr);
  if (vkCreateWin32SurfaceKHR(instance_, &surface_info, nullptr, &surface_) !=
      VK_SUCCESS) {
    Log_Err("[vk] Failed to create window surface");
    return false;
  }

  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
  if (device_count == 0) {
    Log_Err("[vk] Failed to find GPUs with Vulkan support");
    return false;
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

  for (const auto& device : devices) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                             queue_families.data());

    uint32_t i = 0;
    uint32_t g_fam = UINT32_MAX;
    uint32_t p_fam = UINT32_MAX;
    for (const auto& queue_family : queue_families) {
      if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) g_fam = i;
      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_,
                                           &present_support);
      if (present_support) p_fam = i;
      if (g_fam != UINT32_MAX && p_fam != UINT32_MAX) break;
      i++;
    }

    if (g_fam != UINT32_MAX && p_fam != UINT32_MAX) {
      // Check extensions
      uint32_t extension_count;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                           nullptr);
      std::vector<VkExtensionProperties> available_extensions(extension_count);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                           available_extensions.data());
      bool swapchain_supported = false;
      for (const auto& ext : available_extensions) {
        if (std::string(ext.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME) {
          swapchain_supported = true;
          break;
        }
      }

      if (swapchain_supported) {
        physical_device_ = device;
        graphics_family_ = g_fam;
        present_family_ = p_fam;
        break;
      }
    }
  }

  if (physical_device_ == VK_NULL_HANDLE) {
    Log_Err("[vk] Failed to find a suitable GPU");
    return false;
  }

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set<uint32_t> unique_queue_families = {graphics_family_,
                                              present_family_};

  float queue_priority = 1.0f;
  for (uint32_t queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  device_create_info.pQueueCreateInfos = queue_create_infos.data();
  device_create_info.pEnabledFeatures = &device_features;

  const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  device_create_info.enabledExtensionCount =
      static_cast<uint32_t>(device_extensions.size());
  device_create_info.ppEnabledExtensionNames = device_extensions.data();

#ifdef ENABLE_CORE_DEBUG_MODE
  device_create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  device_create_info.ppEnabledLayerNames = layers.data();
#else
  device_create_info.enabledLayerCount = 0;
#endif

  if (vkCreateDevice(physical_device_, &device_create_info, nullptr,
                     &vk_device_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create logical device");
    return false;
  }

  volkLoadDevice(vk_device_);

  if (!allocator_.Initialize(instance_, physical_device_, vk_device_)) {
    return false;
  }

  vkGetDeviceQueue(vk_device_, graphics_family_, 0, &graphics_queue_);
  vkGetDeviceQueue(vk_device_, present_family_, 0, &present_queue_);

  if (!RecreateSwapchain(x_res, y_res)) {
    return false;
  }

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = graphics_family_;
  if (vkCreateCommandPool(vk_device_, &pool_info, nullptr, &command_pool_) !=
      VK_SUCCESS) {
    Log_Err("[vk] Failed to create command pool");
    return false;
  }

  command_buffers_.resize(kMaxFramesInFlight);
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = command_pool_;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount =
      static_cast<uint32_t>(command_buffers_.size());
  if (vkAllocateCommandBuffers(vk_device_, &alloc_info,
                               command_buffers_.data()) != VK_SUCCESS) {
    Log_Err("[vk] Failed to allocate command buffers");
    return false;
  }

  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < kMaxFramesInFlight; i++) {
    if (vkCreateSemaphore(vk_device_, &semaphore_info, nullptr,
                          &image_available_semaphores_[i]) != VK_SUCCESS ||
        vkCreateSemaphore(vk_device_, &semaphore_info, nullptr,
                          &render_finished_semaphores_[i]) != VK_SUCCESS ||
        vkCreateFence(vk_device_, &fence_info, nullptr,
                      &in_flight_fences_[i]) != VK_SUCCESS) {
      Log_Err("[vk] Failed to create sync objects");
      return false;
    }
  }

  // Descriptor Set Layout
  std::array<VkDescriptorSetLayoutBinding, 32> bindings{};
  for (uint32_t i = 0; i < 32; i++) {
    bindings[i].binding = i;
    bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    bindings[i].pImmutableSamplers = nullptr;
  }
  VkDescriptorSetLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_info.pBindings = bindings.data();
  if (vkCreateDescriptorSetLayout(vk_device_, &layout_info, nullptr,
                                  &descriptor_set_layout_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create descriptor set layout");
    return false;
  }

  // Descriptor Pool
  VkDescriptorPoolSize pool_size{};
  pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_size.descriptorCount = 32 * kMaxFramesInFlight;
  VkDescriptorPoolCreateInfo pool_info_desc{};
  pool_info_desc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info_desc.poolSizeCount = 1;
  pool_info_desc.pPoolSizes = &pool_size;
  pool_info_desc.maxSets = kMaxFramesInFlight;
  if (vkCreateDescriptorPool(vk_device_, &pool_info_desc, nullptr,
                             &descriptor_pool_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create descriptor pool");
    return false;
  }

  // Allocate Descriptor Sets
  std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight,
                                             descriptor_set_layout_);
  VkDescriptorSetAllocateInfo alloc_info_desc{};
  alloc_info_desc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info_desc.descriptorPool = descriptor_pool_;
  alloc_info_desc.descriptorSetCount = kMaxFramesInFlight;
  alloc_info_desc.pSetLayouts = layouts.data();
  if (vkAllocateDescriptorSets(vk_device_, &alloc_info_desc,
                               descriptor_sets_.data()) != VK_SUCCESS) {
    Log_Err("[vk] Failed to allocate descriptor sets");
    return false;
  }

  // Pipeline Layout
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = 128;

  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;
  if (vkCreatePipelineLayout(vk_device_, &pipeline_layout_info, nullptr,
                             &pipeline_layout_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create pipeline layout");
    return false;
  }

  if (CreateDefaultRenderStates()) {
    Log_Nfo("[vk] Vulkan default render states initialized successfully.");
  }

  initialized_ = true;
  return true;
}

bool VulkanDevice::Initialize(WindowHandler* window,
                              const int32_t sample_count) {
  window_ = window;
  return InitAPI(window_->GetHandle(),
                 window_->GetInitParameters()->full_screen_, window_->GetXRes(),
                 window_->GetYRes(), sample_count, 60);
}

bool VulkanDevice::DeviceOk() { return initialized_; }
bool VulkanDevice::IsWindowed() { return true; }

void VulkanDevice::Resize(const int32_t x_res, const int32_t y_res) {
  if (x_res != static_cast<int32_t>(swapchain_extent_.width) ||
      y_res != static_cast<int32_t>(swapchain_extent_.height)) {
    RecreateSwapchain(x_res, y_res);
  }
}

void VulkanDevice::SetFullScreenMode(const bool /*full_screen*/,
                                     const int32_t /*x_res*/,
                                     const int32_t /*y_res*/) {}

bool VulkanDevice::RecreateSwapchain(int32_t x_res, int32_t y_res) {
  if (vk_device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(vk_device_);
  }

  CleanupSwapchain();

  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_,
                                            &capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_,
                                       &format_count, formats.data());

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_,
                                            &present_mode_count, nullptr);
  std::vector<VkPresentModeKHR> present_modes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical_device_, surface_, &present_mode_count, present_modes.data());

  VkSurfaceFormatKHR surface_format = formats[0];
  bool format_found = false;
  for (const auto& available_format : formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      surface_format = available_format;
      format_found = true;
      break;
    }
  }
  if (!format_found) {
    for (const auto& available_format : formats) {
      if (available_format.format == VK_FORMAT_R8G8B8A8_UNORM &&
          available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surface_format = available_format;
        format_found = true;
        break;
      }
    }
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (const auto& available_present_mode : present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      present_mode = available_present_mode;
      break;
    }
  }

  VkExtent2D extent = {static_cast<uint32_t>(x_res),
                       static_cast<uint32_t>(y_res)};

  if (extent.width < capabilities.minImageExtent.width)
    extent.width = capabilities.minImageExtent.width;
  if (extent.width > capabilities.maxImageExtent.width)
    extent.width = capabilities.maxImageExtent.width;
  if (extent.height < capabilities.minImageExtent.height)
    extent.height = capabilities.minImageExtent.height;
  if (extent.height > capabilities.maxImageExtent.height)
    extent.height = capabilities.maxImageExtent.height;

  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      image_count > capabilities.maxImageCount) {
    image_count = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface_;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queue_family_indices[] = {graphics_family_, present_family_};
  if (graphics_family_ != present_family_) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  Log_Nfo("[vk] capabilities.supportedCompositeAlpha: 0x{:x}",
          capabilities.supportedCompositeAlpha);
  for (const auto& f : formats) {
    Log_Nfo("[vk] available format: {:d}, colorSpace: {:d}",
            static_cast<int>(f.format), static_cast<int>(f.colorSpace));
  }

  create_info.preTransform = capabilities.currentTransform;
  VkCompositeAlphaFlagBitsKHR composite_alpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  const VkCompositeAlphaFlagBitsKHR composite_alpha_flags[] = {
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
  };
  for (auto flag : composite_alpha_flags) {
    if (capabilities.supportedCompositeAlpha & flag) {
      composite_alpha = flag;
      break;
    }
  }
  create_info.compositeAlpha = composite_alpha;
  Log_Nfo("[vk] Selected swapchain format: {:d}, compositeAlpha: 0x{:x}",
          static_cast<int>(surface_format.format),
          static_cast<uint32_t>(composite_alpha));
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(vk_device_, &create_info, nullptr, &swapchain_) !=
      VK_SUCCESS) {
    Log_Err("[vk] Failed to create swapchain");
    return false;
  }

  vkGetSwapchainImagesKHR(vk_device_, swapchain_, &image_count, nullptr);
  swapchain_images_.resize(image_count);
  vkGetSwapchainImagesKHR(vk_device_, swapchain_, &image_count,
                          swapchain_images_.data());

  swapchain_format_ = surface_format.format;
  swapchain_extent_ = extent;

  swapchain_image_views_.resize(swapchain_images_.size());
  for (size_t i = 0; i < swapchain_images_.size(); i++) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = swapchain_images_[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = swapchain_format_;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vk_device_, &view_info, nullptr,
                          &swapchain_image_views_[i]) != VK_SUCCESS) {
      Log_Err("[vk] Failed to create image views");
      return false;
    }
  }

  VkAttachmentDescription color_attachment{};
  color_attachment.format = swapchain_format_;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref{};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  std::array<VkSubpassDependency, 2> dependencies{};
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = 0;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].dstAccessMask = 0;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
  render_pass_info.pDependencies = dependencies.data();

  if (vkCreateRenderPass(vk_device_, &render_pass_info, nullptr,
                         &render_pass_) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create render pass");
    return false;
  }

  framebuffers_.resize(swapchain_image_views_.size());
  for (size_t i = 0; i < swapchain_image_views_.size(); i++) {
    VkImageView attachments[] = {swapchain_image_views_[i]};

    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = render_pass_;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = swapchain_extent_.width;
    framebuffer_info.height = swapchain_extent_.height;
    framebuffer_info.layers = 1;

    if (vkCreateFramebuffer(vk_device_, &framebuffer_info, nullptr,
                            &framebuffers_[i]) != VK_SUCCESS) {
      Log_Err("[vk] Failed to create framebuffer");
      return false;
    }
  }

  images_in_flight_.resize(swapchain_images_.size(), VK_NULL_HANDLE);

  return true;
}

void VulkanDevice::CleanupSwapchain() {
  for (auto framebuffer : framebuffers_) {
    vkDestroyFramebuffer(vk_device_, framebuffer, nullptr);
  }
  framebuffers_.clear();

  if (render_pass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(vk_device_, render_pass_, nullptr);
    render_pass_ = VK_NULL_HANDLE;
  }

  for (auto image_view : swapchain_image_views_) {
    vkDestroyImageView(vk_device_, image_view, nullptr);
  }
  swapchain_image_views_.clear();

  if (swapchain_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(vk_device_, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
  }
}

//////////////////////////////////////////////////////////////////////////
// Frame lifecycle

bool VulkanDevice::BeginScene() {
  vkWaitForFences(vk_device_, 1, &in_flight_fences_[current_frame_], VK_TRUE,
                  UINT64_MAX);

  VkResult result =
      vkAcquireNextImageKHR(vk_device_, swapchain_, UINT64_MAX,
                            image_available_semaphores_[current_frame_],
                            VK_NULL_HANDLE, &current_image_index_);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapchain(swapchain_extent_.width, swapchain_extent_.height);
    return false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    Log_Err("[vk] Failed to acquire swapchain image");
    return false;
  }

  if (images_in_flight_[current_image_index_] != VK_NULL_HANDLE) {
    vkWaitForFences(vk_device_, 1, &images_in_flight_[current_image_index_],
                    VK_TRUE, UINT64_MAX);
  }
  images_in_flight_[current_image_index_] = in_flight_fences_[current_frame_];

  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(command_buffers_[current_frame_], &begin_info) !=
      VK_SUCCESS) {
    Log_Err("[vk] Failed to begin recording command buffer");
    return false;
  }

  scene_active_ = true;

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = render_pass_;
  render_pass_info.framebuffer = framebuffers_[current_image_index_];
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = swapchain_extent_;

  static bool first_clear = true;
  if (first_clear) {
    first_clear = false;
    Log_Nfo("[vk] BeginScene clear_color: R={:d} G={:d} B={:d} A={:d}",
            clear_color_.R(), clear_color_.G(), clear_color_.B(),
            clear_color_.A());
  }

  VkClearValue clear_color;
  clear_color.color = {{clear_color_.R() / 255.0f, clear_color_.G() / 255.0f,
                        clear_color_.B() / 255.0f, clear_color_.A() / 255.0f}};

  // Phase 2 only requires color attachment clear for this renderer pass.
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(command_buffers_[current_frame_], &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  if (current_viewport_.Width() > 0 && current_viewport_.Height() > 0) {
    SetViewport(current_viewport_);
  } else {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain_extent_.width);
    viewport.height = static_cast<float>(swapchain_extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffers_[current_frame_], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extent_;
    vkCmdSetScissor(command_buffers_[current_frame_], 0, 1, &scissor);
  }

  return true;
}

bool VulkanDevice::EndScene() {
  scene_active_ = false;
  vkCmdEndRenderPass(command_buffers_[current_frame_]);

  if (vkEndCommandBuffer(command_buffers_[current_frame_]) != VK_SUCCESS) {
    Log_Err("[vk] Failed to record command buffer");
    return false;
  }
  return true;
}

bool VulkanDevice::Clear(const bool clear_pixels, const bool clear_depth,
                         const Color& color, const float depth,
                         const int32_t /*stencil*/) {
  if (clear_pixels) {
    clear_color_ = color;
  }
  if (clear_depth) {
    clear_depth_ = depth;
  }
  return true;
}

bool VulkanDevice::Flip(bool vsync) {
  vkResetFences(vk_device_, 1, &in_flight_fences_[current_frame_]);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore wait_semaphores[] = {image_available_semaphores_[current_frame_]};
  VkPipelineStageFlags wait_stages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = wait_semaphores;
  submit_info.pWaitDstStageMask = wait_stages;

  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffers_[current_frame_];

  VkSemaphore signal_semaphores[] = {
      render_finished_semaphores_[current_frame_]};
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = signal_semaphores;

  if (vkQueueSubmit(graphics_queue_, 1, &submit_info,
                    in_flight_fences_[current_frame_]) != VK_SUCCESS) {
    Log_Err("[vk] Failed to submit draw command buffer");
    return false;
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = signal_semaphores;

  VkSwapchainKHR swapchains[] = {swapchain_};
  present_info.swapchainCount = 1;
  present_info.pSwapchains = swapchains;
  present_info.pImageIndices = &current_image_index_;

  VkResult result = vkQueuePresentKHR(present_queue_, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    RecreateSwapchain(swapchain_extent_.width, swapchain_extent_.height);
  } else if (result != VK_SUCCESS) {
    Log_Err("[vk] Failed to present swapchain image");
  }

  current_frame_ = (current_frame_ + 1) % kMaxFramesInFlight;

  return true;
}

bool VulkanDevice::SetViewport(const math::Rect& viewport) {
  current_viewport_ = viewport;
  if (!scene_active_) {
    return true;
  }

  VkViewport vk_viewport{};
  vk_viewport.x = static_cast<float>(viewport.x1);
  vk_viewport.y = static_cast<float>(viewport.y1);
  vk_viewport.width = static_cast<float>(viewport.x2 - viewport.x1);
  vk_viewport.height = static_cast<float>(viewport.y2 - viewport.y1);
  vk_viewport.minDepth = 0.0f;
  vk_viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffers_[current_frame_], 0, 1, &vk_viewport);

  VkRect2D scissor{};
  scissor.offset = {viewport.x1, viewport.y1};
  scissor.extent = {static_cast<uint32_t>(viewport.x2 - viewport.x1),
                    static_cast<uint32_t>(viewport.y2 - viewport.y1)};
  vkCmdSetScissor(command_buffers_[current_frame_], 0, 1, &scissor);

  return true;
}

void VulkanDevice::ForceStateReset() {
  if (vk_device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(vk_device_);
  }
}

void VulkanDevice::TakeScreenShot(std::string_view /*filename*/) {}
void VulkanDevice::InitializeDebugAPI() {}
void VulkanDevice::CaptureCurrentFrame() {}
void VulkanDevice::BeginOcclusionQuery() {}
bool VulkanDevice::EndOcclusionQuery() { return false; }
void VulkanDevice::WaitRetrace() {
  if (vk_device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(vk_device_);
  }
}

//////////////////////////////////////////////////////////////////////////
// Texture

std::unique_ptr<Texture2D> VulkanDevice::CreateTexture2D(
    const int32_t x_res, const int32_t y_res, const uint8_t* data,
    const char bytes_per_pixel, const Format format, const bool render_target) {
  auto result = std::make_unique<VkTexture2D>(this);
  if (!result->Create(x_res, y_res, data, bytes_per_pixel, format,
                      render_target)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<Texture2D> VulkanDevice::CreateTexture2D(const uint8_t* data,
                                                         const int32_t size) {
  auto result = std::make_unique<VkTexture2D>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Vertex / Index buffers

std::unique_ptr<VertexBuffer> VulkanDevice::CreateVertexBuffer(
    const uint8_t* data, const int32_t size) {
  auto result = std::make_unique<VkVertexBuffer>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<VertexBuffer> VulkanDevice::CreateVertexBufferDynamic(
    const int32_t size) {
  auto result = std::make_unique<VkVertexBuffer>(this);
  if (!result->CreateDynamic(size)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<IndexBuffer> VulkanDevice::CreateIndexBuffer(
    const int32_t index_count, const int32_t index_size) {
  auto result = std::make_unique<VkIndexBuffer>(this);
  if (!result->Create(index_count, index_size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Vertex format

std::unique_ptr<VertexFormat> VulkanDevice::CreateVertexFormat(
    const std::vector<VertexAttribute>& attributes, VertexShader* vs) {
  auto result = std::make_unique<VkVertexFormat>(this);
  if (!result->Create(attributes, vs)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — source compilation

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkVertexShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkPixelShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<GeometryShader> VulkanDevice::CreateGeometryShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkGeometryShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<DomainShader> VulkanDevice::CreateDomainShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkDomainShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<HullShader> VulkanDevice::CreateHullShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkHullShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<ComputeShader> VulkanDevice::CreateComputeShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkComputeShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — pre-compiled blob

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<VkVertexShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) shader.reset();
  return shader;
}

std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<VkPixelShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) shader.reset();
  return shader;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — empty shell factory

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShader() {
  return std::make_unique<VkVertexShader>(this);
}
std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShader() {
  return std::make_unique<VkPixelShader>(this);
}
std::unique_ptr<GeometryShader> VulkanDevice::CreateGeometryShader() {
  return std::make_unique<VkGeometryShader>(this);
}
std::unique_ptr<DomainShader> VulkanDevice::CreateDomainShader() {
  return std::make_unique<VkDomainShader>(this);
}
std::unique_ptr<HullShader> VulkanDevice::CreateHullShader() {
  return std::make_unique<VkHullShader>(this);
}
std::unique_ptr<ComputeShader> VulkanDevice::CreateComputeShader() {
  return std::make_unique<VkComputeShader>(this);
}

//////////////////////////////////////////////////////////////////////////
// Constant buffer / shader constants

void VulkanDevice::SetShaderConstants(const ConstantBuffer* buffers) {
  if (buffers && buffers->GetData() && buffers->GetDataLength() > 0) {
    size_t len = std::min(static_cast<size_t>(buffers->GetDataLength()),
                          static_cast<size_t>(128));
    push_constants_.resize(len);
    std::memcpy(push_constants_.data(), buffers->GetData(), len);
  }
}

std::unique_ptr<ConstantBuffer> VulkanDevice::CreateConstantBuffer() {
  return std::make_unique<VkConstantBuffer>(this);
}

//////////////////////////////////////////////////////////////////////////
// Render state objects

std::unique_ptr<BlendState> VulkanDevice::CreateBlendState() {
  return std::make_unique<VkBlendState>(this);
}
std::unique_ptr<DepthStencilState> VulkanDevice::CreateDepthStencilState() {
  return std::make_unique<VkDepthStencilState>(this);
}
std::unique_ptr<RasterizerState> VulkanDevice::CreateRasterizerState() {
  return std::make_unique<VkRasterizerState>(this);
}
std::unique_ptr<SamplerState> VulkanDevice::CreateSamplerState() {
  return std::make_unique<VkSamplerState>(this);
}

bool VulkanDevice::SetRenderTarget(Texture2D* /*rt*/) { return false; }

//////////////////////////////////////////////////////////////////////////
// Draw calls

bool VulkanDevice::DrawIndexedTriangles(int32_t count,
                                        int32_t /*num_vertices*/) {
  if (!ApplyRequestedRenderState()) return false;
  if (current_vertex_buffer_) {
    ApplyVertexBuffer(current_vertex_buffer_, current_vertex_buffer_offset_);
  } else if (requested_vertex_buffer_) {
    ApplyVertexBuffer(requested_vertex_buffer_,
                      requested_vertex_buffer_offset_);
  }
  if (current_ib_) {
    ApplyIndexBuffer(current_ib_);
  }
  FlushDescriptors();
  VkPipeline pipeline =
      GetOrCreatePipeline(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  if (!pipeline) return false;
  vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline);
  vkCmdDrawIndexed(GetCurrentCommandBuffer(), count * 3, 1, 0, 0, 0);
  return true;
}

bool VulkanDevice::DrawIndexedLines(int32_t count, int32_t /*num_vertices*/) {
  if (!ApplyRequestedRenderState()) return false;
  if (current_vertex_buffer_) {
    ApplyVertexBuffer(current_vertex_buffer_, current_vertex_buffer_offset_);
  } else if (requested_vertex_buffer_) {
    ApplyVertexBuffer(requested_vertex_buffer_,
                      requested_vertex_buffer_offset_);
  }
  if (current_ib_) {
    ApplyIndexBuffer(current_ib_);
  }
  FlushDescriptors();
  VkPipeline pipeline = GetOrCreatePipeline(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
  if (!pipeline) return false;
  vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline);
  vkCmdDrawIndexed(GetCurrentCommandBuffer(), count * 2, 1, 0, 0, 0);
  return true;
}

bool VulkanDevice::DrawTriangles(int32_t count) {
  if (!ApplyRequestedRenderState()) return false;
  if (current_vertex_buffer_) {
    ApplyVertexBuffer(current_vertex_buffer_, current_vertex_buffer_offset_);
  } else if (requested_vertex_buffer_) {
    ApplyVertexBuffer(requested_vertex_buffer_,
                      requested_vertex_buffer_offset_);
  }
  FlushDescriptors();
  VkPipeline pipeline =
      GetOrCreatePipeline(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  if (!pipeline) return false;
  vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline);
  vkCmdDraw(GetCurrentCommandBuffer(), count * 3, 1, 0, 0);
  return true;
}

bool VulkanDevice::DrawLines(int32_t count) {
  if (!ApplyRequestedRenderState()) return false;
  if (current_vertex_buffer_) {
    ApplyVertexBuffer(current_vertex_buffer_, current_vertex_buffer_offset_);
  } else if (requested_vertex_buffer_) {
    ApplyVertexBuffer(requested_vertex_buffer_,
                      requested_vertex_buffer_offset_);
  }
  FlushDescriptors();
  VkPipeline pipeline = GetOrCreatePipeline(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
  if (!pipeline) return false;
  vkCmdBindPipeline(GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline);
  vkCmdDraw(GetCurrentCommandBuffer(), count * 2, 1, 0, 0);
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Internal render-state plumbing

bool VulkanDevice::ApplyRenderState(const Sampler sampler,
                                    const RenderState render_state,
                                    const RenderStateValue value) {
  switch (render_state) {
    case RenderState::kBlendState: {
      current_bs_ = static_cast<VkBlendState*>(value.blend_state);
      if (current_bs_) return current_bs_->Apply();
      return true;
    }
    case RenderState::kRasterizerState: {
      current_rs_ = static_cast<VkRasterizerState*>(value.rasterizer_state);
      if (current_rs_) return current_rs_->Apply();
      return true;
    }
    case RenderState::kDepthStencilState: {
      current_dss_ =
          static_cast<VkDepthStencilState*>(value.depth_stencil_state);
      if (current_dss_) return current_dss_->Apply();
      return true;
    }
    case RenderState::kSamplerState: {
      if (!value.sampler_state) return false;
      return value.sampler_state->Apply(sampler);
    }
    case RenderState::kTexture: {
      if (!value.texture) {
        uint32_t slot = 0;
        if (sampler >= Sampler::kPs0 && sampler <= Sampler::kPs15) {
          slot = static_cast<uint32_t>(sampler) -
                 static_cast<uint32_t>(Sampler::kPs0);
        } else if (sampler >= Sampler::kVs0 && sampler <= Sampler::kVs3) {
          slot = 16 + static_cast<uint32_t>(sampler) -
                 static_cast<uint32_t>(Sampler::kVs0);
        } else if (sampler >= Sampler::kGs0 && sampler <= Sampler::kGs3) {
          slot = 20 + static_cast<uint32_t>(sampler) -
                 static_cast<uint32_t>(Sampler::kGs0);
        }
        BindTexture(slot, VK_NULL_HANDLE, VK_NULL_HANDLE);
        return true;
      }
      return ApplyTextureToSampler(sampler, value.texture);
    }
    case RenderState::kVertexFormat: {
      current_vf_ = static_cast<VkVertexFormat*>(value.vertex_format);
      if (current_vf_) {
        current_vertex_format_size_ = current_vf_->GetSize();
        return ApplyVertexFormat(current_vf_);
      }
      current_vertex_format_size_ = 0;
      return true;
    }
    case RenderState::kIndexBuffer: {
      current_ib_ = static_cast<VkIndexBuffer*>(value.index_buffer);
      if (current_ib_) {
        return ApplyIndexBuffer(current_ib_);
      }
      return true;
    }
    case RenderState::kVertexShader: {
      current_vs_ = static_cast<VkVertexShader*>(value.vertex_shader);
      if (current_vs_) return ApplyVertexShader(current_vs_);
      return true;
    }
    case RenderState::kPixelShader: {
      current_ps_ = static_cast<VkPixelShader*>(value.pixel_shader);
      if (current_ps_) return ApplyPixelShader(current_ps_);
      return true;
    }
    case RenderState::kGeometryShader: {
      current_gs_ = static_cast<VkGeometryShader*>(value.geometry_shader);
      if (current_gs_) return ApplyGeometryShader(current_gs_);
      return true;
    }
    default:
      return true;
  }
}

bool VulkanDevice::SetNoVertexBuffer() {
  current_vertex_buffer_ = nullptr;
  current_vertex_buffer_offset_ = 0;
  return true;
}

bool VulkanDevice::CommitRenderStates() { return true; }

//////////////////////////////////////////////////////////////////////////
// Descriptors and Pipeline

void VulkanDevice::BindTexture(uint32_t slot, VkImageView view,
                               VkSampler sampler) {
  if (slot >= 32) return;
  bool dirty = false;
  if (view != VK_NULL_HANDLE && bound_textures_[slot].view != view) {
    bound_textures_[slot].view = view;
    dirty = true;
  }
  if (sampler != VK_NULL_HANDLE && bound_textures_[slot].sampler != sampler) {
    bound_textures_[slot].sampler = sampler;
    dirty = true;
  }
  if (dirty) {
    for (uint32_t f = 0; f < kMaxFramesInFlight; ++f) {
      descriptor_dirty_[f][slot] = true;
    }
  }
}

void VulkanDevice::FlushDescriptors() {
  std::vector<VkDescriptorImageInfo> image_infos;
  std::vector<VkWriteDescriptorSet> descriptor_writes;
  image_infos.reserve(32);
  descriptor_writes.reserve(32);

  for (uint32_t i = 0; i < 32; i++) {
    if (descriptor_dirty_[current_frame_][i]) {
      VkDescriptorImageInfo image_info{};
      image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_info.imageView = bound_textures_[i].view;
      image_info.sampler = bound_textures_[i].sampler;

      if (image_info.imageView != VK_NULL_HANDLE &&
          image_info.sampler != VK_NULL_HANDLE) {
        image_infos.push_back(image_info);
      }
    }
  }

  uint32_t info_index = 0;
  for (uint32_t i = 0; i < 32; i++) {
    if (descriptor_dirty_[current_frame_][i]) {
      if (bound_textures_[i].view != VK_NULL_HANDLE &&
          bound_textures_[i].sampler != VK_NULL_HANDLE) {
        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets_[current_frame_];
        descriptor_write.dstBinding = i;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pImageInfo = &image_infos[info_index++];
        descriptor_writes.push_back(descriptor_write);
      }
      descriptor_dirty_[current_frame_][i] = false;
    }
  }

  if (!descriptor_writes.empty()) {
    vkUpdateDescriptorSets(vk_device_,
                           static_cast<uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);
  }
  vkCmdBindDescriptorSets(GetCurrentCommandBuffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0,
                          1, &descriptor_sets_[current_frame_], 0, nullptr);

  if (!push_constants_.empty() && pipeline_layout_ != VK_NULL_HANDLE) {
    vkCmdPushConstants(
        GetCurrentCommandBuffer(), pipeline_layout_,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        static_cast<uint32_t>(push_constants_.size()), push_constants_.data());
  }
}

VkCommandBuffer VulkanDevice::BeginSingleTimeCommands() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command_pool_;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vk_device_, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer cmd) {
  vkEndCommandBuffer(cmd);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  vkQueueSubmit(graphics_queue_, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphics_queue_);

  vkFreeCommandBuffers(vk_device_, command_pool_, 1, &cmd);
}

VkPipeline VulkanDevice::GetOrCreatePipeline(VkPrimitiveTopology topology) {
  VkBlendState* bs =
      current_bs_ ? current_bs_
                  : static_cast<VkBlendState*>(default_blend_state_.get());
  VkRasterizerState* rs =
      current_rs_
          ? current_rs_
          : static_cast<VkRasterizerState*>(default_rasterizer_state_.get());
  VkDepthStencilState* dss = current_dss_
                                 ? current_dss_
                                 : static_cast<VkDepthStencilState*>(
                                       default_depth_stencil_state_.get());

  if (!current_vs_ || !current_ps_ || !current_vf_ || !bs || !rs || !dss) {
    return VK_NULL_HANDLE;
  }

  PipelineKey key{};
  key.vs = reinterpret_cast<VkShaderModule>(current_vs_->GetHandle());
  key.ps = reinterpret_cast<VkShaderModule>(current_ps_->GetHandle());
  if (key.vs == VK_NULL_HANDLE || key.ps == VK_NULL_HANDLE) {
    return VK_NULL_HANDLE;
  }
  key.gs = current_gs_
               ? reinterpret_cast<VkShaderModule>(current_gs_->GetHandle())
               : VK_NULL_HANDLE;
  key.vertex_format_hash = current_vf_->GetHash();
  key.blend_enable = bs->GetDesc().blend_enable;
  key.src_blend = bs->GetDesc().src_blend;
  key.dst_blend = bs->GetDesc().dest_blend;
  key.blend_op = bs->GetDesc().blend_op;
  key.src_blend_alpha = bs->GetDesc().src_blend_alpha;
  key.dst_blend_alpha = bs->GetDesc().dest_blend_alpha;
  key.blend_op_alpha = bs->GetDesc().blend_op_alpha;
  key.write_mask = bs->GetDesc().render_target_write_mask;
  key.cull_mode = rs->GetCullMode();
  key.fill_mode = rs->GetFillMode();
  key.depth_enable = dss->GetDepthEnable();
  key.depth_write = dss->GetDepthWriteEnable();
  key.depth_func = dss->GetDepthFunc();
  key.topology = topology;

  auto it = pipeline_cache_.find(key);
  if (it != pipeline_cache_.end()) {
    return it->second;
  }

  std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  VkPipelineShaderStageCreateInfo vert_stage{};
  vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_stage.module = key.vs;
  vert_stage.pName = current_vs_->GetEntryFunction().empty()
                         ? "main"
                         : current_vs_->GetEntryFunction().c_str();
  shader_stages.push_back(vert_stage);

  VkPipelineShaderStageCreateInfo frag_stage{};
  frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_stage.module = key.ps;
  frag_stage.pName = current_ps_->GetEntryFunction().empty()
                         ? "main"
                         : current_ps_->GetEntryFunction().c_str();
  shader_stages.push_back(frag_stage);

  if (key.gs) {
    VkPipelineShaderStageCreateInfo geom_stage{};
    geom_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geom_stage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geom_stage.module = key.gs;
    geom_stage.pName = current_gs_->GetEntryFunction().empty()
                           ? "main"
                           : current_gs_->GetEntryFunction().c_str();
    shader_stages.push_back(geom_stage);
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info{};
  vertex_input_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  auto binding = current_vf_->GetBindingDescription();
  auto attributes = current_vf_->GetAttributeDescriptions();
  if (!attributes.empty()) {
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding;
    vertex_input_info.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributes.size());
    vertex_input_info.pVertexAttributeDescriptions = attributes.data();
  }

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = topology;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer = *rs->GetVkDesc();
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depth_stencil = *dss->GetVkDesc();
  depth_stencil.depthTestEnable = VK_FALSE;
  depth_stencil.depthWriteEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo color_blending = *bs->GetVkDesc();

  std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamic_state{};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount =
      static_cast<uint32_t>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
  pipeline_info.pStages = shader_stages.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = &depth_stencil;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state;
  pipeline_info.layout = pipeline_layout_;
  pipeline_info.renderPass = render_pass_;
  pipeline_info.subpass = 0;

  VkPipeline pipeline;
  if (vkCreateGraphicsPipelines(vk_device_, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline) != VK_SUCCESS) {
    Log_Err("[vk] Failed to create graphics pipeline");
    return VK_NULL_HANDLE;
  }

  pipeline_cache_[key] = pipeline;
  return pipeline;
}

}  // namespace renderer
