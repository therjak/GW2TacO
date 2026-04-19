module;
#include <string_view>

export module taco.pro_font;

import whiteboard;

export bool CreateProFont(gui::CWBApplication* app, std::string_view font_name);
export bool CreateUniFont(gui::CWBApplication* app, std::string_view font_name);
export bool CreateUniFontOutlined(gui::CWBApplication* app,
                                  std::string_view font_name);
export bool CreateProFontOutlined(gui::CWBApplication* app,
                                  std::string_view font_name);
