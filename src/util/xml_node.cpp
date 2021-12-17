#include "src/util/xml_node.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "src/util//RapidXML/rapidxml.hpp"
#include "src/util/xml_document.h"

using rapidxml::node_type;
using rapidxml::xml_node;

int32_t GetStringHash(const char* string) {
  if (!string) return 0;

  int32_t c;
  const char* str = string;

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

CXMLNode& CXMLNode::operator=(const CXMLNode Original) {
  nLevel = Original.nLevel;
  pNode = Original.pNode;
  pDoc = Original.pDoc;
  return *this;
}

CXMLNode::~CXMLNode() = default;

int32_t CXMLNode::GetChildCount() const {
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

int32_t CXMLNode::GetChildCount(const char* szNodeName) const {
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

std::string CXMLNode::GetNodeName() const {
  if (!pNode) return {};
  return (pNode->name());
}

CXMLNode CXMLNode::GetChild(int32_t n) const {
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

CXMLNode CXMLNode::GetChild(const char* szNodeName) const {
  if (!pNode) return CXMLNode();

  auto node = pNode->first_node(szNodeName);
  if (!node) return CXMLNode();

  return CXMLNode(node, pDoc, nLevel + 1);
}

CXMLNode CXMLNode::GetChild(const char* szNodeName, int32_t n) const {
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

bool CXMLNode::Next(CXMLNode& out, const char* szNodeName) {
  if (!pNode) return false;

  auto node = pNode->next_sibling(szNodeName);
  if (!node) return false;

  out = CXMLNode(node, pDoc, nLevel);
  return true;
}

void CXMLNode::GetText(char* szBuffer, int32_t nBufferSize) { int x = 0; }

std::string CXMLNode::GetText() { return {}; }

bool CXMLNode::GetAttribute(std::string_view szAttribute, char* szBuffer,
                            int32_t nBufferSize) const {
  if (!pNode) return false;

  auto attr = pNode->first_attribute(szAttribute.data());
  if (!attr) return false;

  strncpy_s(szBuffer, nBufferSize, attr->value(), _TRUNCATE);
  return true;
}

std::string CXMLNode::GetAttribute(std::string_view szAttribute) const {
  if (!pNode) return {};

  auto attr = pNode->first_attribute(szAttribute.data(), szAttribute.size());
  if (!attr) return {};

  return std::string(attr->value(), attr->value_size());
}

std::string CXMLNode::GetAttributeAsString(std::string_view szAttribute) const {
  return GetAttribute(szAttribute);
}

bool CXMLNode::HasAttribute(std::string_view szAttribute) const {
  if (!pNode) return false;

  auto attr = pNode->first_attribute(szAttribute.data(), szAttribute.size());
  return attr != nullptr;
}

int32_t CXMLNode::IsValid() const { return pNode != nullptr; }

void CXMLNode::GetAttributeAsInteger(std::string_view szAttribute,
                                     int32_t* pnValue) const {
  char s[20] = {0};
  GetAttribute(szAttribute, s, 20);
  sscanf_s(s, "%d", pnValue);
}

void CXMLNode::GetAttributeAsFloat(std::string_view szAttribute,
                                   float* pfValue) const {
  char s[20] = {0};
  GetAttribute(szAttribute, s, 20);
  sscanf_s(s, "%g", pfValue);
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
  _snprintf_s(s, 64, "%d", nValue);
  SetAttribute(szAttributeName, s);
}

void CXMLNode::SetAttributeFromFloat(std::string_view szAttributeName,
                                     float fValue) {
  char s[64];
  _snprintf_s(s, 64, "%g", fValue);
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
  _snprintf_s(s, 64, "%d", Int);
  SetText(s);
}

void CXMLNode::SetFloat(float Float) {
  char s[64];
  _snprintf_s(s, 64, "%g", Float);
  SetText(s);
}

bool CXMLNode::GetValue(int32_t& Int) {
  char s[20] = {0};
  GetText(s, 20);
  return sscanf_s(s, "%d", &Int) == 1;
}

bool CXMLNode::GetValue(uint8_t& Int) {
  char s[20] = {0};
  GetText(s, 20);
  int32_t x = 0;
  int32_t r = sscanf_s(s, "%d", &x);
  if (r == 1) Int = x;
  return r == 1;
}

bool CXMLNode::GetValue(bool& Int) {
  char s[20] = {0};
  GetText(s, 20);
  int32_t x = 0;
  int32_t r = sscanf_s(s, "%d", &x);
  if (r == 1) Int = x != 0;
  return r == 1;
}

bool CXMLNode::GetValue(float& Float) {
  char s[20] = {0};
  GetText(s, 20);
  return sscanf_s(s, "%g", &Float) == 1;
}
