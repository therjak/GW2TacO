#include "EnumHelpers.h"

#include <cstdint>
#include <vector>

bool FindEnumByName(const std::vector<EnumNamePair>& Pairs,
                    const std::string_view& Name, int32_t& Result) {
  for (const auto& p : Pairs) {
    if (Name == p.Name) {
      Result = p.Value;
      return true;
    }
  }
  return false;
}

std::string_view FindNameByEnum(const std::vector<EnumNamePair>& Pairs,
                                int32_t Enum) {
  for (const auto& p : Pairs) {
    if (Enum == p.Value) return p.Name;
  }
  return std::string_view();
}
