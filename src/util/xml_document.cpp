#include "xml_document.h"

#include <combaseapi.h>
#include <objbase.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/base/read_file.h"
#include "src/util/RapidXML/rapidxml.hpp"
#include "src/util/RapidXML/rapidxml_print.hpp"
#include "src/util/xml_node.h"

CXMLDocument::CXMLDocument() = default;

CXMLDocument::~CXMLDocument() { CoUninitialize(); }

bool CXMLDocument::LoadFromFile(std::string_view szFileName) {
  memString = baselib::ReadFile(szFileName);
  if (memString.empty()) {
    return false;
  }

  try {
    doc.parse<0>(memString.data());
  } catch (const std::exception& e) {
    LOG_ERR("[XML] Failed to load document: %s", e.what());
    return false;
  }

  return true;
}

// throws HRESULT as a long on FAILED
#define HRCALL(a)                       \
  {                                     \
    HRESULT __hr;                       \
    __hr = (a);                         \
    if (FAILED(__hr)) throw(long) __hr; \
  }

// throws HRESULT as a long on FAILED or if !b is true
#define HRCALLRV(a, b)                          \
  {                                             \
    HRESULT __hr;                               \
    __hr = (a);                                 \
    if (FAILED(__hr) || !(b)) throw(long) __hr; \
  }

bool CXMLDocument::LoadFromString(std::string_view s) {
  memString = s;

  try {
    doc.parse<0>(memString.data());
  } catch (const std::exception& e) {
    LOG_ERR("[XML] Failed to load document: %s", e.what());
    return false;
  }

  return true;
}

CXMLNode CXMLDocument::GetDocumentNode() { return CXMLNode(&doc, this, 0); }

std::string CXMLDocument::SaveToString() {
  std::stringstream ss;
  rapidxml::print<char>(ss, *doc.first_node());
  return ss.str();
}

bool CXMLDocument::SaveToFile(std::string_view sz) {
  auto s = SaveToString();

  HANDLE h = CreateFile(sz.data(), GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS,
                        NULL, nullptr);
  if (h == INVALID_HANDLE_VALUE) return false;
  DWORD b;
  WriteFile(h, s.c_str(), s.length(), &b, nullptr);
  CloseHandle(h);

  return true;
}
