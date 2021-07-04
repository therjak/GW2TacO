#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

#include <vector>

class Language
{
public:

  CString name;
  CDictionary<CString, CString> dict;
};

class Localization
{
  size_t activeLanguageIdx = 0;
  std::vector< Language > languages;

  void ImportFile( const CString& file );
  void ImportLanguage( CXMLDocument& d );

  std::vector<int> usedGlyphs;

public:

  Localization();

  void SetActiveLanguage( const CString& language );
  CStringArray GetLanguages();

  void Import();
  CString Localize( const char* token, const CString& fallback = CString( "" ) );
  CString Localize( const CString& token, const CString& fallback = CString( "" ) );

  int GetActiveLanguageIndex();
  std::vector<int>& GetUsedGlyphs();
  void ProcessStringForUsedGlyphs( CString& string );
};

extern Localization* localization;
#define DICT( token, ... ) localization->Localize( token, __VA_ARGS__ )
