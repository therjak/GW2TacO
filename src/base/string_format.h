#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> Split(std::string_view input, std::string_view delim);
std::vector<std::string> SplitByWhitespace(std::string_view input);

std::string_view Trim(std::string_view);

std::string B64Decode(std::string_view str64);
std::string B64Encode(std::string_view str64);

std::wstring string2wstring(std::string_view s);
std::string wstring2string(std::wstring_view s);

uint32_t CalculateHash(std::string_view s);
