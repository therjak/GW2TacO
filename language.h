#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/util/xml_document.h"

class Language {
 public:
  std::string name;
  std::unordered_map<std::string, std::string> dict;
};

class Localization {
  size_t activeLanguageIdx = 0;
  std::vector<Language> languages;

  void ImportFile(std::string_view file);
  void ImportLanguage(CXMLDocument& d);

  std::vector<int> usedGlyphs;

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
};

extern std::unique_ptr<Localization> localization;
#define DICT(token, ...) localization->Localize(token, __VA_ARGS__)
