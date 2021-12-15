#include "src/white_board/message.h"

#include "src/white_board/application.h"

CWBMessage::CWBMessage() {
  App = nullptr;
  Message = WBM_NONE;
  Target = 0;
}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target) {
  App = app;
  Message = message;
  Target = target;
}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       int32_t x) {
  App = app;
  Message = message;
  Target = target;
  Data = x;
}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       int32_t x, int32_t y) {
  App = app;
  Message = message;
  Target = target;
  Position[0] = x;
  Position[1] = y;
}

bool CWBMessage::IsMouseMessage() {
  return Message >= WBM_MOUSEMOVE && Message <= WBM_MOUSEWHEEL;
}

bool CWBMessage::IsTargetID(std::string_view Name) {
  if (!App) return false;
  CWBItem* i = App->FindItemByGuid(Target);
  if (!i) return false;
  return i->GetID() == Name;
}

std::string CWBMessage::GetTargetID() {
  if (!App) return _T( "" );
  CWBItem* i = App->FindItemByGuid(Target);
  if (!i) return _T( "" );
  return i->GetID();
}
