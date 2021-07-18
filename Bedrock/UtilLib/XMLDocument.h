#pragma once

#include <string>
#include <string_view>

#include "../UtilLib/RapidXML/rapidxml.hpp"
#include "XMLNode.h"

class CXMLDocument {
  rapidxml::xml_document<> doc;
  std::string memString;

 public:
  CXMLDocument();
  ~CXMLDocument();

  bool LoadFromFile(std::string_view);
  bool LoadFromString(std::string_view);
  bool SaveToFile(std::string_view);
  std::string SaveToString();
  CXMLNode GetDocumentNode();
};
