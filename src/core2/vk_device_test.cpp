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

TEST(VulkanShaderTest, CompileTrailShaders) {
  const std::string hlsl_source =
      "[[vk::combinedImageSampler]] [[vk::binding(0, 0)]] Texture2D GuiTexture;"
      "[[vk::combinedImageSampler]] [[vk::binding(0, 0)]] SamplerState Sampler;"
      "[[vk::binding(32, 0)]] cbuffer resdata"
      "{"
      "   float4x4 camera;"
      "   float4x4 persp;"
      "   float4   charpos;"
      "   float4   data;"
      "   float4   color;"
      "   float2   nearFarFades;"
      "   float    fadeOutBubble;"
      "   float    width;"
      "   float    uvScale;"
      "   float    width2d;"
      "};"
      "float rayspheredepth(float3 o, float3 d)"
      "{"
      "  if (fadeOutBubble==0) return 1;"
      "  float4 c = mul(camera,charpos + float4(0,1,0,0));"
      "  c/=c.w;"
      "  float r=2.00;"
      "  float3 oc=o-c.xyz;"
      "  float discriminant = dot(oc, d) * dot(oc, d) - ( dot( oc, oc ) - r * "
      "r ); "
      "  if (discriminant < 0) return 1;"
      "  float t1 = -dot(d, oc) - sqrt(discriminant);"
      "  float t2 = -dot(d, oc) + sqrt(discriminant);"
      "  return saturate(1-abs(t2-t1)/r/2.0);"
      "}"
      "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; "
      "float4 Pos2 : TEXCOORD1; float4 Color : COLOR0; };"
      "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; float4 p : TEXCOORD1; };"
      "VSOUT vsmain(VSIN x) { VSOUT k; "
      "k.p=k.Position=mul(camera,(x.Position-x.Pos2)*width+x.Pos2); "
      "k.p/=k.p.w; float4 p2 = mul(camera,x.Pos2); p2/=p2.w; "
      "k.Position=mul(persp,(k.Position-p2)*width2d+p2); "
      "k.Position.y=-k.Position.y; "
      "k.UV=float2(x.UV.x,x.UV.y*uvScale); k.Color=x.Color; k.p/=k.p.w; return "
      "k; }"
      "float4 psmain(VSOUT x) : SV_TARGET0 {  float farFade = "
      "saturate(1.0-(x.p.z-nearFarFades.x)/(nearFarFades.y-nearFarFades.x)); "
      "float a=rayspheredepth(0,normalize(x.p.xyz)); a*=farFade; return "
      "x.Color*GuiTexture.Sample(Sampler,x.UV + "
      "float2(0,data.x))*color*float4(1,1,1,a); }";

  std::vector<uint32_t> vs_spirv;
  std::string vs_error;
  bool vs_success =
      CompileHlslToSpirv(hlsl_source, "vsmain", "vs_6_0", &vs_spirv, &vs_error);
  EXPECT_TRUE(vs_success) << "VS failed: " << vs_error;
  EXPECT_FALSE(vs_spirv.empty());
  if (!vs_spirv.empty()) {
    EXPECT_EQ(vs_spirv[0], 0x07230203u);
  }

  std::vector<uint32_t> ps_spirv;
  std::string ps_error;
  bool ps_success =
      CompileHlslToSpirv(hlsl_source, "psmain", "ps_6_0", &ps_spirv, &ps_error);
  EXPECT_TRUE(ps_success) << "PS failed: " << ps_error;
  EXPECT_FALSE(ps_spirv.empty());
  if (!ps_spirv.empty()) {
    EXPECT_EQ(ps_spirv[0], 0x07230203u);
  }
}

}  // namespace renderer
