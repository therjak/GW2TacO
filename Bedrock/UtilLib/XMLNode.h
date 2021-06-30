#pragma once
#include "../UtilLib/RapidXML/rapidxml.hpp"

#include "../BaseLib/BaseLib.h"

#include <string_view>
#include <memory>
#include <cstdint>

class CXMLDocument;

class CXMLNode
{
public:
  CXMLNode( void );
  CXMLNode( const CXMLNode &Original );
  CXMLNode(rapidxml::xml_node<char>*, CXMLDocument*, int32_t);
  virtual ~CXMLNode();

  CXMLNode operator=( const CXMLNode Original );

  int32_t GetChildCount();
  int32_t GetChildCount( TCHAR * );
  CXMLNode GetChild( int32_t );
  CXMLNode GetChild( TCHAR * );
  CXMLNode GetChild( TCHAR *, int32_t );

  bool Next( CXMLNode& out, TCHAR* );

  int32_t IsValid();

  CString GetNodeName();

  void GetText( TCHAR*, int32_t );
  CString GetText();
  bool GetValue( int32_t &Int );
  bool GetValue( TBOOL &Int );
  bool GetValue( float &Float );

  bool GetAttribute( TCHAR * szAttribute, TCHAR * szBuffer, int32_t nBufferSize );
  CString GetAttribute( TCHAR * szAttribute );
  void GetAttributeAsInteger( TCHAR * szAttribute, int32_t * nValue );
  void GetAttributeAsFloat( TCHAR * szAttribute, float * fValue );
  CString GetAttributeAsString( TCHAR * szAttribute );
  bool HasAttribute( TCHAR * szAttribute );

  CXMLNode& AddChild( TCHAR*);
  void SetText( const TCHAR* );
  void SetText( CString &s );
  void SetInt( int32_t Int );
  void SetFloat( float Float );
  void SetAttribute( const std::string_view& szAttributeName, const std::string_view& szValue );
  void SetAttributeFromInteger( const std::string_view& szAttributeName, int32_t nValue );
  void SetAttributeFromFloat( const std::string_view& szAttributeName, float nValue );


private:

  int32_t childCount = -1;
  CDictionary< int32_t, int32_t > childCounts;
  std::vector<std::unique_ptr<std::string>> stringStore;
  std::vector<std::unique_ptr<CXMLNode>> children;

  std::string value;

  int32_t nLevel;
  rapidxml::xml_node<char>* pNode;
  CXMLDocument *pDoc;
};
