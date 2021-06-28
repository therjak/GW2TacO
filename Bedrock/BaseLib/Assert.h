#pragma once
#include "BaseConfig.h"

#ifdef _DEBUG
#include <intrin.h>
#endif

#ifndef MEMORY_TRACKING
// memtracking disabled:

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>

#define BASEASSERT(v)                                                  \
  do {                                                                 \
    if (!(v)) {                                                        \
      _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, \
                    #v "\n");                                          \
      __debugbreak();                                                    \
    }                                                                  \
  } while (0)
#define BASEASSERTR(v, r)                                              \
  do {                                                                 \
    if (!(v)) {                                                        \
      _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, \
                    #v "\n");                                          \
      __debugbreak();                                                    \
      return r;                                                        \
    }                                                                  \
  } while (0)
#else
#define BASEASSERT(v) \
  do {                \
    (v);              \
  } while (0)
#define BASEASSERTR(v, r) \
  do {                    \
    (v);                  \
  } while (0)
#endif

#else
// memtracking enabled:

#ifdef _DEBUG
#define BASEASSERT(v) \
  do {                \
    if (!(v)) {       \
      __debugbreak();   \
    }                 \
  } while (0)
#define BASEASSERTR(v, r) \
  do {                    \
    if (!(v)) {           \
      __debugbreak();       \
      return r;           \
    }                     \
  } while (0)
#else
#define BASEASSERT(v) \
  do {                \
    (v);              \
  } while (0)
#define BASEASSERTR(v, r) \
  do {                    \
    (v);                  \
  } while (0)
#endif

#endif
