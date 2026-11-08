#pragma once

#ifdef _WIN32
#include <guiddef.h>
#else
#include "src/base/win_compat.h"
#endif

#include <cstddef>
#include <functional>

namespace std {
template <>
struct hash<GUID> {
  std::size_t operator()(const GUID& guid) const;
};
}  // namespace std
