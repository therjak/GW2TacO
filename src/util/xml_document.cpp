#include "xml_document.h"

#include <windows.h>
//
#include <combaseapi.h>
#include <fileapi.h>
#include <objbase.h>

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
  HANDLE hFile =
      CreateFile(name.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 nullptr, OPEN_EXISTING, 0, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    return {};
  }
  int32_t size = GetFileSize(hFile, nullptr);
  auto ret = std::string(size, 0);
  DWORD nRead = 0;
  bool b = ::ReadFile(hFile, ret.data(), size, &nRead, nullptr);
  CloseHandle(hFile);
  if (!b || nRead != size) {
    return {};
  }
  return ret;
}
}  // namespace

CXMLDocument::CXMLDocument()
    : doc(std::make_unique<rapidxml::xml_document<>>()) {}

CXMLDocument::~CXMLDocument() { CoUninitialize(); }

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

  HANDLE h = CreateFile(sz.data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0,
                        nullptr);
  if (h == INVALID_HANDLE_VALUE) return false;
  DWORD b = 0;
  WriteFile(h, s.c_str(), s.length(), &b, nullptr);
  CloseHandle(h);

  return true;
}
