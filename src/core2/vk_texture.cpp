#include "src/core2/vk_texture.h"

namespace renderer {

VkTexture2D::VkTexture2D(VulkanDevice* device) : Texture2D(device) {}
VkTexture2D::~VkTexture2D() = default;

bool VkTexture2D::Create(const int32_t /*x_res*/, const int32_t /*y_res*/,
                         const uint8_t* /*data*/,
                         const char /*bytes_per_pixel*/,
                         const Format /*format*/,
                         const bool /*render_target*/) {
  return false;
}
bool VkTexture2D::Create(const uint8_t* /*data*/, int32_t /*size*/) {
  return false;
}
bool VkTexture2D::CreateDepthBuffer(const int32_t /*x_res*/,
                                    const int32_t /*y_res*/,
                                    const int32_t /*ms_count*/) {
  return false;
}
bool VkTexture2D::Lock(void** /*result*/, int32_t& /*pitch*/) { return false; }
bool VkTexture2D::UnLock() { return false; }
bool VkTexture2D::Update(const uint8_t* /*data*/, const int32_t /*x_res*/,
                         const int32_t /*y_res*/,
                         const char /*bytes_per_pixel*/) {
  return false;
}
void VkTexture2D::ExportToImage(std::string_view /*filename*/,
                                bool /*clear_alpha*/,
                                ExportImageFormat /*format*/,
                                bool /*degamma*/) {}
bool VkTexture2D::SetToSampler(const Sampler /*sampler*/) { return false; }

}  // namespace renderer
