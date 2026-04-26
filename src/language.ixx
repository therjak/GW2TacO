module;
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module taco.language;

import xml;

export class Language {
 public:
  std::string name_;
  std::unordered_map<std::string, std::string> dict_;
};

export class Localization {
 public:
  Localization();

  void SetActiveLanguage(std::string_view language);
  std::vector<std::string> GetLanguages();

  void Import();
  std::string Localize(std::string_view token,
                       std::string_view fallback = (""));

  int GetActiveLanguageIndex();
  std::vector<int>& GetUsedGlyphs();
  void ProcessStringForUsedGlyphs(std::string_view string);

 private:
  void ImportFile(std::string_view file);
  void ImportLanguage(CXMLDocument& d);

  size_t active_language_idx_ = 0;
  std::vector<Language> languages_;

  std::vector<int> used_glyphs_;
};

export std::unique_ptr<Localization> localization;

export template <class... Args>
std::string DICT(const std::string_view token, Args&&... args) {
  return localization->Localize(token, args...);
}
