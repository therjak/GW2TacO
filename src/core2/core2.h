#pragma once
#include "core2_config.h"

#ifdef CORE_API_DX11

#include "dx11_device.h"
typedef CCoreDX11Device CCore;

#endif

#define UVTRANSLATION(uv, res) ((uv + UVOffset) / (float)(res))

#include "resource.h"
#include "texture.h"
