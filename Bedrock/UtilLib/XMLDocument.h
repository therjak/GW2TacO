#pragma once

#include <string>
#include <string_view>

#include "XMLNode.h"
#include "../UtilLib/RapidXML/rapidxml.hpp"

class CXMLDocument {
  rapidxml::xml_document<> doc;
  std::string memString;

public:
  CXMLDocument( void );
  ~CXMLDocument( void );

  bool LoadFromFile(const std::string_view&);
  bool LoadFromString( const std::string_view& );
  bool SaveToFile(const std::string_view&);
  std::string SaveToString();
  CXMLNode GetDocumentNode();
};
