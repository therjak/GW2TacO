#pragma once

#include <string>
#include <string_view>

std::string FormatString(std::string_view format, ...);
std::string FormatString(std::string_view format, va_list args);
