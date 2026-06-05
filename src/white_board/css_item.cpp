module;

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

module whiteboard;

import :css_item;

namespace gui {

CSS::CSS() = default;

CSS::~CSS() = default;

void CSS::AddClass(std::string_view s) {
  if (!HasClass(s)) {
    classes_.emplace_back(s);
  }
}

void CSS::RemoveClass(std::string_view s) {
  classes_.erase(std::remove(classes_.begin(), classes_.end(), s),
                 classes_.end());
}

void CSS::ToggleClass(std::string_view s) {
  if (HasClass(s)) {
    RemoveClass(s);
  } else {
    AddClass(s);
  }
}

bool CSS::HasClass(std::string_view s) {
  return std::find(classes_.begin(), classes_.end(), s) != classes_.end();
}

void CSS::SetID(std::string_view s) { id_ = s; }

std::string_view CSS::GetID() const { return id_; }

bool CSS::IsFitForSelector(std::string_view selector) {
  if (selector.empty()) {
    return false;
  }
  if (selector[0] == '#') {
    if (selector.substr(1) != GetID()) return false;
  } else if (selector[0] == '.') {
    if (!HasClass(selector.substr(1))) return false;
  } else if (selector[0] == '*') {
    return true;
  } else {
    if (!InstanceOf(selector)) return false;
  }
  return true;
}

bool CSS::ApplyStyle(std::string_view prop, std::string_view value,
                     const std::vector<std::string>& Pseudo) {
  return false;
}

std::string CSS::GetClassString() {
  std::string ret;
  for (const auto& p : classes_) {
    if (!ret.empty()) {
      ret.append(" ");
    }
    ret.append(p);
  }
  return ret;
}

}  // namespace gui
