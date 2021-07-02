#include "Enums.h"

#include <vector>


const std::vector<EnumNamePair> ComparisonFunctionNames = {
	{CORECMP_NEVER,			("Never")},
	{CORECMP_LESS,			("Less")},
	{CORECMP_EQUAL,			("Equal")},
	{CORECMP_LEQUAL,		("LessEqual")},
	{CORECMP_GREATER,		("Greater")},
	{CORECMP_NOTEQUAL,		("NotEqual")},
	{CORECMP_GEQUAL,		("GreaterEqual")},
	{CORECMP_ALWAYS,		("Always")},
	{-1,""}
};

const std::vector<EnumNamePair> BlendFactorNames = {
	{COREBLEND_ZERO,				("Zero")},
	{COREBLEND_ONE,					("One")},
	{COREBLEND_SRCCOL,				("SrcCol")},
	{COREBLEND_INVSRCCOL,			("InvSrcCol")},
	{COREBLEND_SRCALPHA,			("SrcAlpha")},
	{COREBLEND_INVSRCALPHA,			("InvSrcAlpha")},
	{COREBLEND_DSTALPHA,			("DstAlpha")},
	{COREBLEND_INVDSTALPHA,			("InvDstAlpha")},
	{COREBLEND_DSTCOLOR,			("DstCol")},
	{COREBLEND_INVDSTCOLOR,			("InvDstCol")},
	{COREBLEND_SRCALPHASATURATE,	("SrcAlphaSaturate")},
	{COREBLEND_BLENDFACTOR,			("BlendFactor")},
	{COREBLEND_INVBLENDFACTOR,		("InvBlendFactor")},
    {-1, ""}};


const std::vector<EnumNamePair> BlendOpNames = {
	{COREBLENDOP_ADD,		("Add")},
	{COREBLENDOP_SUB,		("Sub")},
	{COREBLENDOP_REVSUB,	("RevSub")},
	{COREBLENDOP_MIN,		("Min")},
	{COREBLENDOP_MAX,		("Max")},
	{-1, ""}};

const std::vector<EnumNamePair> CullModeNames = {
	{CORECULL_NONE,			("None")},
	{CORECULL_CW,			("CW")},
	{CORECULL_CCW,			("CCW")},
	{-1, ""}};

const std::vector<EnumNamePair> FillModeNames = {
	{COREFILL_SOLID,		("Solid")},
	{COREFILL_EDGES,		("Edges")},
	{COREFILL_POINTS,		("Points")},
	{-1, ""}};

const std::vector<EnumNamePair> AddressModeNames = {
	{CORETEXADDRESS_WRAP,		("Wrap")},
	{CORETEXADDRESS_MIRROR,		("Mirror")},
	{CORETEXADDRESS_CLAMP,		("Clamp")},
	{CORETEXADDRESS_BORDER,		("Border")},
	{CORETEXADDRESS_MIRRORONCE,	("MirrorOnce")},
	{-1,""}
};

const std::vector<EnumNamePair> SamplerNames = {
	{CORESMP_PS0,			("PS0")},
	{CORESMP_PS1,			("PS1")},
	{CORESMP_PS2,			("PS2")},
	{CORESMP_PS3,			("PS3")},
	{CORESMP_PS4,			("PS4")},
	{CORESMP_PS5,			("PS5")},
	{CORESMP_PS6,			("PS6")},
	{CORESMP_PS7,			("PS7")},
	{CORESMP_PS8,			("PS8")},
	{CORESMP_PS9,			("PS9")},
	{CORESMP_PS10,			("PS10")},
	{CORESMP_PS11,			("PS11")},
	{CORESMP_PS12,			("PS12")},
	{CORESMP_PS13,			("PS13")},
	{CORESMP_PS14,			("PS14")},
	{CORESMP_PS15,			("PS15")},
	{CORESMP_VS0,			("VS0")},
	{CORESMP_VS1,			("VS1")},
	{CORESMP_VS2,			("VS2")},
	{CORESMP_VS3,			("VS3")},
	{CORESMP_GS0,			("GS0")},
	{CORESMP_GS1,			("GS1")},
	{CORESMP_GS2,			("GS2")},
	{CORESMP_GS3,			("GS3")},
	{-1,""}
};

const std::vector<EnumNamePair> FilterNames = {
	{COREFILTER_MIN_MAG_MIP_POINT                           ,("Min_Mag_Mip_Point")},
	{COREFILTER_MIN_MAG_POINT_MIP_LINEAR                    ,("Min_Mag_Point_Mip_Linear")},
	{COREFILTER_MIN_POINT_MAG_LINEAR_MIP_POINT              ,("Min_Point_Mag_Linear_Mip_Point")},
	{COREFILTER_MIN_POINT_MAG_MIP_LINEAR                    ,("Min_Point_Mag_Mip_Linear")},
	{COREFILTER_MIN_LINEAR_MAG_MIP_POINT                    ,("Min_Linear_Mag_Mip_Point")},
	{COREFILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR             ,("Min_Linear_Mag_Point_Mip_Linear")},
	{COREFILTER_MIN_MAG_LINEAR_MIP_POINT                    ,("Min_Mag_Linear_Mip_Point")},
	{COREFILTER_MIN_MAG_MIP_LINEAR                          ,("Min_Mag_Mip_Linear")},
	{COREFILTER_ANISOTROPIC                                 ,("Anisotropic")},
	{COREFILTER_COMPARISON_MIN_MAG_MIP_POINT                ,("Comparison_Min_Mag_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR         ,("Comparison_Min_Mag_Point_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT   ,("Comparison_Min_Point_Mag_Linear_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR         ,("Comparison_Min_Point_Mag_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT         ,("Comparison_Min_Linear_Mag_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR  ,("Comparison_Min_Linear_Mag_Point_Mip_Linear")},
	{COREFILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT         ,("Comparison_Min_Mag_Linear_Mip_Point")},
	{COREFILTER_COMPARISON_MIN_MAG_MIP_LINEAR               ,("Comparison_Min_Mag_Mip_Linear")},
	{COREFILTER_COMPARISON_ANISOTROPIC                      ,("Comparison_Anisotropic")},
	{-1,""}
};
