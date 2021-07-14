#pragma once
#include "Core2_Config.h"

#ifdef CORE_API_DX11

#include "DX11Device.h"
typedef CCoreDX11Device CCore;

#endif

#define UVTRANSLATION(uv, res) ((uv + UVOffset) / (float)(res))

#include "Resource.h"
#include "Texture.h"
