module;
#include <string_view>

#include "src/white_board/application.h"

export module taco.pro_font;

export bool CreateProFont(CWBApplication* App, std::string_view FontName);
export bool CreateUniFont(CWBApplication* App, std::string_view FontName);
export bool CreateUniFontOutlined(CWBApplication* App, std::string_view FontName);
export bool CreateProFontOutlined(CWBApplication* App, std::string_view FontName);
