module;

#include <memory>
#include <string>
#include <string_view>

#include "src/util/RapidXML/rapidxml.hpp"

export module xml:document;

import :node;

export class CXMLDocument {
 public:
  CXMLDocument();
  ~CXMLDocument();

  bool LoadFromFile(std::string_view);
  bool LoadFromString(std::string_view);
  bool SaveToFile(std::string_view);
  std::string SaveToString();
  CXMLNode GetDocumentNode();

 private:
  std::unique_ptr<rapidxml::xml_document<>> doc;
  std::string memString;
};
