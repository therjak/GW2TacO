module;
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/util/xml_document.h"

export module taco.language;

export class Language {
 public:
  std::string name;
  std::unordered_map<std::string, std::string> dict;
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

  size_t activeLanguageIdx = 0;
  std::vector<Language> languages;

  std::vector<int> usedGlyphs;
};

export extern std::unique_ptr<Localization> localization;

export template <class... Args>
std::string DICT(const std::string_view token, Args&&... args) {
  return localization->Localize(token, args...);
}
