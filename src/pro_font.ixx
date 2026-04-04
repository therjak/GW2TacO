module;
#include <string_view>

export module taco.pro_font;

import whiteboard;

export bool CreateProFont(CWBApplication* App, std::string_view FontName);
export bool CreateUniFont(CWBApplication* App, std::string_view FontName);
export bool CreateUniFontOutlined(CWBApplication* App,
                                  std::string_view FontName);
export bool CreateProFontOutlined(CWBApplication* App,
                                  std::string_view FontName);
