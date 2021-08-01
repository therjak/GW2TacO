#pragma once
#include "GuiItem.h"

enum WBMESSAGE {
  WBM_NONE = 0,
  WBM_REPOSITION = 0x100,
  WBM_ROOTRESIZE,
  WBM_HIDE,
  WBM_UNHIDE,
  WBM_CLOSE,
  WBM_CONTEXTMESSAGE,
  WBM_HSCROLL,
  WBM_VSCROLL,
  WBM_KEYDOWN,
  WBM_KEYUP,
  WBM_CHAR,
  WBM_FOCUSLOST,
  WBM_FOCUSGAINED,
  WBM_CONTENTOFFSETCHANGE,
  WBM_CLIENTAREACHANGED,
  WBM_REBUILDCONTEXTITEM,

  WBM_MOUSEMOVE = 0x1000,
  WBM_LEFTBUTTONDOWN,
  WBM_LEFTBUTTONREPEAT,
  WBM_LEFTBUTTONUP,
  WBM_RIGHTBUTTONDOWN,
  WBM_RIGHTBUTTONREPEAT,
  WBM_RIGHTBUTTONUP,
  WBM_MIDDLEBUTTONDOWN,
  WBM_MIDDLEBUTTONREPEAT,
  WBM_MIDDLEBUTTONUP,
  WBM_MOUSEWHEEL,
  WBM_LEFTBUTTONDBLCLK,
  WBM_RIGHTBUTTONDBLCLK,
  WBM_MIDDLEBUTTONDBLCLK,

  WBM_COMMAND = 0x2000,
  WBM_SELECTITEM,
  WBM_ITEMSELECTED,
  WBM_VALUECHANGED,
  WBM_TEXTCHANGED,
  WBM_ITEMRENAMED,
  WBM_SELECTIONCHANGE,
  WBM_NEEDSNUMPADTEXTUPDATE,
  WBM_WINDOWDRAGSTOPPED,

  WBM_GAMEPAD_DPAD_UP = 0x3000,
  WBM_GAMEPAD_DPAD_DOWN,
  WBM_GAMEPAD_DPAD_LEFT,
  WBM_GAMEPAD_DPAD_RIGHT,
  WBM_GAMEPAD_START,
  WBM_GAMEPAD_BACK,
  WBM_GAMEPAD_LEFT_THUMB,
  WBM_GAMEPAD_RIGHT_THUMB,
  WBM_GAMEPAD_LEFT_SHOULDER,
  WBM_GAMEPAD_RIGHT_SHOULDER,
  WBM_GAMEPAD_A,
  WBM_GAMEPAD_B,
  WBM_GAMEPAD_X,
  WBM_GAMEPAD_Y,
  WBM_GAMEPAD_LEFT_TRIGGER,
  WBM_GAMEPAD_RIGHT_TRIGGER,
  WBM_GAMEPAD_LEFT_THUMB_X,
  WBM_GAMEPAD_LEFT_THUMB_Y,
  WBM_GAMEPAD_RIGHT_THUMB_X,
  WBM_GAMEPAD_RIGHT_THUMB_Y,

};

constexpr int32_t WB_KBSTATE_ALT = 0x00000001;
constexpr int32_t WB_KBSTATE_CTRL = 0x00000002;
constexpr int32_t WB_KBSTATE_SHIFT = 0x00000004;

class CWBMessage final {
  CWBApplication* App;
  WBMESSAGE Message;
  WBGUID Target;

 public:
  union {
    struct {
      CRect Rectangle;
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

  INLINE WBMESSAGE GetMessage() const { return Message; }
  INLINE WBGUID GetTarget() const { return Target; }
  INLINE CPoint GetPosition() const { return CPoint(Position[0], Position[1]); }

  bool IsTargetID(std::string_view Name);
  std::string GetTargetID();

  bool IsMouseMessage();
};
