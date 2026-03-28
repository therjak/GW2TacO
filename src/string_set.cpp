module;
#include <string>
#include <string_view>
#include <unordered_set>

module taco.string_set;

std::unordered_set<std::string> string_set;
std::string_view AddStringToSet(std::string_view string) {
  if (string.empty()) return {};
  const auto& p = string_set.emplace(string);
  return *p.first;
}