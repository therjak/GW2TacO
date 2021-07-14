#pragma once

//////////////////////////////////////////////////////////////////////////
// configuration file for CoRE2
// defines high level configuration options like the used graphics api

//////////////////////////////////////////////////////////////////////////
// config macros

// this enables debugging features
#ifdef _DEBUG
#define ENABLE_CORE_DEBUG_MODE
#endif

#define CORE_API_DX11

#define CORE_VERBOSE_LOG

//////////////////////////////////////////////////////////////////////////
// logic
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>

#ifdef CORE_API_DX11

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#ifdef CORE_API_D3DX
#include <D3DX11.h>

#pragma comment(lib, "d3dx11.lib")
#endif

#endif
