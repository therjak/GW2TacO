#include "hasher.h"

#include <sstream>

namespace std {
std::size_t hash<GUID>::operator()(const GUID& guid) const {
  std::stringstream ss;
  ss << std::hex << guid.Data1 << guid.Data2 << guid.Data3 << guid.Data4;
  return std::hash<std::string>()(ss.str());
}
}  // namespace std
