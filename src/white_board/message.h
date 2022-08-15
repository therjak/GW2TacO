#pragma once
#include "src/base/rectangle.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/message_enum.h"

class CWBMessage final {
  CWBApplication* App;
  WBMESSAGE Message;
  WBGUID Target;

 public:
  union {
    struct {
      math::CRect Rectangle;
      bool Moved;
      bool Resized;
    };
    int32_t Position[2];
    int32_t Data;
    int32_t HumanEdit;
    struct {
      int32_t Key;
      int32_t KeyboardState;
    };
    struct {
      int32_t ID;
      int32_t Flags;
    };
  };

  CWBMessage();
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target, int32_t x);
  CWBMessage(CWBApplication* App, WBMESSAGE Message, WBGUID Target, int32_t x,
             int32_t y);
  CWBMessage(const CWBMessage&) = default;
  CWBMessage(CWBMessage&&) = default;
  CWBMessage& operator=(const CWBMessage&) = default;
  CWBMessage& operator=(CWBMessage&&) = default;
  ~CWBMessage() = default;

  WBMESSAGE GetMessage() const { return Message; }
  [[nodiscard]] WBGUID GetTarget() const { return Target; }
  [[nodiscard]] math::CPoint GetPosition() const {
    return math::CPoint(Position[0], Position[1]);
  }

  bool IsTargetID(std::string_view Name);
  std::string GetTargetID();

  bool IsMouseMessage();
};
