#include "src/base/hasher.h"

#include <format>

namespace std {
std::size_t hash<GUID>::operator()(const GUID& guid) const {
  const auto h = std::format(
      "{:08x}{:04x}{:04x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
      guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1],
      guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6],
      guid.Data4[7]);
  return std::hash<std::string>()(h);
}
}  // namespace std
