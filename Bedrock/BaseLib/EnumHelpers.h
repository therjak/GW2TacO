#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct EnumNamePair {
  int32_t Value;
  std::string_view Name;
};

bool FindEnumByName(const std::vector<EnumNamePair>& Pairs,
                    std::string_view Name, int32_t& Result);
std::string_view FindNameByEnum(const std::vector<EnumNamePair>& Pairs,
                                int32_t Enum);
