#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "src/core2/enums.h"
#include "src/core2/vk_device.h"
#include "src/core2/vk_shader.h"

namespace renderer {

TEST(VulkanDeviceTest, APITypeIsVulkan) {
  VulkanDevice device;
  EXPECT_EQ(device.GetAPIType(), DeviceApi::kVulkan);
  EXPECT_FALSE(device.DeviceOk());
  EXPECT_TRUE(device.IsWindowed());
}

TEST(VulkanShaderTest, CompileVertexShaderToSpirvSuccess) {
  const std::string hlsl_source = R"(
    struct VSInput {
      float4 pos : POSITION;
    };
    struct VSOutput {
      float4 pos : SV_Position;
    };
    VSOutput main(VSInput input) {
      VSOutput output;
      output.pos = input.pos;
      return output;
    }
  )";

  std::vector<uint32_t> spirv;
  std::string error;
  bool success = CompileHlslToSpirv(hlsl_source, "main", "vs_6_0", &spirv, &error);

  EXPECT_TRUE(success) << "Compilation failed: " << error;
  EXPECT_FALSE(spirv.empty());
  // Standard SPIR-V magic number is 0x07230203
  if (!spirv.empty()) {
    EXPECT_EQ(spirv[0], 0x07230203u);
  }
}

TEST(VulkanShaderTest, CompileInvalidShaderFails) {
  const std::string invalid_hlsl = "this is not valid hlsl code;";
  std::vector<uint32_t> spirv;
  std::string error;
  bool success = CompileHlslToSpirv(invalid_hlsl, "main", "vs_6_0", &spirv, &error);

  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
}

TEST(VulkanShaderTest, CompileGuiShaders) {
  const std::string hlsl_source =
      "[[vk::combinedImageSampler]] [[vk::binding(0, 0)]] Texture2D GuiTexture;"
      "[[vk::combinedImageSampler]] [[vk::binding(0, 0)]] SamplerState Sampler;"
      "struct ResData { float4 resolution; };"
      "[[vk::push_constant]] ResData resdata;"
      "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; };"
      "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; };"
      "VSOUT vsmain(VSIN x) { VSOUT k; "
      "k.Position=float4(x.Position.x/resdata.resolution.x*2-1,-x.Position.y/"
      "resdata.resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
      "float4 psmain(VSOUT x) : SV_TARGET0 { return "
      "x.Color*GuiTexture.Sample(Sampler,x.UV); }";

  std::vector<uint32_t> vs_spirv;
  std::string vs_error;
  bool vs_success = CompileHlslToSpirv(hlsl_source, "vsmain", "vs_6_0", &vs_spirv, &vs_error);
  EXPECT_TRUE(vs_success) << "VS failed: " << vs_error;

  EXPECT_FALSE(vs_spirv.empty());
  if (!vs_spirv.empty()) {
    EXPECT_EQ(vs_spirv[0], 0x07230203u);
  }

  std::vector<uint32_t> ps_spirv;
  std::string ps_error;
  bool ps_success = CompileHlslToSpirv(hlsl_source, "psmain", "ps_6_0", &ps_spirv, &ps_error);
  EXPECT_TRUE(ps_success) << "PS failed: " << ps_error;
  EXPECT_FALSE(ps_spirv.empty());
  if (!ps_spirv.empty()) {
    EXPECT_EQ(ps_spirv[0], 0x07230203u);
  }
}

}  // namespace renderer
