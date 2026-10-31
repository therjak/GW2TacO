#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/core2/device.h"

#define VK_USE_PLATFORM_WIN32_KHR 1
#include "volk.h"
#include "src/core2/vk_allocator.h"

import math;

namespace renderer {

class VkVertexShader;
class VkPixelShader;
class VkGeometryShader;
class VkVertexFormat;
class VkBlendState;
class VkRasterizerState;
class VkDepthStencilState;
class VkIndexBuffer;

class VulkanDevice : public Device {
 public:
  VulkanDevice();
  ~VulkanDevice() override;

  DeviceApi GetAPIType() override { return DeviceApi::kVulkan; }

  bool Initialize(WindowHandler* window,
                  const int32_t sample_count = 0) override;

  bool DeviceOk() override;
  bool IsWindowed() override;
  void Resize(const int32_t x_res, const int32_t y_res) override;
  void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                         const int32_t y_res) override;

  // Texture creation
  std::unique_ptr<Texture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4, const Format format = Format::kA8R8G8B8,
      const bool render_target = false) override;
  std::unique_ptr<Texture2D> CreateTexture2D(const uint8_t* data,
                                             const int32_t size) override;

  // Buffer creation
  std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint8_t* data,
                                                   const int32_t size) override;
  std::unique_ptr<VertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) override;
  std::unique_ptr<IndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) override;
  std::unique_ptr<VertexFormat> CreateVertexFormat(
      const std::vector<VertexAttribute>& attributes,
      VertexShader* vs = nullptr) override;

  // Shader creation — source compilation
  std::unique_ptr<VertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<PixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<GeometryShader> CreateGeometryShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<DomainShader> CreateDomainShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<HullShader> CreateHullShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<ComputeShader> CreateComputeShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;

  // Shader creation — pre-compiled blob
  std::unique_ptr<VertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<PixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) override;

  // Empty-shell shader factory (no source / blob)
  std::unique_ptr<VertexShader> CreateVertexShader() override;
  std::unique_ptr<PixelShader> CreatePixelShader() override;
  std::unique_ptr<GeometryShader> CreateGeometryShader() override;
  std::unique_ptr<DomainShader> CreateDomainShader() override;
  std::unique_ptr<HullShader> CreateHullShader() override;
  std::unique_ptr<ComputeShader> CreateComputeShader() override;

  void SetShaderConstants(const ConstantBuffer* buffers) override;
  std::unique_ptr<ConstantBuffer> CreateConstantBuffer() override;

  // Render state objects
  std::unique_ptr<BlendState> CreateBlendState() override;
  std::unique_ptr<DepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<RasterizerState> CreateRasterizerState() override;
  std::unique_ptr<SamplerState> CreateSamplerState() override;

  bool SetRenderTarget(Texture2D* rt) override;
  bool SetViewport(const math::Rect& viewport) override;

  // Frame lifecycle
  bool BeginScene() override;
  bool EndScene() override;
  bool Clear(const bool clear_pixels = true, const bool clear_depth = true,
             const Color& color = Color(), const float depth = 1,
             const int32_t stencil = 0) override;
  bool Flip(bool vsync = true) override;

  // Draw calls
  bool DrawIndexedTriangles(int32_t count, int32_t num_vertices) override;
  bool DrawIndexedLines(int32_t count, int32_t num_vertices) override;
  bool DrawTriangles(int32_t count) override;
  bool DrawLines(int32_t count) override;

  void ForceStateReset() override;
  void TakeScreenShot(std::string_view filename) override;
  void InitializeDebugAPI() override;
  void CaptureCurrentFrame() override;

  void BeginOcclusionQuery() override;
  bool EndOcclusionQuery() override;
  void WaitRetrace() override;

  // Accessors for resource creation (used by vk_* resource classes)
  VkDevice GetVkDevice() const { return vk_device_; }
  VkPhysicalDevice GetPhysicalDevice() const { return physical_device_; }
  VkInstance GetInstance() const { return instance_; }
  VkQueue GetGraphicsQueue() const { return graphics_queue_; }
  VkCommandPool GetCommandPool() const { return command_pool_; }
  VkCommandBuffer GetCurrentCommandBuffer() const { return command_buffers_[current_frame_]; }
  VmaAllocator GetAllocator() const { return allocator_.Get(); }
  VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptor_set_layout_; }
  VkPipelineLayout GetPipelineLayout() const { return pipeline_layout_; }
  void BindTexture(uint32_t slot, VkImageView view, VkSampler sampler);
  void FlushDescriptors();

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer cmd);

 private:
  void ResetPrivateResources() override;
  bool InitAPI(const HWND window_handle, const bool full_screen,
               const int32_t x_res, const int32_t y_res,
               const int32_t sample_count = 0,
               const int32_t refresh_rate = 60) override;
  bool ApplyRenderState(const Sampler sampler, const RenderState render_state,
                        const RenderStateValue value) override;
  bool SetNoVertexBuffer() override;
  bool CommitRenderStates() override;

  bool RecreateSwapchain(int32_t x_res, int32_t y_res);
  void CleanupSwapchain();

  VkPipeline GetOrCreatePipeline(VkPrimitiveTopology topology);

  VkInstance instance_ = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice vk_device_ = VK_NULL_HANDLE;
  uint32_t graphics_family_ = UINT32_MAX;
  uint32_t present_family_ = UINT32_MAX;
  VkQueue graphics_queue_ = VK_NULL_HANDLE;
  VkQueue present_queue_ = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  VkFormat swapchain_format_ = VK_FORMAT_UNDEFINED;
  VkExtent2D swapchain_extent_ = {};
  std::vector<VkImage> swapchain_images_;
  std::vector<VkImageView> swapchain_image_views_;
  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> framebuffers_;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;

  static constexpr uint32_t kMaxFramesInFlight = 2;
  std::array<VkSemaphore, kMaxFramesInFlight> image_available_semaphores_ = {};
  std::array<VkSemaphore, kMaxFramesInFlight> render_finished_semaphores_ = {};
  std::array<VkFence, kMaxFramesInFlight> in_flight_fences_ = {};
  std::vector<VkFence> images_in_flight_;
  uint32_t current_frame_ = 0;
  uint32_t current_image_index_ = 0;
  bool initialized_ = false;

  Color clear_color_;
  float clear_depth_ = 1.0f;
  VkMemAllocator allocator_;

  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  std::array<VkDescriptorSet, kMaxFramesInFlight> descriptor_sets_ = {};
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  std::array<std::array<bool, 32>, kMaxFramesInFlight> descriptor_dirty_ = {};
  struct BoundTexture {
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
  };
  std::array<BoundTexture, 32> bound_textures_ = {};

  struct PipelineKey {
    VkShaderModule vs;
    VkShaderModule ps;
    VkShaderModule gs;
    size_t vertex_format_hash;
    bool blend_enable;
    BlendFactor src_blend;
    BlendFactor dst_blend;
    BlendOp blend_op;
    CullMode cull_mode;
    FillMode fill_mode;
    bool depth_enable;
    bool depth_write;
    ComparisonFunction depth_func;
    VkPrimitiveTopology topology;

    bool operator==(const PipelineKey& other) const {
      return vs == other.vs && ps == other.ps && gs == other.gs &&
             vertex_format_hash == other.vertex_format_hash &&
             blend_enable == other.blend_enable &&
             src_blend == other.src_blend && dst_blend == other.dst_blend &&
             blend_op == other.blend_op && cull_mode == other.cull_mode &&
             fill_mode == other.fill_mode &&
             depth_enable == other.depth_enable &&
             depth_write == other.depth_write &&
             depth_func == other.depth_func && topology == other.topology;
    }
  };

  struct PipelineKeyHash {
    size_t operator()(const PipelineKey& k) const {
      size_t res = 17;
      res = res * 31 + std::hash<uint64_t>()(reinterpret_cast<uint64_t>(k.vs));
      res = res * 31 + std::hash<uint64_t>()(reinterpret_cast<uint64_t>(k.ps));
      res = res * 31 + std::hash<uint64_t>()(reinterpret_cast<uint64_t>(k.gs));
      res = res * 31 + k.vertex_format_hash;
      res = res * 31 + std::hash<bool>()(k.blend_enable);
      res = res * 31 + std::hash<int>()(static_cast<int>(k.src_blend));
      res = res * 31 + std::hash<int>()(static_cast<int>(k.dst_blend));
      res = res * 31 + std::hash<int>()(static_cast<int>(k.blend_op));
      res = res * 31 + std::hash<int>()(static_cast<int>(k.cull_mode));
      res = res * 31 + std::hash<int>()(static_cast<int>(k.fill_mode));
      res = res * 31 + std::hash<bool>()(k.depth_enable);
      res = res * 31 + std::hash<bool>()(k.depth_write);
      res = res * 31 + std::hash<int>()(static_cast<int>(k.depth_func));
      res = res * 31 + std::hash<int>()(static_cast<int>(k.topology));
      return res;
    }
  };

  std::unordered_map<PipelineKey, VkPipeline, PipelineKeyHash> pipeline_cache_;

  VkVertexShader* current_vs_ = nullptr;
  VkPixelShader* current_ps_ = nullptr;
  VkGeometryShader* current_gs_ = nullptr;
  VkVertexFormat* current_vf_ = nullptr;
  VkBlendState* current_bs_ = nullptr;
  VkRasterizerState* current_rs_ = nullptr;
  VkDepthStencilState* current_dss_ = nullptr;
  VkIndexBuffer* current_ib_ = nullptr;
  std::vector<uint8_t> push_constants_;
};

}  // namespace renderer

