#pragma once

#include "../BaseLib/BaseLib.h"
#include "XMLNode.h"

class CXMLDocument
{
  //MSXML2::IXMLDOMDocument2 * pDoc;
  xml_document<> doc;
  CString memString;

public:
  CXMLDocument( void );
  ~CXMLDocument( void );

  TBOOL Allocate();
  TBOOL Cleanup();

  TBOOL LoadFromFile( const TCHAR * );
  TBOOL LoadFromString( CString );
  TBOOL SaveToFile( const TCHAR * );
  CString SaveToString();
  CXMLNode GetDocumentNode();

  //CXMLNode CreateNode(VARIANT type, BSTR Name, int32_t Level);
  //MSXML2::IXMLDOMDocument *GetDoc() { return pDoc; }
};
