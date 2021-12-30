#include "src/white_board/style_manager.h"

#include "src/base/logger.h"
#include "src/base/string_format.h"

CStyleManager::CStyleManager() = default;

CStyleManager::~CStyleManager() = default;

void CStyleManager::ParseDeclarations(
    std::string_view s,
    std::unordered_map<std::string, std::string>& dRuleset) {
  auto propertiesArr = Split(s, ";");
  for (const auto& p : propertiesArr) {
    auto prop = Split(p, ":");
    if (prop.size() != 2) continue;
    std::string key(Trim(prop[0]));
    std::string value(Trim(prop[1]));
    dRuleset[key] = value;
  }
}

bool CStyleManager::ParseStyleData(std::string_view s) {
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
    ParseDeclarations(properties[1], dRuleset);

    auto selectors = Split(properties[0], ",");
    for (const auto& s : selectors) {
      std::string selector(Trim(s));
      auto& mr = dRules[selector];
      for (auto& rs : dRuleset) {
        mr.insert_or_assign(rs.first, rs.second);
      }
    }
  }
  return true;
}

void CStyleManager::Reset() { dRules.clear(); }

void CStyleManager::CollectElementsBySimpleSelector(
    CWBItem* pItem, std::vector<CWBItem*>& itemset, std::string_view selector,
    bool bIncludeRoot) {
  if (bIncludeRoot && pItem->IsFitForSelector(selector))
    itemset.emplace_back(pItem);

  for (unsigned int i = 0; i < pItem->NumChildren(); i++) {
    CollectElementsBySimpleSelector((CWBItem*)pItem->GetChild(i), itemset,
                                    selector, true);
  }
}

std::vector<CWBItem*> CStyleManager::GetElementsBySelector(
    CWBItem* pRootItem, std::string_view selector) {
  std::vector<CWBItem*> result;
  result.emplace_back((CWBItem*)pRootItem);
  auto components = Split(selector, " ");
  for (size_t i = 0; i < components.size(); i++) {
    if (components[i].size() < 1) continue;
    std::vector<CWBItem*> narrowResult;
    for (const auto& r : result) {
      CollectElementsBySimpleSelector(r, narrowResult, components[i], i == 0);
    }
    result = narrowResult;
  }

  return result;
}

void CStyleManager::ApplyStyles(CWBItem* pRootItem) {
  for (auto& r : dRules) {
    std::string selector = r.first;
    std::unordered_map<std::string, std::string>& rules = r.second;

    auto PseudoTags = Split(selector, ":");
    std::vector<CWBItem*> items =
        GetElementsBySelector(pRootItem, PseudoTags[0]);

    for (auto& item : items) {
      for (auto& ry : rules) {
        std::string rule = ry.first;
        std::string value = ry.second;

        if (!item->ApplyStyle(rule, value, PseudoTags)) {
          LOG_DBG("[css] rule %s was not handled by item '%s' #%s .%s",
                  rule.c_str(), item->GetType().c_str(), item->GetID().c_str(),
                  item->GetClassString().c_str());
        }
      }
    }
  }
}

void CStyleManager::ApplyStylesFromDeclarations(
    CWBItem* pRootItem, std::string_view sDeclarations) {
  std::unordered_map<std::string, std::string> dRuleset;
  ParseDeclarations(sDeclarations, dRuleset);

  for (auto& r : dRuleset) {
    std::string rule = r.first;
    std::string value = r.second;
    auto PseudoTags = Split(rule, ":");

    if (!pRootItem->ApplyStyle(rule, value, PseudoTags)) {
      LOG_DBG("[css] rule '%s' was not handled by item '%s' #%s .%s",
              rule.c_str(), pRootItem->GetType().c_str(),
              pRootItem->GetID().c_str(), pRootItem->GetClassString().c_str());
    }
  }
}
