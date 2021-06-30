#include "XMLNode.h"

#include <string_view>
#include <memory>
#include <cstdint>

#include "../UtilLib/RapidXML/rapidxml.hpp"
#include "XMLDocument.h"

using rapidxml::xml_node;
using rapidxml::xml_node;
using rapidxml::node_type;

int32_t GetStringHash( TCHAR* string )
{
  if ( !string )
    return 0;

  int32_t c;
  TCHAR *str = string;

  int32_t Hash = 5381;
  while ( c = *str++ )
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c

  return Hash;
}

CXMLNode::CXMLNode()
{
	pNode = NULL;
	pDoc = NULL;
	nLevel = 0;
}

CXMLNode::CXMLNode(const CXMLNode &Original)
{
	nLevel = Original.nLevel;
	pNode = Original.pNode;
	pDoc = Original.pDoc;
}

CXMLNode::CXMLNode(xml_node<char>* p, CXMLDocument* d, int32_t l) {
  pNode = p;
  pDoc = d;
  nLevel = l;
}

CXMLNode CXMLNode::operator=( const CXMLNode Original )
{
	//if (pNode) pNode->Release();

	nLevel = Original.nLevel;
	pNode = Original.pNode;
	pDoc = Original.pDoc;
	return *this;
}


CXMLNode::~CXMLNode()
{
}

int32_t CXMLNode::GetChildCount()
{
  if ( !pNode )
    return 0;

  if ( childCount != -1 )
    return childCount;

  auto node = pNode->first_node();
  if ( !node )
  {
    childCount = 0;
    return 0;
  }

  int32_t count = 1;

  while ( node = node->next_sibling() )
    count++;

  childCount = count;

  return count;
}

int32_t CXMLNode::GetChildCount( TCHAR * szNodeName )
{
  int32_t hash = GetStringHash( szNodeName );

  if ( childCounts.HasKey( hash ) )
    return childCounts[ hash ];

  auto node = pNode->first_node( szNodeName );
  if ( !node )
  {
    childCounts[ hash ] = 0;
    return 0;
  }

  int32_t count = 1;

  while ( node = node->next_sibling( szNodeName ) )
    count++;

  childCounts[ hash ] = count;

  return count;
}

CString CXMLNode::GetNodeName()
{
  if ( !pNode )
    return CString();
  return CString( pNode->name() );
}

CXMLNode CXMLNode::GetChild(int32_t n)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node();
  if ( !node )
    return CXMLNode();

  if ( n == 0 )
    return CXMLNode( node, pDoc, nLevel + 1 );

  int32_t count = 1;

  while ( node = node->next_sibling() )
  {
    if ( n == count )
      return CXMLNode( node, pDoc, nLevel + 1 );
    count++;
  }

  return CXMLNode();
}

CXMLNode CXMLNode::GetChild(TCHAR * szNodeName)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node( szNodeName );
  if ( !node )
    return CXMLNode();

  return CXMLNode( node, pDoc, nLevel + 1 );
}

CXMLNode CXMLNode::GetChild(TCHAR * szNodeName, int32_t n)
{
  if ( !pNode )
    return CXMLNode();

  auto node = pNode->first_node( szNodeName );
  if ( !node )
    return CXMLNode();

  if ( n == 0 )
    return CXMLNode( node, pDoc, nLevel + 1 );

  int32_t count = 1;

  while ( node = node->next_sibling( szNodeName ) )
  {
    if ( n == count )
      return CXMLNode( node, pDoc, nLevel + 1 );
    count++;
  }

  return CXMLNode();
}

bool CXMLNode::Next( CXMLNode& out, TCHAR* szNodeName )
{
  if ( !pNode )
    return false;

  auto node = pNode->next_sibling( szNodeName );
  if ( !node )
    return false;

  out = CXMLNode( node, pDoc, nLevel );
  return true;
}

void CXMLNode::GetText(TCHAR * szBuffer, int32_t nBufferSize)
{
  int x = 0;
}
CString CXMLNode::GetText()
{
  return CString();
}

bool CXMLNode::GetAttribute(TCHAR * szAttribute, TCHAR * szBuffer, int32_t nBufferSize)
{
  if ( !pNode )
    return false;

  auto attr = pNode->first_attribute( szAttribute );
  if ( !attr )
    return false;

  _tcsncpy_s( szBuffer, nBufferSize, attr->value(), _TRUNCATE );
  return true;
}


CString CXMLNode::GetAttribute(TCHAR * szAttribute)
{
  if ( !pNode )
    return CString();

  auto attr = pNode->first_attribute( szAttribute );
  if ( !attr )
    return CString();

  return CString( attr->value(), attr->value_size() );
}


CString CXMLNode::GetAttributeAsString(TCHAR * szAttribute)
{
  return GetAttribute( szAttribute );
}

bool CXMLNode::HasAttribute(TCHAR * szAttribute)
{
  if ( !pNode )
    return false;

  auto attr = pNode->first_attribute( szAttribute );
  return attr != nullptr;
}

int32_t CXMLNode::IsValid()
{
	return pNode != NULL;
}

void CXMLNode::GetAttributeAsInteger(TCHAR * szAttribute, int32_t * pnValue)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetAttribute(szAttribute, s, 20);
	_stscanf_s(s, _T("%d"), pnValue);
}

void CXMLNode::GetAttributeAsFloat(TCHAR * szAttribute, float * pfValue)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetAttribute(szAttribute, s, 20);
	_stscanf_s(s, _T("%g"), pfValue);
}

CXMLNode& CXMLNode::AddChild(TCHAR * szNodeName)
{
  TCHAR *tc = szNodeName;
  if ( szNodeName )
  {
    while ( *tc )
    {
      if ( *tc == ' ' )
      {
        *tc = '_';
      }
      tc++;
    };
  }

  if ( !pNode || !pDoc )
  {
    children.emplace_back( std::make_unique<CXMLNode>() );
    return *children.back();
  }

  auto node = pNode->document()->allocate_node( node_type::node_element, szNodeName );

  pNode->append_node( node );

  children.emplace_back( std::make_unique<CXMLNode>( node, pDoc, nLevel + 1 ) );
  return *children.back();
}

void CXMLNode::SetAttribute(const std::string_view& szAttributeName,
                            const std::string_view& szValue) {
  if ( !pNode || !pDoc )
    return;

  stringStore.emplace_back(std::make_unique<std::string>(szValue));
  std::string* strVal = stringStore.back().get();

  stringStore.emplace_back(std::make_unique<std::string>(szAttributeName));
  std::string* strNam = stringStore.back().get();

  auto attr = pNode->first_attribute( strNam->c_str() );

  if ( !attr )
  {
    attr = pNode->document()->allocate_attribute( strNam->c_str(), strVal->c_str() );
    pNode->append_attribute( attr );
    return;
  }

  attr->value( strVal->c_str() );
}

void CXMLNode::SetAttributeFromInteger(const std::string_view& szAttributeName,
                                       int32_t nValue) {
	TCHAR s[64];
  memset( s, 0, sizeof( TCHAR ) * 64 );
	_sntprintf_s(s, 64, _T("%d"), nValue);
	SetAttribute(szAttributeName, s);
}

void CXMLNode::SetAttributeFromFloat(const std::string_view& szAttributeName,
                                     float fValue) {
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%g"), fValue);
	SetAttribute(szAttributeName, s);
}

void CXMLNode::SetText(const TCHAR * sz)
{
  if ( !pNode )
    return;

  value = std::string( sz );

  pNode->value( value.c_str() );
}

void CXMLNode::SetText(CString &s)
{
	SetText(s.GetPointer());
}

void CXMLNode::SetInt(int32_t Int)
{
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%d"), Int);
	SetText(s);
}

void CXMLNode::SetFloat(float Float)
{
	TCHAR s[64];
	_sntprintf_s(s, 64, _T("%g"), Float);
	SetText(s);
}

bool CXMLNode::GetValue(int32_t &Int)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	return _stscanf_s(s, _T("%d"), &Int) == 1;
}

bool CXMLNode::GetValue(TBOOL &Int)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	int32_t x = 0;
	int32_t r = _stscanf_s(s, _T("%d"), &x);
	if (r == 1)
		Int = x != 0;
	return r == 1;
}

bool CXMLNode::GetValue(float &Float)
{
	TCHAR s[20];
	ZeroMemory(s, 20);
	GetText(s, 20);
	return _stscanf_s(s, _T("%g"), &Float) == 1;
}

