#pragma once
#include "src/core2/core2_config.h"

#ifdef CORE_API_DX11

#include "src/core2/dx11_device.h"
typedef CCoreDX11Device CCore;

#endif

#define UVTRANSLATION(uv, res) ((uv + UVOffset) / (float)(res))

#include "src/core2/resource.h"
#include "src/core2/texture.h"
