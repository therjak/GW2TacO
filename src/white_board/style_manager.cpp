module;
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/logger.h"
#include "src/base/string_format.h"

module whiteboard;

import :gui_item;
import :style_manager;

namespace gui {

StyleManager::StyleManager() = default;

StyleManager::~StyleManager() = default;

void StyleManager::ParseDeclarations(
    std::string_view s,
    std::unordered_map<std::string, std::string>* dRuleset) {
  auto propertiesArr = Split(s, ";");
  for (const auto& p : propertiesArr) {
    auto prop = Split(p, ":");
    if (prop.size() != static_cast<size_t>(2)) continue;
    std::string key(Trim(prop[0]));
    std::string value(Trim(prop[1]));
    (*dRuleset)[key] = value;
  }
}

bool StyleManager::ParseStyleData(std::string_view s) {
  int nPos = 0;
  std::string style(s);
  while ((nPos = style.find("/*")) != std::string::npos) {
    int nEnd = style.find("*/", nPos);
    if (nEnd == std::string::npos) nEnd = style.size();

    style = style.substr(0, nPos) + style.substr(nEnd + 2);
  }

  auto rules = Split(style, "}");
  for (const auto& r : rules) {
    auto properties = Split(r, "{");
    if (properties.size() != 2) continue;

    std::unordered_map<std::string, std::string> dRuleset;
    ParseDeclarations(properties[1], &dRuleset);

    auto selectors = Split(properties[0], ",");
    for (const auto& s : selectors) {
      std::string selector(Trim(s));
      auto& mr = rules_[selector];
      for (auto& rs : dRuleset) {
        mr.insert_or_assign(rs.first, rs.second);
      }
    }
  }
  return true;
}

void StyleManager::Reset() { rules_.clear(); }

void StyleManager::CollectElementsBySimpleSelector(
    Widget* pItem, std::vector<Widget*>* itemset, std::string_view selector,
    bool bIncludeRoot) {
  if (bIncludeRoot && pItem->IsFitForSelector(selector)) {
    itemset->emplace_back(pItem);
  }

  for (unsigned int i = 0; i < pItem->NumChildren(); i++) {
    CollectElementsBySimpleSelector(pItem->GetChild(i), itemset, selector,
                                    true);
  }
}

std::vector<Widget*> StyleManager::GetElementsBySelector(
    Widget* pRootItem, std::string_view selector) {
  std::vector<Widget*> result;
  result.emplace_back(pRootItem);
  auto components = Split(selector, " ");
  for (size_t i = 0; i < components.size(); i++) {
    if (components[i].size() < 1) continue;
    std::vector<Widget*> narrowResult;
    for (const auto& r : result) {
      CollectElementsBySimpleSelector(r, &narrowResult, components[i], i == 0);
    }
    result = narrowResult;
  }

  return result;
}

void StyleManager::ApplyStyles(Widget* pRootItem) {
  for (auto& r : rules_) {
    std::string selector = r.first;
    std::unordered_map<std::string, std::string>& rules = r.second;

    auto PseudoTags = Split(selector, ":");
    std::vector<Widget*> items =
        GetElementsBySelector(pRootItem, PseudoTags[0]);

    for (auto& item : items) {
      for (auto& ry : rules) {
        std::string rule = ry.first;
        std::string value = ry.second;

        if (!item->ApplyStyle(rule, value, PseudoTags)) {
          Log_Dbg("[css] rule {:s} was not handled by item '{:s}' #{:s} .{:s}",
                  rule, item->GetType(), item->GetID(), item->GetClassString());
        }
      }
    }
  }
}

void StyleManager::ApplyStylesFromDeclarations(Widget* pRootItem,
                                               std::string_view sDeclarations) {
  std::unordered_map<std::string, std::string> dRuleset;
  ParseDeclarations(sDeclarations, &dRuleset);

  for (auto& r : dRuleset) {
    std::string rule = r.first;
    std::string value = r.second;
    auto PseudoTags = Split(rule, ":");

    if (!pRootItem->ApplyStyle(rule, value, PseudoTags)) {
      Log_Dbg("[css] rule '{:s}' was not handled by item '{:s}' #{:s} .{:s}",
              rule, pRootItem->GetType(), pRootItem->GetID(),
              pRootItem->GetClassString());
    }
  }
}

}  // namespace gui
