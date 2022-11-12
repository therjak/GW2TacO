#include "src/white_board/message.h"

#include "src/white_board/application.h"

CWBMessage::CWBMessage() : app_(nullptr), message_(WBM_NONE), target_(0) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target)
    : app_(app), message_(message), target_(target) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       int32_t x)
    : app_(app), message_(message), target_(target), data_(x) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       int32_t x, int32_t y)
    : app_(app), message_(message), target_(target), data_(std::array{x, y}) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       keyboard&& kb)
    : app_(app), message_(message), target_(target), data_(std::move(kb)) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       menucontext&& mc)
    : app_(app), message_(message), target_(target), data_(std::move(mc)) {}

CWBMessage::CWBMessage(CWBApplication* app, WBMESSAGE message, WBGUID target,
                       const math::CRect rec, bool moved, bool resized)
    : app_(app),
      message_(message),
      target_(target),
      data_(reposition{rec, moved, resized}) {}

bool CWBMessage::IsMouseMessage() {
  return message_ >= WBM_MOUSEMOVE && message_ <= WBM_MOUSEWHEEL;
}

bool CWBMessage::IsTargetID(std::string_view Name) {
  if (!app_) return false;
  CWBItem* i = app_->FindItemByGuid(target_);
  if (!i) return false;
  return i->GetID() == Name;
}

std::string_view CWBMessage::GetTargetID() {
  if (!app_) return "";
  CWBItem* i = app_->FindItemByGuid(target_);
  if (!i) return "";
  return i->GetID();
}
