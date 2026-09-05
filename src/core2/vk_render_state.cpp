#include "src/core2/vk_render_state.h"

namespace renderer {

//////////////////////////////////////////////////////////////////////////
// VkBlendState

VkBlendState::VkBlendState(VulkanDevice* device)
    : BlendState(device), device_(device) {}
VkBlendState::~VkBlendState() = default;
bool VkBlendState::Update() { return false; }
bool VkBlendState::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkDepthStencilState

VkDepthStencilState::VkDepthStencilState(VulkanDevice* device)
    : DepthStencilState(device), device_(device) {}
VkDepthStencilState::~VkDepthStencilState() = default;
bool VkDepthStencilState::Update() { return false; }
bool VkDepthStencilState::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkRasterizerState

VkRasterizerState::VkRasterizerState(VulkanDevice* device)
    : RasterizerState(device), device_(device) {}
VkRasterizerState::~VkRasterizerState() = default;
bool VkRasterizerState::Update() { return false; }
bool VkRasterizerState::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkSamplerState

VkSamplerState::VkSamplerState(VulkanDevice* device)
    : SamplerState(device), device_(device) {}
VkSamplerState::~VkSamplerState() = default;
bool VkSamplerState::Update() { return false; }
bool VkSamplerState::Apply(Sampler /*sampler*/) { return false; }

}  // namespace renderer
