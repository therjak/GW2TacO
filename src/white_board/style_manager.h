#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "src/white_board/gui_item.h"

class CStyleManager {
 public:
  CStyleManager();
  ~CStyleManager();
  void Reset();
  static void ParseDeclarations(
      std::string_view s,
      std::unordered_map<std::string, std::string>& dRuleset);
  bool ParseStyleData(std::string_view s);

  void ApplyStyles(CWBItem* pRootItem);

  static std::vector<CWBItem*> GetElementsBySelector(CWBItem* pRootItem,
                                                     std::string_view selector);
  static void CollectElementsBySimpleSelector(CWBItem* pItem,
                                              std::vector<CWBItem*>& itemset,
                                              std::string_view selector,
                                              bool bIncludeRoot);
  static void ApplyStylesFromDeclarations(CWBItem* pRootItem,
                                          std::string_view sDeclarations);

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      dRules;
};
