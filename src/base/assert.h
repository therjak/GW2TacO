#pragma once

#ifdef _DEBUG
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__clang__)
#define __debugbreak() __builtin_debugtrap()
#elif defined(__GNUC__)
#define __debugbreak() __builtin_trap()
#else
#include <cstdlib>
#define __debugbreak() abort()
#endif
#endif

#ifndef _DEBUG
// memtracking disabled:

#ifdef _DEBUG
#include <crtdbg.h>
#include <stdlib.h>

#define BASEASSERT(v)                                                  \
  do {                                                                 \
    if (!(v)) {                                                        \
      _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, \
                    #v "\n");                                          \
      __debugbreak();                                                  \
    }                                                                  \
  } while (0)
#define BASEASSERTR(v, r)                                              \
  do {                                                                 \
    if (!(v)) {                                                        \
      _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, ASSERTMODULENAME, \
                    #v "\n");                                          \
      __debugbreak();                                                  \
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
      __debugbreak(); \
    }                 \
  } while (0)
#define BASEASSERTR(v, r) \
  do {                    \
    if (!(v)) {           \
      __debugbreak();     \
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
