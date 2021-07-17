#pragma once
#include "GuiItem.h"

#include <string>
#include <unordered_map>

class CStyleManager
{
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      dRules;

public:
 CStyleManager();
 ~CStyleManager();
 void Reset();
 static void ParseDeclarations(
     std::string_view s,
     std::unordered_map<std::string, std::string> &dRuleset);
 bool ParseStyleData(std::string_view s);

 void ApplyStyles(CWBItem *pRootItem);

 static CArray<CWBItem *> GetElementsBySelector(CWBItem *pRootItem,
                                                std::string_view selector);
 static void CollectElementsBySimpleSelector(CWBItem *pItem,
                                             CArray<CWBItem *> &itemset,
                                             std::string_view selector,
                                             bool bIncludeRoot);
 static void ApplyStylesFromDeclarations(CWBItem *pRootItem,
                                         std::string_view sDeclarations);
};

