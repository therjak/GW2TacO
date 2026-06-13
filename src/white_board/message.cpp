module;

#include <string_view>
#include <utility>

module whiteboard;

import :application;
import :gui_item;
import :message;
import :message_enum;

namespace gui {

Message::Message() : app_(nullptr), message_(WBM_NONE), target_(0) {}

Message::Message(Application* app, MessageType message, WBGUID target)
    : app_(app), message_(message), target_(target) {}

Message::Message(Application* app, MessageType message, WBGUID target,
                 int32_t x)
    : app_(app), message_(message), target_(target), data_(x) {}

Message::Message(Application* app, MessageType message, WBGUID target,
                 int32_t x, int32_t y)
    : app_(app), message_(message), target_(target), data_(std::array{x, y}) {}

Message::Message(Application* app, MessageType message, WBGUID target,
                 keyboard&& kb)
    : app_(app), message_(message), target_(target), data_(std::move(kb)) {}

Message::Message(Application* app, MessageType message, WBGUID target,
                 menucontext&& mc)
    : app_(app), message_(message), target_(target), data_(std::move(mc)) {}

Message::Message(Application* app, MessageType message, WBGUID target,
                 const math::Rect rec, bool moved, bool resized)
    : app_(app),
      message_(message),
      target_(target),
      data_(reposition{rec, moved, resized}) {}

bool Message::IsMouseMessage() {
  return message_ >= WBM_MOUSEMOVE && message_ <= WBM_MOUSEWHEEL;
}

bool Message::IsTargetID(std::string_view Name) {
  if (!app_) return false;
  Widget* i = app_->FindItemByGuid(target_);
  if (!i) return false;
  return i->GetID() == Name;
}

std::string_view Message::GetTargetID() {
  if (!app_) return "";
  Widget* i = app_->FindItemByGuid(target_);
  if (!i) return "";
  return i->GetID();
}

}  // namespace gui
