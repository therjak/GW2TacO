#pragma once
#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "../BaseLib/Types.h"
#include "../UtilLib/RapidXML/rapidxml.hpp"

class CXMLDocument;

class CXMLNode {
 public:
  CXMLNode();
  CXMLNode(const CXMLNode& Original);
  CXMLNode(rapidxml::xml_node<char>*, CXMLDocument*, int32_t);
  virtual ~CXMLNode();

  CXMLNode operator=(const CXMLNode Original);

  int32_t GetChildCount();
  int32_t GetChildCount(char*);
  CXMLNode GetChild(int32_t);
  CXMLNode GetChild(char*);
  CXMLNode GetChild(char*, int32_t);

  bool Next(CXMLNode& out, char*);

  int32_t IsValid();

  std::string GetNodeName();

  void GetText(char*, int32_t);
  std::string GetText();
  bool GetValue(int32_t& Int);
  bool GetValue(uint8_t& Int);
  bool GetValue(bool& Int);
  bool GetValue(float& Float);

  bool GetAttribute(char* szAttribute, char* szBuffer, int32_t nBufferSize);
  std::string GetAttribute(std::string_view szAttribute);
  void GetAttributeAsInteger(char* szAttribute, int32_t* nValue);
  void GetAttributeAsFloat(char* szAttribute, float* fValue);
  std::string GetAttributeAsString(std::string_view szAttribute);
  bool HasAttribute(std::string_view szAttribute);

  CXMLNode& AddChild(std::string_view);
  void SetText(std::string_view s);
  void SetInt(int32_t Int);
  void SetFloat(float Float);
  void SetAttribute(std::string_view szAttributeName, std::string_view szValue);
  void SetAttributeFromInteger(std::string_view szAttributeName,
                               int32_t nValue);
  void SetAttributeFromFloat(std::string_view szAttributeName, float nValue);

 private:
  int32_t childCount = -1;
  std::unordered_map<int32_t, int32_t> childCounts;
  std::vector<std::unique_ptr<std::string>> stringStore;
  std::vector<std::unique_ptr<CXMLNode>> children;

  std::string value;

  int32_t nLevel;
  rapidxml::xml_node<char>* pNode;
  CXMLDocument* pDoc;
};
