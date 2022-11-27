#pragma once
#include <variant>

#include "src/base/rectangle.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/message_enum.h"

class CWBMessage final {
 public:
  struct reposition {
    math::CRect rectangle;
    bool moved;
    bool resized;
  };
  struct keyboard {
    int32_t key;
    int32_t keyboard_state;
  };
  struct menucontext {
    int32_t item;
    WBGUID menu;
  };

  CWBMessage();
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target, int32_t x);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target, int32_t x,
             int32_t y);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target,
             keyboard&& kb);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target,
             menucontext&& mc);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target,
             const math::CRect rec, bool moved, bool resized);
  CWBMessage(const CWBMessage&) = default;
  CWBMessage(CWBMessage&&) = default;
  CWBMessage& operator=(const CWBMessage&) = default;
  CWBMessage& operator=(CWBMessage&&) = default;
  ~CWBMessage() = default;

  WBMESSAGE GetMessage() const { return message_; }
  [[nodiscard]] WBGUID GetTarget() const { return target_; }

  // WBM_CONTENTOFFSETCHANGE, yes, this is x,y but really?
  // WBM_{anything mouse button related}
  [[nodiscard]] math::CPoint GetPosition() const {
    auto& pos = std::get<std::array<int32_t, 2>>(data_);
    return math::CPoint(pos[0], pos[1]);
  }
  [[nodiscard]] int32_t Data() const { return std::get<int32_t>(data_); }

  // WBM_REPOSITION
  [[nodiscard]] math::CRect Rectangle() const {
    return std::get<reposition>(data_).rectangle;
  };
  // WBM_REPOSITION
  [[nodiscard]] bool Moved() const {
    return std::get<reposition>(data_).moved;
  };
  // WBM_REPOSITION
  [[nodiscard]] bool Resized() const {
    return std::get<reposition>(data_).resized;
  };

  // WBM_KEYDOWN, WBM_KEYUP, WBM_CHAR
  [[nodiscard]] int32_t Key() const { return std::get<keyboard>(data_).key; };
  // WBM_KEYDOWN, WBM_KEYUP, WBM_CHAR
  [[nodiscard]] int32_t KeyboardState() const {
    return std::get<keyboard>(data_).keyboard_state;
  };
  // WBM_REBUILDCONTEXTITEM
  [[nodiscard]] menucontext MenuContext() const {
    return std::get<menucontext>(data_);
  }

  bool IsTargetID(std::string_view Name);
  std::string_view GetTargetID();

  bool IsMouseMessage();

 private:
  CWBApplication* app_;
  WBMESSAGE message_;
  WBGUID target_;
  std::variant<reposition, std::array<int32_t, 2>, int32_t, keyboard,
               menucontext>
      data_;
};
