#pragma once
#include "../UtilLib/RapidXML/rapidxml.hpp"

#include "../BaseLib/BaseLib.h"

//#import "msxml3.dll" named_guids raw_interfaces_only
//#include "msxml3.tlh"
using namespace rapidxml;

//using namespace MSXML2;

class CXMLDocument;

class CXMLNode
{
public:
  CXMLNode( void );
  CXMLNode( const CXMLNode &Original );
  //CXMLNode(MSXML2::IXMLDOMNode *, CXMLDocument *, int32_t);
  CXMLNode( xml_node<char> *, CXMLDocument *, int32_t );
  virtual ~CXMLNode();

  CXMLNode operator=( const CXMLNode Original );

  int32_t GetChildCount();
  int32_t GetChildCount( TCHAR * );
  CXMLNode GetChild( int32_t );
  CXMLNode GetChild( TCHAR * );
  CXMLNode GetChild( TCHAR *, int32_t );

  TBOOL Next( CXMLNode& out );
  TBOOL Next( CXMLNode& out, TCHAR* );

  int32_t IsValid();

  CString GetNodeName();

  void GetText( TCHAR*, int32_t );
  CString GetText();
  TBOOL GetValue( int32_t &Int );
  TBOOL GetValue( TBOOL &Int );
  TBOOL GetValue( TF32 &Float );

  TBOOL GetAttribute( TCHAR * szAttribute, TCHAR * szBuffer, int32_t nBufferSize );
  CString GetAttribute( TCHAR * szAttribute );
  void GetAttributeAsInteger( TCHAR * szAttribute, int32_t * nValue );
  void GetAttributeAsFloat( TCHAR * szAttribute, TF32 * fValue );
  CString GetAttributeAsString( TCHAR * szAttribute );
  TBOOL HasAttribute( TCHAR * szAttribute );

  //void FlushNode();
  CXMLNode& AddChild( TCHAR*, TBOOL PostEnter = true );
  void SetText( const TCHAR* );
  void SetText( CString &s );
  void SetInt( int32_t Int );
  void SetFloat( TF32 Float );
  void SetAttribute( TCHAR * szAttributeName, const TCHAR * szValue );
  void SetAttributeFromInteger( TCHAR * szAttributeName, int32_t nValue );
  void SetAttributeFromFloat( TCHAR * szAttributeName, TF32 nValue );

  //MSXML2::IXMLDOMNode * GetNode() { return pNode; }

private:

  int32_t childCount = -1;
  CDictionary< int32_t, int32_t > childCounts;
  CArray<CString*> stringStore;
  CArray<CXMLNode*> children;

  CString value;

  int32_t nLevel;
  xml_node<char>* pNode;
  //MSXML2::IXMLDOMNode * pNode;
  //MSXML2::IXMLDOMDocument * pDoc;
  CXMLDocument *pDoc;
};
