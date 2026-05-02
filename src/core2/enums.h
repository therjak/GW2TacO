#pragma once
#include <array>
#include <vector>

#include "src/base/enum_helpers.h"

namespace renderer {

enum class DeviceApi : uint8_t {
  kDx9,
  kDx11,
  kOpenGl,
  kCore1Wrapper,
};

enum class Format : uint8_t {
  kUnknown = 0,
  kA8R8G8B8 = 1,
  kA32B32G32R32f = 2,
  kR32f = 3,
  kG16R16f = 4,
  kR16G16B16A16Float = 5,
};

enum class VertexAttribute : int16_t {
  kStop = -1,
  kPosition3 = 0,
  kPosition4 = 1,
  kNormal3 = 2,
  kTexCoord2 = 3,
  kColor4 = 4,
  kColor16 = 5,
  kPositionT4 = 6,
  kTexCoord4 = 7,
};

enum class ComparisonFunction : uint16_t {
  kNever = 0,
  kLess = 1,
  kEqual = 2,
  kLessEqual = 3,
  kGreater = 4,
  kNotEqual = 5,
  kGreaterEqual = 6,
  kAlways = 7,
};

using Cmp = EnumNamePair<ComparisonFunction>;
constexpr std::array ComparisonFunctionNames = {
    Cmp{ComparisonFunction::kNever, ("Never")},
    Cmp{ComparisonFunction::kLess, ("Less")},
    Cmp{ComparisonFunction::kEqual, ("Equal")},
    Cmp{ComparisonFunction::kLessEqual, ("LessEqual")},
    Cmp{ComparisonFunction::kGreater, ("Greater")},
    Cmp{ComparisonFunction::kNotEqual, ("NotEqual")},
    Cmp{ComparisonFunction::kGreaterEqual, ("GreaterEqual")},
    Cmp{ComparisonFunction::kAlways, ("Always")}};

enum class BlendFactor : uint16_t {
  kZero = 0,
  kOne = 1,
  kSrcCol = 2,
  kInvSrcCol = 3,
  kSrcAlpha = 4,
  kInvSrcAlpha = 5,
  kDstAlpha = 6,
  kInvDstAlpha = 7,
  kDstColor = 8,
  kInvDstColor = 9,
  kSrcAlphaSaturate = 10,
  kBlendFactor = 11,
  kInvBlendFactor = 12,
};

using Blend = EnumNamePair<BlendFactor>;
constexpr std::array BlendFactorNames = {
    Blend{BlendFactor::kZero, ("Zero")},
    Blend{BlendFactor::kOne, ("One")},
    Blend{BlendFactor::kSrcCol, ("SrcCol")},
    Blend{BlendFactor::kInvSrcCol, ("InvSrcCol")},
    Blend{BlendFactor::kSrcAlpha, ("SrcAlpha")},
    Blend{BlendFactor::kInvSrcAlpha, ("InvSrcAlpha")},
    Blend{BlendFactor::kDstAlpha, ("DstAlpha")},
    Blend{BlendFactor::kInvDstAlpha, ("InvDstAlpha")},
    Blend{BlendFactor::kDstColor, ("DstCol")},
    Blend{BlendFactor::kInvDstColor, ("InvDstCol")},
    Blend{BlendFactor::kSrcAlphaSaturate, ("SrcAlphaSaturate")},
    Blend{BlendFactor::kBlendFactor, ("BlendFactor")},
    Blend{BlendFactor::kInvBlendFactor, ("InvBlendFactor")}};

enum class BlendOp : uint8_t {
  kAdd = 0,
  kSub = 1,
  kRevSub = 2,
  kMin = 3,
  kMax = 4,
};
using Blendo = EnumNamePair<BlendOp>;
constexpr std::array BlendOpNames = {
    Blendo{BlendOp::kAdd, ("Add")}, Blendo{BlendOp::kSub, ("Sub")},
    Blendo{BlendOp::kRevSub, ("RevSub")}, Blendo{BlendOp::kMin, ("Min")},
    Blendo{BlendOp::kMax, ("Max")}};

enum class CullMode : uint8_t {
  kNone = 0,
  kCw = 1,
  kCcw = 2,
};
using Cullmo = EnumNamePair<CullMode>;
constexpr std::array CullModeNames = {Cullmo{CullMode::kNone, ("None")},
                                      Cullmo{CullMode::kCw, ("CW")},
                                      Cullmo{CullMode::kCcw, ("CCW")}};

enum class FillMode : uint8_t {
  kSolid = 0,
  kEdges = 1,
  kPoints = 2,
};
using Fillmod = EnumNamePair<FillMode>;
constexpr std::array FillModeNames = {Fillmod{FillMode::kSolid, ("Solid")},
                                      Fillmod{FillMode::kEdges, ("Edges")},
                                      Fillmod{FillMode::kPoints, ("Points")}};

enum class TextureAddressMode : uint8_t {
  kWrap = 0,
  kMirror = 1,
  kClamp = 2,
  kBorder = 3,
  kMirrorOnce = 4,
};
using TextureAddressmod = EnumNamePair<TextureAddressMode>;
constexpr std::array AddressModeNames = {
    TextureAddressmod{TextureAddressMode::kWrap, ("Wrap")},
    TextureAddressmod{TextureAddressMode::kMirror, ("Mirror")},
    TextureAddressmod{TextureAddressMode::kClamp, ("Clamp")},
    TextureAddressmod{TextureAddressMode::kBorder, ("Border")},
    TextureAddressmod{TextureAddressMode::kMirrorOnce, ("MirrorOnce")}};

enum class RenderState : uint16_t {
  kVertexShader = 0,
  kPixelShader,
  kGeometryShader,
  kHullShader,
  kDomainShader,
  kTexture,
  kVertexFormat,
  kIndexBuffer,
  kSamplerState,
  kBlendState,
  kRasterizerState,
  kDepthStencilState,
  kComputeShader,
};

enum class Sampler : int16_t {
  kPs0 = 0,
  kPs1 = 1,
  kPs2 = 2,
  kPs3 = 3,
  kPs4 = 4,
  kPs5 = 5,
  kPs6 = 6,
  kPs7 = 7,
  kPs8 = 8,
  kPs9 = 9,
  kPs10 = 10,
  kPs11 = 11,
  kPs12 = 12,
  kPs13 = 13,
  kPs14 = 14,
  kPs15 = 15,
  kVs0 = 100,
  kVs1 = 101,
  kVs2 = 102,
  kVs3 = 103,
  kGs0 = 200,
  kGs1 = 201,
  kGs2 = 202,
  kGs3 = 203,
};

constexpr uint32_t operator-(const Sampler& l, const Sampler& r) {
  return static_cast<int16_t>(l) - static_cast<int16_t>(r);
}

constexpr std::array SamplerNames = {
    EnumNamePair<Sampler>{Sampler::kPs0, ("PS0")},
    EnumNamePair<Sampler>{Sampler::kPs1, ("PS1")},
    EnumNamePair<Sampler>{Sampler::kPs2, ("PS2")},
    EnumNamePair<Sampler>{Sampler::kPs3, ("PS3")},
    EnumNamePair<Sampler>{Sampler::kPs4, ("PS4")},
    EnumNamePair<Sampler>{Sampler::kPs5, ("PS5")},
    EnumNamePair<Sampler>{Sampler::kPs6, ("PS6")},
    EnumNamePair<Sampler>{Sampler::kPs7, ("PS7")},
    EnumNamePair<Sampler>{Sampler::kPs8, ("PS8")},
    EnumNamePair<Sampler>{Sampler::kPs9, ("PS9")},
    EnumNamePair<Sampler>{Sampler::kPs10, ("PS10")},
    EnumNamePair<Sampler>{Sampler::kPs11, ("PS11")},
    EnumNamePair<Sampler>{Sampler::kPs12, ("PS12")},
    EnumNamePair<Sampler>{Sampler::kPs13, ("PS13")},
    EnumNamePair<Sampler>{Sampler::kPs14, ("PS14")},
    EnumNamePair<Sampler>{Sampler::kPs15, ("PS15")},
    EnumNamePair<Sampler>{Sampler::kVs0, ("VS0")},
    EnumNamePair<Sampler>{Sampler::kVs1, ("VS1")},
    EnumNamePair<Sampler>{Sampler::kVs2, ("VS2")},
    EnumNamePair<Sampler>{Sampler::kVs3, ("VS3")},
    EnumNamePair<Sampler>{Sampler::kGs0, ("GS0")},
    EnumNamePair<Sampler>{Sampler::kGs1, ("GS1")},
    EnumNamePair<Sampler>{Sampler::kGs2, ("GS2")},
    EnumNamePair<Sampler>{Sampler::kGs3, ("GS3")}};

enum class Filter : uint16_t {
  kMinMagMipPoint = 0,
  kMinMagPointMipLinear = 1,
  kMinPointMagLinearMipPoint = 2,
  kMinPointMagMipLinear = 3,
  kMinLinearMagMipPoint = 4,
  kMinLinearMagPointMipLinear = 5,
  kMinMagLinearMipPoint = 6,
  kMinMagMipLinear = 7,
  kAnisotropic = 8,
  kComparisonMinMagMipPoint = 9,
  kComparisonMinMagPointMipLinear = 10,
  kComparisonMinPointMagLinearMipPoint = 11,
  kComparisonMinPointMagMipLinear = 12,
  kComparisonMinLinearMagMipPoint = 13,
  kComparisonMinLinearMagPointMipLinear = 14,
  kComparisonMinMagLinearMipPoint = 15,
  kComparisonMinMagMipLinear = 16,
  kComparisonAnisotropic = 17,
};
constexpr std::array FilterNames = {
    EnumNamePair<Filter>{Filter::kMinMagMipPoint, ("Min_Mag_Mip_Point")},
    EnumNamePair<Filter>{Filter::kMinMagPointMipLinear,
                         ("Min_Mag_Point_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kMinPointMagLinearMipPoint,
                         ("Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<Filter>{Filter::kMinPointMagMipLinear,
                         ("Min_Point_Mag_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kMinLinearMagMipPoint,
                         ("Min_Linear_Mag_Mip_Point")},
    EnumNamePair<Filter>{Filter::kMinLinearMagPointMipLinear,
                         ("Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kMinMagLinearMipPoint,
                         ("Min_Mag_Linear_Mip_Point")},
    EnumNamePair<Filter>{Filter::kMinMagMipLinear, ("Min_Mag_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kAnisotropic, ("Anisotropic")},
    EnumNamePair<Filter>{Filter::kComparisonMinMagMipPoint,
                         ("Comparison_Min_Mag_Mip_Point")},
    EnumNamePair<Filter>{Filter::kComparisonMinMagPointMipLinear,
                         ("Comparison_Min_Mag_Point_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kComparisonMinPointMagLinearMipPoint,
                         ("Comparison_Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<Filter>{Filter::kComparisonMinPointMagMipLinear,
                         ("Comparison_Min_Point_Mag_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kComparisonMinLinearMagMipPoint,
                         ("Comparison_Min_Linear_Mag_Mip_Point")},
    EnumNamePair<Filter>{Filter::kComparisonMinLinearMagPointMipLinear,
                         ("Comparison_Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kComparisonMinMagLinearMipPoint,
                         ("Comparison_Min_Mag_Linear_Mip_Point")},
    EnumNamePair<Filter>{Filter::kComparisonMinMagMipLinear,
                         ("Comparison_Min_Mag_Mip_Linear")},
    EnumNamePair<Filter>{Filter::kComparisonAnisotropic,
                         ("Comparison_Anisotropic")}};

constexpr int32_t kLockReadonly = 0x00000010L;
constexpr int32_t kLockDiscard = 0x00002000L;
constexpr int32_t kLockNoOverwrite = 0x00001000L;
constexpr int32_t kLockNoSysLock = 0x00000800L;
constexpr int32_t kLockDoNotWait = 0x00004000L;

}  // namespace renderer
