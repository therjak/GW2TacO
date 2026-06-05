module;
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module whiteboard:style_manager;

import :gui_item;

export namespace gui {

class StyleManager {
 public:
  StyleManager();
  ~StyleManager();
  void Reset();
  bool ParseStyleData(std::string_view s);

  void ApplyStyles(Widget* pRootItem);

  static std::vector<Widget*> GetElementsBySelector(Widget* pRootItem,
                                                    std::string_view selector);
  static void ApplyStylesFromDeclarations(Widget* pRootItem,
                                          std::string_view sDeclarations);

 private:
  static void ParseDeclarations(
      std::string_view s,
      std::unordered_map<std::string, std::string>* dRuleset);
  static void CollectElementsBySimpleSelector(Widget* pItem,
                                              std::vector<Widget*>* itemset,
                                              std::string_view selector,
                                              bool bIncludeRoot);

  std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
      rules_;
};

}  // namespace gui
