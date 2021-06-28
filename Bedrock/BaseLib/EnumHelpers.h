#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

struct EnumNamePair {
  const int32_t Value;
  const std::string_view Name;
};

bool FindEnumByName(const std::vector<EnumNamePair>& Pairs,
                    const std::string_view& Name, int32_t& Result);
std::string_view FindNameByEnum(const std::vector<EnumNamePair>& Pairs,
                                int32_t Enum);
