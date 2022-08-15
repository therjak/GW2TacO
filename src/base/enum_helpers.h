#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

template <class T>
struct EnumNamePair {
  T Value;
  std::string_view Name;
};

template <class T, class S>
constexpr bool FindEnumByName(const T& Pairs, std::string_view Name,
                              S& Result) {
  for (const auto& p : Pairs) {
    if (Name == p.Name) {
      Result = p.Value;
      return true;
    }
  }
  return false;
}

template <class T, class S>
constexpr std::string_view FindNameByEnum(const T& Pairs, S Enum) {
  for (const auto& p : Pairs) {
    if (Enum == p.Value) return p.Name;
  }
  return {};
}
