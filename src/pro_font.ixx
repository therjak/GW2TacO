module;
#include <string_view>

export module taco.pro_font;

import whiteboard;

export bool CreateProFont(gui::Application* app, std::string_view font_name);
export bool CreateUniFont(gui::Application* app, std::string_view font_name);
export bool CreateUniFontOutlined(gui::Application* app,
                                  std::string_view font_name);
export bool CreateProFontOutlined(gui::Application* app,
                                  std::string_view font_name);
