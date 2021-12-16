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

//////////////////////////////////////////////////////////////////////////
// logic
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCKAPI_
#include <windows.h>
//
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
