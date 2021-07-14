#include "cssitem.h"

IWBCSS::IWBCSS() {}

IWBCSS::~IWBCSS() {}

void IWBCSS::AddClass(std::string_view s) {
  if (!HasClass(s)) {
    aClasses.emplace_back(s);
  }
}

void IWBCSS::RemoveClass(std::string_view s) {
  aClasses.erase(std::remove(aClasses.begin(), aClasses.end(), s));
}

void IWBCSS::ToggleClass(std::string_view s) {
  if (HasClass(s))
    RemoveClass(s);
  else
    AddClass(s);
}

bool IWBCSS::HasClass(std::string_view s) {
  return std::find(aClasses.begin(), aClasses.end(), s) != aClasses.end();
}

void IWBCSS::SetID(std::string_view s) { sID = s; }

std::string& IWBCSS::GetID() { return sID; }

bool IWBCSS::IsFitForSelector(std::string_view selector) {
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

TBOOL IWBCSS::ApplyStyle(std::string_view prop, std::string_view value,
                         const std::vector<std::string>& Pseudo) {
  return false;
}

std::string IWBCSS::GetClassString() {
  std::string ret;
  for (const auto& p : aClasses) {
    if (!ret.empty()) {
      ret.append(" ");
    }
    ret.append(p);
  }
  return ret;
}
