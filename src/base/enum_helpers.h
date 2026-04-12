#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

template <class T>
struct EnumNamePair {
  T value;
  std::string_view name;
};

template <class T, class S>
constexpr bool FindEnumByName(const T& pairs, std::string_view name,
                              S& result) {
  for (const auto& p : pairs) {
    if (name == p.name) {
      result = p.value;
      return true;
    }
  }
  return false;
}

template <class T, class S>
constexpr std::string_view FindNameByEnum(const T& pairs, S enumeration) {
  for (const auto& p : pairs) {
    if (enumeration == p.value) return p.name;
  }
  return {};
}
