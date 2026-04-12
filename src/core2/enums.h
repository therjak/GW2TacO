#pragma once
#include <array>
#include <vector>

#include "src/base/enum_helpers.h"

namespace renderer {

enum class CoreDeviceApi : uint8_t {
  kDx9,
  kDx11,
  kOpenGl,
  kCore1Wrapper,
};

enum class CoreFormat : uint8_t {
  kUnknown = 0,
  kA8R8G8B8 = 1,
  kA32B32G32R32f = 2,
  kR32f = 3,
  kG16R16f = 4,
  kR16G16B16A16Float = 5,
};

enum class CoreVertexAttribute : int16_t {
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

enum class CoreComparisonFunction : uint16_t {
  kNever = 0,
  kLess = 1,
  kEqual = 2,
  kLessEqual = 3,
  kGreater = 4,
  kNotEqual = 5,
  kGreaterEqual = 6,
  kAlways = 7,
};

using CoreCmp = EnumNamePair<CoreComparisonFunction>;
constexpr std::array ComparisonFunctionNames = {
    CoreCmp{CoreComparisonFunction::kNever, ("Never")},
    CoreCmp{CoreComparisonFunction::kLess, ("Less")},
    CoreCmp{CoreComparisonFunction::kEqual, ("Equal")},
    CoreCmp{CoreComparisonFunction::kLessEqual, ("LessEqual")},
    CoreCmp{CoreComparisonFunction::kGreater, ("Greater")},
    CoreCmp{CoreComparisonFunction::kNotEqual, ("NotEqual")},
    CoreCmp{CoreComparisonFunction::kGreaterEqual, ("GreaterEqual")},
    CoreCmp{CoreComparisonFunction::kAlways, ("Always")}};

enum class CoreBlendFactor : uint16_t {
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

using CoreBlend = EnumNamePair<CoreBlendFactor>;
constexpr std::array BlendFactorNames = {
    CoreBlend{CoreBlendFactor::kZero, ("Zero")},
    CoreBlend{CoreBlendFactor::kOne, ("One")},
    CoreBlend{CoreBlendFactor::kSrcCol, ("SrcCol")},
    CoreBlend{CoreBlendFactor::kInvSrcCol, ("InvSrcCol")},
    CoreBlend{CoreBlendFactor::kSrcAlpha, ("SrcAlpha")},
    CoreBlend{CoreBlendFactor::kInvSrcAlpha, ("InvSrcAlpha")},
    CoreBlend{CoreBlendFactor::kDstAlpha, ("DstAlpha")},
    CoreBlend{CoreBlendFactor::kInvDstAlpha, ("InvDstAlpha")},
    CoreBlend{CoreBlendFactor::kDstColor, ("DstCol")},
    CoreBlend{CoreBlendFactor::kInvDstColor, ("InvDstCol")},
    CoreBlend{CoreBlendFactor::kSrcAlphaSaturate, ("SrcAlphaSaturate")},
    CoreBlend{CoreBlendFactor::kBlendFactor, ("BlendFactor")},
    CoreBlend{CoreBlendFactor::kInvBlendFactor, ("InvBlendFactor")}};

enum class CoreBlendOp : uint8_t {
  kAdd = 0,
  kSub = 1,
  kRevSub = 2,
  kMin = 3,
  kMax = 4,
};
using CoreBlendo = EnumNamePair<CoreBlendOp>;
constexpr std::array BlendOpNames = {
    CoreBlendo{CoreBlendOp::kAdd, ("Add")},
    CoreBlendo{CoreBlendOp::kSub, ("Sub")},
    CoreBlendo{CoreBlendOp::kRevSub, ("RevSub")},
    CoreBlendo{CoreBlendOp::kMin, ("Min")},
    CoreBlendo{CoreBlendOp::kMax, ("Max")}};

enum class CoreCullMode : uint8_t {
  kNone = 0,
  kCw = 1,
  kCcw = 2,
};
using CoreCullmo = EnumNamePair<CoreCullMode>;
constexpr std::array CullModeNames = {CoreCullmo{CoreCullMode::kNone, ("None")},
                                      CoreCullmo{CoreCullMode::kCw, ("CW")},
                                      CoreCullmo{CoreCullMode::kCcw, ("CCW")}};

enum class CoreFillMode : uint8_t {
  kSolid = 0,
  kEdges = 1,
  kPoints = 2,
};
using CoreFillmod = EnumNamePair<CoreFillMode>;
constexpr std::array FillModeNames = {
    CoreFillmod{CoreFillMode::kSolid, ("Solid")},
    CoreFillmod{CoreFillMode::kEdges, ("Edges")},
    CoreFillmod{CoreFillMode::kPoints, ("Points")}};

enum class CoreTextureAddressMode : uint8_t {
  kWrap = 0,
  kMirror = 1,
  kClamp = 2,
  kBorder = 3,
  kMirrorOnce = 4,
};
using CoreTextureAddressmod = EnumNamePair<CoreTextureAddressMode>;
constexpr std::array AddressModeNames = {
    CoreTextureAddressmod{CoreTextureAddressMode::kWrap, ("Wrap")},
    CoreTextureAddressmod{CoreTextureAddressMode::kMirror, ("Mirror")},
    CoreTextureAddressmod{CoreTextureAddressMode::kClamp, ("Clamp")},
    CoreTextureAddressmod{CoreTextureAddressMode::kBorder, ("Border")},
    CoreTextureAddressmod{CoreTextureAddressMode::kMirrorOnce, ("MirrorOnce")}};

enum class CoreRenderState : uint16_t {
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

enum class CoreSampler : int16_t {
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

constexpr uint32_t operator-(const CoreSampler& l, const CoreSampler& r) {
  return static_cast<int16_t>(l) - static_cast<int16_t>(r);
}

constexpr std::array SamplerNames = {
    EnumNamePair<CoreSampler>{CoreSampler::kPs0, ("PS0")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs1, ("PS1")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs2, ("PS2")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs3, ("PS3")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs4, ("PS4")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs5, ("PS5")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs6, ("PS6")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs7, ("PS7")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs8, ("PS8")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs9, ("PS9")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs10, ("PS10")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs11, ("PS11")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs12, ("PS12")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs13, ("PS13")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs14, ("PS14")},
    EnumNamePair<CoreSampler>{CoreSampler::kPs15, ("PS15")},
    EnumNamePair<CoreSampler>{CoreSampler::kVs0, ("VS0")},
    EnumNamePair<CoreSampler>{CoreSampler::kVs1, ("VS1")},
    EnumNamePair<CoreSampler>{CoreSampler::kVs2, ("VS2")},
    EnumNamePair<CoreSampler>{CoreSampler::kVs3, ("VS3")},
    EnumNamePair<CoreSampler>{CoreSampler::kGs0, ("GS0")},
    EnumNamePair<CoreSampler>{CoreSampler::kGs1, ("GS1")},
    EnumNamePair<CoreSampler>{CoreSampler::kGs2, ("GS2")},
    EnumNamePair<CoreSampler>{CoreSampler::kGs3, ("GS3")}};

enum class CoreFilter : uint16_t {
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
    EnumNamePair<CoreFilter>{CoreFilter::kMinMagMipPoint,
                             ("Min_Mag_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinMagPointMipLinear,
                             ("Min_Mag_Point_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinPointMagLinearMipPoint,
                             ("Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinPointMagMipLinear,
                             ("Min_Point_Mag_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinLinearMagMipPoint,
                             ("Min_Linear_Mag_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinLinearMagPointMipLinear,
                             ("Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinMagLinearMipPoint,
                             ("Min_Mag_Linear_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kMinMagMipLinear,
                             ("Min_Mag_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kAnisotropic, ("Anisotropic")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinMagMipPoint,
                             ("Comparison_Min_Mag_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinMagPointMipLinear,
                             ("Comparison_Min_Mag_Point_Mip_Linear")},
    EnumNamePair<CoreFilter>{
        CoreFilter::kComparisonMinPointMagLinearMipPoint,
        ("Comparison_Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinPointMagMipLinear,
                             ("Comparison_Min_Point_Mag_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinLinearMagMipPoint,
                             ("Comparison_Min_Linear_Mag_Mip_Point")},
    EnumNamePair<CoreFilter>{
        CoreFilter::kComparisonMinLinearMagPointMipLinear,
        ("Comparison_Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinMagLinearMipPoint,
                             ("Comparison_Min_Mag_Linear_Mip_Point")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonMinMagMipLinear,
                             ("Comparison_Min_Mag_Mip_Linear")},
    EnumNamePair<CoreFilter>{CoreFilter::kComparisonAnisotropic,
                             ("Comparison_Anisotropic")}};

constexpr int32_t kCoreLockReadonly = 0x00000010L;
constexpr int32_t kCoreLockDiscard = 0x00002000L;
constexpr int32_t kCoreLockNoOverwrite = 0x00001000L;
constexpr int32_t kCoreLockNoSysLock = 0x00000800L;
constexpr int32_t kCoreLockDoNotWait = 0x00004000L;

}  // namespace renderer
