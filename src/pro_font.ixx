module;
#include <string_view>

export module taco.pro_font;

import whiteboard;

export bool CreateProFont(gui::CWBApplication* App, std::string_view FontName);
export bool CreateUniFont(gui::CWBApplication* App, std::string_view FontName);
export bool CreateUniFontOutlined(gui::CWBApplication* App,
                                  std::string_view FontName);
export bool CreateProFontOutlined(gui::CWBApplication* App,
                                  std::string_view FontName);
