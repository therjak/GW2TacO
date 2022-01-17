#pragma once
#include <array>
#include <vector>

#include "src/base/enum_helpers.h"

enum class COREDEVICEAPI : uint8_t {
  DX9,
  DX11,
  OPENGL,
  CORE1WRAPPER,
};

enum class COREFORMAT : uint8_t {
  UNKNOWN = 0,
  A8R8G8B8 = 1,
  A32B32G32R32F = 2,
  R32F = 3,
  G16R16F = 4,
  R16G16B16A16_FLOAT = 5,
};

enum class COREVERTEXATTRIBUTE : int16_t {
  STOP = -1,
  POSITION3 = 0,
  POSITION4 = 1,
  NORMAL3 = 2,
  TEXCOORD2 = 3,
  COLOR4 = 4,
  COLOR16 = 5,
  POSITIONT4 = 6,
  TEXCOORD4 = 7,
};

enum class CORECOMPARISONFUNCTION : uint16_t {
  NEVER = 0,
  LESS = 1,
  EQUAL = 2,
  LEQUAL = 3,
  GREATER = 4,
  NOTEQUAL = 5,
  GEQUAL = 6,
  ALWAYS = 7,
};

using CORECMP = EnumNamePair<CORECOMPARISONFUNCTION>;
constexpr std::array ComparisonFunctionNames = {
    CORECMP{CORECOMPARISONFUNCTION::NEVER, ("Never")},
    CORECMP{CORECOMPARISONFUNCTION::LESS, ("Less")},
    CORECMP{CORECOMPARISONFUNCTION::EQUAL, ("Equal")},
    CORECMP{CORECOMPARISONFUNCTION::LEQUAL, ("LessEqual")},
    CORECMP{CORECOMPARISONFUNCTION::GREATER, ("Greater")},
    CORECMP{CORECOMPARISONFUNCTION::NOTEQUAL, ("NotEqual")},
    CORECMP{CORECOMPARISONFUNCTION::GEQUAL, ("GreaterEqual")},
    CORECMP{CORECOMPARISONFUNCTION::ALWAYS, ("Always")}};

enum class COREBLENDFACTOR : uint16_t {
  ZERO = 0,
  ONE = 1,
  SRCCOL = 2,
  INVSRCCOL = 3,
  SRCALPHA = 4,
  INVSRCALPHA = 5,
  DSTALPHA = 6,
  INVDSTALPHA = 7,
  DSTCOLOR = 8,
  INVDSTCOLOR = 9,
  SRCALPHASATURATE = 10,
  BLENDFACTOR = 11,
  INVBLENDFACTOR = 12,
};

using COREBLEND = EnumNamePair<COREBLENDFACTOR>;
constexpr std::array BlendFactorNames = {
    COREBLEND{COREBLENDFACTOR::ZERO, ("Zero")},
    COREBLEND{COREBLENDFACTOR::ONE, ("One")},
    COREBLEND{COREBLENDFACTOR::SRCCOL, ("SrcCol")},
    COREBLEND{COREBLENDFACTOR::INVSRCCOL, ("InvSrcCol")},
    COREBLEND{COREBLENDFACTOR::SRCALPHA, ("SrcAlpha")},
    COREBLEND{COREBLENDFACTOR::INVSRCALPHA, ("InvSrcAlpha")},
    COREBLEND{COREBLENDFACTOR::DSTALPHA, ("DstAlpha")},
    COREBLEND{COREBLENDFACTOR::INVDSTALPHA, ("InvDstAlpha")},
    COREBLEND{COREBLENDFACTOR::DSTCOLOR, ("DstCol")},
    COREBLEND{COREBLENDFACTOR::INVDSTCOLOR, ("InvDstCol")},
    COREBLEND{COREBLENDFACTOR::SRCALPHASATURATE, ("SrcAlphaSaturate")},
    COREBLEND{COREBLENDFACTOR::BLENDFACTOR, ("BlendFactor")},
    COREBLEND{COREBLENDFACTOR::INVBLENDFACTOR, ("InvBlendFactor")}};

enum class COREBLENDOP : uint8_t {
  ADD = 0,
  SUB = 1,
  REVSUB = 2,
  MIN = 3,
  MAX = 4,
};
using COREBLENDO = EnumNamePair<COREBLENDOP>;
constexpr std::array BlendOpNames = {
    COREBLENDO{COREBLENDOP::ADD, ("Add")},
    COREBLENDO{COREBLENDOP::SUB, ("Sub")},
    COREBLENDO{COREBLENDOP::REVSUB, ("RevSub")},
    COREBLENDO{COREBLENDOP::MIN, ("Min")},
    COREBLENDO{COREBLENDOP::MAX, ("Max")}};

enum class CORECULLMODE : uint8_t {
  NONE = 0,
  CW = 1,
  CCW = 2,
};
using CORECULLMO = EnumNamePair<CORECULLMODE>;
constexpr std::array CullModeNames = {CORECULLMO{CORECULLMODE::NONE, ("None")},
                                      CORECULLMO{CORECULLMODE::CW, ("CW")},
                                      CORECULLMO{CORECULLMODE::CCW, ("CCW")}};

enum class COREFILLMODE : uint8_t {
  SOLID = 0,
  EDGES = 1,
  POINTS = 2,
};
using COREFILLMOD = EnumNamePair<COREFILLMODE>;
constexpr std::array FillModeNames = {
    COREFILLMOD{COREFILLMODE::SOLID, ("Solid")},
    COREFILLMOD{COREFILLMODE::EDGES, ("Edges")},
    COREFILLMOD{COREFILLMODE::POINTS, ("Points")}};

enum class CORETEXTUREADDRESSMODE : uint8_t {
  WRAP = 0,
  MIRROR = 1,
  CLAMP = 2,
  BORDER = 3,
  MIRRORONCE = 4,
};
using CORETEXTUREADDRESSMOD = EnumNamePair<CORETEXTUREADDRESSMODE>;
constexpr std::array AddressModeNames = {
    CORETEXTUREADDRESSMOD{CORETEXTUREADDRESSMODE::WRAP, ("Wrap")},
    CORETEXTUREADDRESSMOD{CORETEXTUREADDRESSMODE::MIRROR, ("Mirror")},
    CORETEXTUREADDRESSMOD{CORETEXTUREADDRESSMODE::CLAMP, ("Clamp")},
    CORETEXTUREADDRESSMOD{CORETEXTUREADDRESSMODE::BORDER, ("Border")},
    CORETEXTUREADDRESSMOD{CORETEXTUREADDRESSMODE::MIRRORONCE, ("MirrorOnce")}};

enum class CORERENDERSTATE : uint16_t {
  VERTEXSHADER = 0,
  PIXELSHADER,
  GEOMETRYSHADER,
  HULLSHADER,
  DOMAINSHADER,
  TEXTURE,
  VERTEXFORMAT,
  INDEXBUFFER,
  SAMPLERSTATE,
  BLENDSTATE,
  RASTERIZERSTATE,
  DEPTHSTENCILSTATE,
  COMPUTESHADER,
};

enum class CORESAMPLER : int16_t {
  PS0 = 0,
  PS1 = 1,
  PS2 = 2,
  PS3 = 3,
  PS4 = 4,
  PS5 = 5,
  PS6 = 6,
  PS7 = 7,
  PS8 = 8,
  PS9 = 9,
  PS10 = 10,
  PS11 = 11,
  PS12 = 12,
  PS13 = 13,
  PS14 = 14,
  PS15 = 15,
  VS0 = 100,
  VS1 = 101,
  VS2 = 102,
  VS3 = 103,
  GS0 = 200,
  GS1 = 201,
  GS2 = 202,
  GS3 = 203,
};

constexpr uint32_t operator-(const CORESAMPLER& l, const CORESAMPLER& r) {
  return static_cast<int16_t>(l) - static_cast<int16_t>(r);
}

constexpr std::array SamplerNames = {
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS0, ("PS0")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS1, ("PS1")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS2, ("PS2")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS3, ("PS3")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS4, ("PS4")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS5, ("PS5")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS6, ("PS6")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS7, ("PS7")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS8, ("PS8")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS9, ("PS9")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS10, ("PS10")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS11, ("PS11")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS12, ("PS12")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS13, ("PS13")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS14, ("PS14")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::PS15, ("PS15")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::VS0, ("VS0")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::VS1, ("VS1")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::VS2, ("VS2")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::VS3, ("VS3")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::GS0, ("GS0")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::GS1, ("GS1")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::GS2, ("GS2")},
    EnumNamePair<CORESAMPLER>{CORESAMPLER::GS3, ("GS3")}};

enum class COREFILTER : uint16_t {
  MIN_MAG_MIP_POINT = 0,
  MIN_MAG_POINT_MIP_LINEAR = 1,
  MIN_POINT_MAG_LINEAR_MIP_POINT = 2,
  MIN_POINT_MAG_MIP_LINEAR = 3,
  MIN_LINEAR_MAG_MIP_POINT = 4,
  MIN_LINEAR_MAG_POINT_MIP_LINEAR = 5,
  MIN_MAG_LINEAR_MIP_POINT = 6,
  MIN_MAG_MIP_LINEAR = 7,
  ANISOTROPIC = 8,
  COMPARISON_MIN_MAG_MIP_POINT = 9,
  COMPARISON_MIN_MAG_POINT_MIP_LINEAR = 10,
  COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT = 11,
  COMPARISON_MIN_POINT_MAG_MIP_LINEAR = 12,
  COMPARISON_MIN_LINEAR_MAG_MIP_POINT = 13,
  COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 14,
  COMPARISON_MIN_MAG_LINEAR_MIP_POINT = 15,
  COMPARISON_MIN_MAG_MIP_LINEAR = 16,
  COMPARISON_ANISOTROPIC = 17,
};
constexpr std::array FilterNames = {
    EnumNamePair<COREFILTER>{COREFILTER::MIN_MAG_MIP_POINT,
                             ("Min_Mag_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_MAG_POINT_MIP_LINEAR,
                             ("Min_Mag_Point_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_POINT_MAG_LINEAR_MIP_POINT,
                             ("Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_POINT_MAG_MIP_LINEAR,
                             ("Min_Point_Mag_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_LINEAR_MAG_MIP_POINT,
                             ("Min_Linear_Mag_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_LINEAR_MAG_POINT_MIP_LINEAR,
                             ("Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_MAG_LINEAR_MIP_POINT,
                             ("Min_Mag_Linear_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::MIN_MAG_MIP_LINEAR,
                             ("Min_Mag_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::ANISOTROPIC, ("Anisotropic")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_MAG_MIP_POINT,
                             ("Comparison_Min_Mag_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
                             ("Comparison_Min_Mag_Point_Mip_Linear")},
    EnumNamePair<COREFILTER>{
        COREFILTER::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
        ("Comparison_Min_Point_Mag_Linear_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
                             ("Comparison_Min_Point_Mag_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
                             ("Comparison_Min_Linear_Mag_Mip_Point")},
    EnumNamePair<COREFILTER>{
        COREFILTER::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        ("Comparison_Min_Linear_Mag_Point_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
                             ("Comparison_Min_Mag_Linear_Mip_Point")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_MIN_MAG_MIP_LINEAR,
                             ("Comparison_Min_Mag_Mip_Linear")},
    EnumNamePair<COREFILTER>{COREFILTER::COMPARISON_ANISOTROPIC,
                             ("Comparison_Anisotropic")}};

constexpr int32_t CORELOCK_READONLY = 0x00000010L;
constexpr int32_t CORELOCK_DISCARD = 0x00002000L;
constexpr int32_t CORELOCK_NOOVERWRITE = 0x00001000L;
constexpr int32_t CORELOCK_NOSYSLOCK = 0x00000800L;
constexpr int32_t CORELOCK_DONOTWAIT = 0x00004000L;
