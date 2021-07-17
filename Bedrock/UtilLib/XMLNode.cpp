#include "XMLNode.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <algorithm>

#include "../UtilLib/RapidXML/rapidxml.hpp"
#include "XMLDocument.h"
#include <tchar.h>

using rapidxml::node_type;
using rapidxml::xml_node;

int32_t GetStringHash(char* string) {
  if (!string) return 0;

  int32_t c;
  char* str = string;

  int32_t Hash = 5381;
  while ((c = *str++)) Hash = ((Hash << 5) + Hash) + c;  // hash * 33 + c

  return Hash;
}

CXMLNode::CXMLNode() {
  pNode = nullptr;
  pDoc = nullptr;
  nLevel = 0;
}

CXMLNode::CXMLNode(const CXMLNode& Original) {
  nLevel = Original.nLevel;
  pNode = Original.pNode;
  pDoc = Original.pDoc;
}

CXMLNode::CXMLNode(xml_node<char>* p, CXMLDocument* d, int32_t l) {
  pNode = p;
  pDoc = d;
  nLevel = l;
}

CXMLNode CXMLNode::operator=(const CXMLNode Original) {
  // if (pNode) pNode->Release();

  nLevel = Original.nLevel;
  pNode = Original.pNode;
  pDoc = Original.pDoc;
  return *this;
}

CXMLNode::~CXMLNode() = default;

int32_t CXMLNode::GetChildCount() {
  if (!pNode) return 0;

  if (childCount != -1) return childCount;

  auto node = pNode->first_node();
  if (!node) {
    childCount = 0;
    return 0;
  }

  int32_t count = 1;

  while ((node = node->next_sibling())) count++;

  childCount = count;

  return count;
}

int32_t CXMLNode::GetChildCount(char* szNodeName) {
  int32_t hash = GetStringHash(szNodeName);

  if (childCounts.find(hash) != childCounts.end()) return childCounts[hash];

  auto node = pNode->first_node(szNodeName);
  if (!node) {
    childCounts[hash] = 0;
    return 0;
  }

  int32_t count = 1;

  while ((node = node->next_sibling(szNodeName))) count++;

  childCounts[hash] = count;

  return count;
}

std::string CXMLNode::GetNodeName() {
  if (!pNode) return {};
  return (pNode->name());
}

CXMLNode CXMLNode::GetChild(int32_t n) {
  if (!pNode) return CXMLNode();

  auto node = pNode->first_node();
  if (!node) return CXMLNode();

  if (n == 0) return CXMLNode(node, pDoc, nLevel + 1);

  int32_t count = 1;

  while ((node = node->next_sibling())) {
    if (n == count) return CXMLNode(node, pDoc, nLevel + 1);
    count++;
  }

  return CXMLNode();
}

CXMLNode CXMLNode::GetChild(char* szNodeName) {
  if (!pNode) return CXMLNode();

  auto node = pNode->first_node(szNodeName);
  if (!node) return CXMLNode();

  return CXMLNode(node, pDoc, nLevel + 1);
}

CXMLNode CXMLNode::GetChild(char* szNodeName, int32_t n) {
  if (!pNode) return CXMLNode();

  auto node = pNode->first_node(szNodeName);
  if (!node) return CXMLNode();

  if (n == 0) return CXMLNode(node, pDoc, nLevel + 1);

  int32_t count = 1;

  while ((node = node->next_sibling(szNodeName))) {
    if (n == count) return CXMLNode(node, pDoc, nLevel + 1);
    count++;
  }

  return CXMLNode();
}

bool CXMLNode::Next(CXMLNode& out, char* szNodeName) {
  if (!pNode) return false;

  auto node = pNode->next_sibling(szNodeName);
  if (!node) return false;

  out = CXMLNode(node, pDoc, nLevel);
  return true;
}

void CXMLNode::GetText(char* szBuffer, int32_t nBufferSize) { int x = 0; }

std::string CXMLNode::GetText() { return {}; }

bool CXMLNode::GetAttribute(char* szAttribute, char* szBuffer,
                            int32_t nBufferSize) {
  if (!pNode) return false;

  auto attr = pNode->first_attribute(szAttribute);
  if (!attr) return false;

  _tcsncpy_s(szBuffer, nBufferSize, attr->value(), _TRUNCATE);
  return true;
}

std::string CXMLNode::GetAttribute(std::string_view szAttribute) {
  if (!pNode) return {};

  auto attr = pNode->first_attribute(szAttribute.data(), szAttribute.size());
  if (!attr) return {};

  return std::string(attr->value(), attr->value_size());
}

std::string CXMLNode::GetAttributeAsString(std::string_view szAttribute) {
  return GetAttribute(szAttribute);
}

bool CXMLNode::HasAttribute(std::string_view szAttribute) {
  if (!pNode) return false;

  auto attr = pNode->first_attribute(szAttribute.data(), szAttribute.size());
  return attr != nullptr;
}

int32_t CXMLNode::IsValid() { return pNode != nullptr; }

void CXMLNode::GetAttributeAsInteger(char* szAttribute, int32_t* pnValue) {
  char s[20] = {0};
  GetAttribute(szAttribute, s, 20);
  _stscanf_s(s, _T("%d"), pnValue);
}

void CXMLNode::GetAttributeAsFloat(char* szAttribute, float* pfValue) {
  char s[20] = {0};
  GetAttribute(szAttribute, s, 20);
  _stscanf_s(s, _T("%g"), pfValue);
}

CXMLNode& CXMLNode::AddChild(std::string_view szNodeName) {
  std::string tc(szNodeName);
  std::replace(tc.begin(), tc.end(), ' ', '_');

  if (!pNode || !pDoc) {
    children.emplace_back(std::make_unique<CXMLNode>());
    return *children.back();
  }
  stringStore.emplace_back(std::make_unique<std::string>(tc));
  auto node = pNode->document()->allocate_node(node_type::node_element,
                                               stringStore.back()->c_str());

  pNode->append_node(node);

  children.emplace_back(std::make_unique<CXMLNode>(node, pDoc, nLevel + 1));
  return *children.back();
}

void CXMLNode::SetAttribute(std::string_view szAttributeName,
                            std::string_view szValue) {
  if (!pNode || !pDoc) return;

  stringStore.emplace_back(std::make_unique<std::string>(szValue));
  std::string* strVal = stringStore.back().get();

  stringStore.emplace_back(std::make_unique<std::string>(szAttributeName));
  std::string* strNam = stringStore.back().get();

  auto attr = pNode->first_attribute(strNam->c_str());

  if (!attr) {
    attr =
        pNode->document()->allocate_attribute(strNam->c_str(), strVal->c_str());
    pNode->append_attribute(attr);
    return;
  }

  attr->value(strVal->c_str());
}

void CXMLNode::SetAttributeFromInteger(std::string_view szAttributeName,
                                       int32_t nValue) {
  char s[64];
  memset(s, 0, sizeof(char) * 64);
  _sntprintf_s(s, 64, _T("%d"), nValue);
  SetAttribute(szAttributeName, s);
}

void CXMLNode::SetAttributeFromFloat(std::string_view szAttributeName,
                                     float fValue) {
  char s[64];
  _sntprintf_s(s, 64, _T("%g"), fValue);
  SetAttribute(szAttributeName, s);
}

void CXMLNode::SetText(std::string_view s) {
  if (!pNode) return;

  stringStore.emplace_back(std::make_unique<std::string>(s));
  std::string* str = stringStore.back().get();

  pNode->value(str->c_str());
}

void CXMLNode::SetInt(int32_t Int) {
  char s[64];
  _sntprintf_s(s, 64, _T("%d"), Int);
  SetText(s);
}

void CXMLNode::SetFloat(float Float) {
  char s[64];
  _sntprintf_s(s, 64, _T("%g"), Float);
  SetText(s);
}

bool CXMLNode::GetValue(int32_t& Int) {
  char s[20] = {0};
  GetText(s, 20);
  return _stscanf_s(s, _T("%d"), &Int) == 1;
}

bool CXMLNode::GetValue(TBOOL& Int) {
  char s[20] = {0};
  GetText(s, 20);
  int32_t x = 0;
  int32_t r = _stscanf_s(s, _T("%d"), &x);
  if (r == 1) Int = x != 0;
  return r == 1;
}

bool CXMLNode::GetValue(float& Float) {
  char s[20] = {0};
  GetText(s, 20);
  return _stscanf_s(s, _T("%g"), &Float) == 1;
}
