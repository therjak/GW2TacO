#include "../UtilLib/RapidXML/rapidxml.hpp"
#include "../UtilLib/RapidXML/rapidxml_print.hpp"

#include <sstream>
#include <string>
#include <string_view>
#include <cstdio>

#include "../BaseLib/read_file.h"
#include "XMLDocument.h"

CXMLDocument::CXMLDocument(void)
{
}

CXMLDocument::~CXMLDocument(void)
{
	CoUninitialize();
}

bool CXMLDocument::LoadFromFile(const std::string_view& szFileName) {
  memString = baselib::ReadFile(szFileName);
  if (memString.empty()) {
    return false;
  }

  try
  {
    doc.parse<0>( memString.data() );
  }
  catch ( const std::exception &e )
  {
    LOG_ERR( "[XML] Failed to load document: %s", e.what() );
    return false;
  }

	return true;
}

// throws HRESULT as a long on FAILED
#define HRCALL( a ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) ) \
	throw (long)__hr; \
}

// throws HRESULT as a long on FAILED or if !b is true
#define HRCALLRV( a, b ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) || !(b) ) \
	throw (long)__hr; \
}

bool CXMLDocument::LoadFromString(const std::string_view& s) {
  memString = s;

  try
  {
    doc.parse<0>( memString.data() );
  }
  catch ( const std::exception &e )
  {
    LOG_ERR( "[XML] Failed to load document: %s", e.what() );
    return false;
  }

  return true;
}

CXMLNode CXMLDocument::GetDocumentNode()
{
  return CXMLNode( &doc, this, 0 );
}

std::string CXMLDocument::SaveToString()
{
  std::stringstream ss;
  print<char>( ss, *doc.first_node() );
  return ss.str();
}

bool CXMLDocument::SaveToFile(const std::string_view& sz) {
	auto s = SaveToString();

	HANDLE h = CreateFile(sz.data(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;
	DWORD b;
	WriteFile(h, s.c_str(), s.length(), &b, NULL);
	CloseHandle(h);

	return true;
}
