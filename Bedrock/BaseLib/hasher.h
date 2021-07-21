#pragma once

#include <cstddef>
#include <functional>
#include <guiddef.h>

namespace std {
template <> struct hash<GUID> {
  std::size_t operator()(const GUID& guid) const;
};
} // namespace std
