#include "xml_document.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/util/RapidXML/rapidxml.hpp"
#include "src/util/RapidXML/rapidxml_print.hpp"
#include "src/util/xml_node.h"

namespace {
std::string ReadFile(std::string_view name) {
  std::FILE* fp = std::fopen(name.data(), "rb");
  if (!fp) {
    return {};
  }
  std::string contents;
  std::fseek(fp, 0, SEEK_END);
  contents.resize(std::ftell(fp));
  std::rewind(fp);
  std::fread(contents.data(), 1, contents.size(), fp);
  std::fclose(fp);
  return (contents);
}
}  // namespace

CXMLDocument::CXMLDocument()
    : doc(std::make_unique<rapidxml::xml_document<>>()) {}

CXMLDocument::~CXMLDocument() {}

bool CXMLDocument::LoadFromFile(std::string_view szFileName) {
  memString = ReadFile(szFileName);
  if (memString.empty()) {
    return false;
  }

  try {
    doc->parse<0>(memString.data());
  } catch (const std::exception& e) {
    Log_Err("[XML] Failed to load document: {:s}", e.what());
    return false;
  }

  return true;
}

bool CXMLDocument::LoadFromString(std::string_view s) {
  memString = s;

  try {
    doc->parse<0>(memString.data());
  } catch (const std::exception& e) {
    Log_Err("[XML] Failed to load document: {:s}", e.what());
    return false;
  }

  return true;
}

CXMLNode CXMLDocument::GetDocumentNode() { return {doc.get(), this, 0}; }

std::string CXMLDocument::SaveToString() {
  std::stringstream ss;
  rapidxml::print<char>(ss, *doc->first_node());
  return ss.str();
}

bool CXMLDocument::SaveToFile(std::string_view sz) {
  auto s = SaveToString();
  std::FILE* fp = std::fopen(sz.data(), "wb");
  if (!fp) {
    return false;
  }
  std::fwrite(s.c_str(), 1, s.size(), fp);
  std::fclose(fp);
  return true;
}
