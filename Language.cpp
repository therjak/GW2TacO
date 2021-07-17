#include "Language.h"
#include "OverlayConfig.h"

#include <algorithm>
#include <string_view>
#include <string>
#include <functional>
#include <vector>

typedef std::function<bool(uint32_t)> UTF8CHARCALLBACK;
void DecodeUtf8(std::string_view Input, UTF8CHARCALLBACK callback) {
  auto it = Input.begin();
  while (it != Input.end()) {
    int Char = *it;
    if ((Char & 0x80))  // decode utf-8
    {
      if ((Char & 0xe0) == 0xc0) {
        Char = Char & ((1 << 5) - 1);
        for (int z = 0; z < 1; z++) {
          if ((it + 1) != Input.end()) {
            Char = (Char << 6) + ((*(it + 1)) & 0x3f);
            ++it;
          }
        }
      } else if ((Char & 0xf0) == 0xe0) {
        Char = Char & ((1 << 4) - 1);
        for (int z = 0; z < 2; z++) {
          if ((it + 1) != Input.end()) {
            Char = (Char << 6) + ((*(it + 1)) & 0x3f);
            ++it;
          }
        }
      } else if ((Char & 0xf8) == 0xf0) {
        Char = Char & ((1 << 3) - 1);
        for (int z = 0; z < 3; z++) {
          if ((it + 1) != Input.end()) {
            Char = (Char << 6) + ((*(it + 1)) & 0x3f);
            ++it;
          }
        }
      } else if ((Char & 0xfc) == 0xf8) {
        Char = Char & ((1 << 2) - 1);
        for (int z = 0; z < 4; z++) {
          if ((it + 1) != Input.end()) {
            Char = (Char << 6) + ((*(it + 1)) & 0x3f);
            ++it;
          }
        }
      } else if ((Char & 0xfe) == 0xfc) {
        Char = Char & ((1 << 1) - 1);
        for (int z = 0; z < 5; z++) {
          if ((it + 1) != Input.end()) {
            Char = (Char << 6) + ((*(it + 1)) & 0x3f);
            ++it;
          }
        }
      }
    }

    if (!callback(Char)) return;
    ++it;
  }
}


Localization* localization = nullptr;

void Localization::ImportFile( std::string_view s )
{
  CXMLDocument d;
  if ( !d.LoadFromFile( s ) ) return;
  ImportLanguage( d );
}

void Localization::ImportLanguage( CXMLDocument& d )
{
  if ( !d.GetDocumentNode().GetChildCount( "TacOLanguageData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "TacOLanguageData" );

  std::string language;

  if ( root.HasAttribute( "language" ) ) 
    language = root.GetAttributeAsString( "language" );
  else
  {
    LOG_ERR( "[GW2TacO] Language data file didn't specify the language." );
    return;
  }

  std::transform(language.begin(), language.end(), language.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  

  int langIdx = - 1;

  for ( int x = 0; x < languages.size(); x++ )
  {
    if ( languages[ x ].name == language )
      langIdx = x;
  }

  if ( langIdx < 0 )
  {
    Language lang;
    lang.name = language;
    langIdx = languages.size();
    languages.push_back(lang);
  }
  
  auto& lang = languages[ langIdx ].dict;

  int tokenCount = root.GetChildCount( "token" );

  for ( int x = 0; x < tokenCount; x++ )
  {
    auto& token = root.GetChild( "token", x );
    if ( !token.HasAttribute( "key" ) || !token.HasAttribute( "value" ) )
      continue;

    std::string key = token.GetAttributeAsString( "key" );
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::string value = token.GetAttributeAsString( "value" );     

    lang[ key ] = value;

    ProcessStringForUsedGlyphs( value );
  }

  ProcessStringForUsedGlyphs( language );
}

Localization::Localization()
{
  for ( int x = 0; x <= 0x460; x++ )
    usedGlyphs.push_back( x );
}

void Localization::SetActiveLanguage( std::string_view language )
{
  std::string lang(language);
  std::transform(lang.begin(), lang.end(), lang.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  for ( size_t x = 0; x < languages.size(); x++ )
  {
    if ( languages[ x ].name == lang )
    {
      activeLanguageIdx = x;
      SetConfigString( "language", lang );
      LOG_NFO( "[GW2TacO] Setting TacO language to %s", std::string(language).c_str() );
      return;
    }
  }

  activeLanguageIdx = 0;
}

std::vector<std::string> Localization::GetLanguages() {
  std::vector<std::string> langs;

  for ( const auto& l: languages )
    langs.push_back(l.name);

  for ( auto& l: langs )
    l[ 0 ] = std::toupper( l[ 0 ] );

  return langs;
}

std::string str_tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } 
  );
  return s;
}

const std::string taco_lang_en = str_tolower("TacO_Language_en.xml");

void Localization::Import()
{
  ImportFile( "TacO_Language_en.xml" );

  CFileList list;
  list.ExpandSearch( "TacO_Language_*.xml", ".", false );
  for ( const auto& f : list.Files )
    if ( str_tolower(f.FileName) == taco_lang_en )
    {
      ImportFile( (f.Path + f.FileName).c_str() );
    }

  if ( HasConfigString( "language" ) )
  {
    SetActiveLanguage( GetConfigString( "language" ) );
  }
}

std::string Localization::Localize(std::string_view token,
                                   std::string_view fallback) {
  std::string tokenString( token );
  std::transform(tokenString.begin(), tokenString.end(), tokenString.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string rawToken;
  if ( !fallback.empty() )
    rawToken = fallback;
  else
    rawToken = "[" + tokenString + "]";

  if ( activeLanguageIdx >= languages.size() )
    return rawToken;

  auto& lang = languages[ activeLanguageIdx ].dict;

  if ( lang.find( tokenString ) == lang.end() )
  {
    static std::vector<std::string> dumpedStrings;

    if ( std::find(dumpedStrings.begin(), dumpedStrings.end(), tokenString ) == dumpedStrings.end() )
    {
      LOG_WARN( "[GW2TacO] Translation for token '%s' is not available in the %s language.", tokenString.c_str(), languages[ activeLanguageIdx ].name.c_str() );
      dumpedStrings.push_back(tokenString);
    }

    return rawToken;
  }

  return lang[ tokenString ];
}

int Localization::GetActiveLanguageIndex()
{
  return activeLanguageIdx;
}

std::vector<int>& Localization::GetUsedGlyphs()
{
  return usedGlyphs;
}

void Localization::ProcessStringForUsedGlyphs(std::string_view string) {
  DecodeUtf8(string, [&](uint32_t Char) -> bool {
    if (std::find(usedGlyphs.begin(), usedGlyphs.end(), Char) ==
        usedGlyphs.end()) {
      usedGlyphs.push_back(Char);
    }
    return true;
  });
}
