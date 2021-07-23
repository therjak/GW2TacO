#pragma once

#include <guiddef.h>

#include <cstddef>
#include <functional>

namespace std {
template <>
struct hash<GUID> {
  std::size_t operator()(const GUID& guid) const;
};
}  // namespace std
