#pragma once
#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>

#include "../UtilLib/RapidXML/rapidxml.hpp"

class CXMLDocument;

class CXMLNode {
 public:
  CXMLNode();
  CXMLNode(const CXMLNode& Original);
  CXMLNode(rapidxml::xml_node<char>*, CXMLDocument*, int32_t);
  virtual ~CXMLNode();

  CXMLNode& operator=(const CXMLNode Original);

  int32_t GetChildCount() const;
  int32_t GetChildCount(const char*) const;
  CXMLNode GetChild(int32_t) const;
  CXMLNode GetChild(const char*) const;
  CXMLNode GetChild(const char*, int32_t) const;

  bool Next(CXMLNode& out, const char*);

  int32_t IsValid() const;

  std::string GetNodeName() const;

  void GetText(char*, int32_t);
  std::string GetText();
  bool GetValue(int32_t& Int);
  bool GetValue(uint8_t& Int);
  bool GetValue(bool& Int);
  bool GetValue(float& Float);

  bool GetAttribute(std::string_view szAttribute, char* szBuffer,
                    int32_t nBufferSize) const;
  std::string GetAttribute(std::string_view szAttribute) const;
  void GetAttributeAsInteger(std::string_view szAttribute,
                             int32_t* nValue) const;
  void GetAttributeAsFloat(std::string_view szAttribute, float* fValue) const;
  std::string GetAttributeAsString(std::string_view szAttribute) const;
  bool HasAttribute(std::string_view szAttribute) const;

  CXMLNode& AddChild(std::string_view);
  void SetText(std::string_view s);
  void SetInt(int32_t Int);
  void SetFloat(float Float);
  void SetAttribute(std::string_view szAttributeName, std::string_view szValue);
  void SetAttributeFromInteger(std::string_view szAttributeName,
                               int32_t nValue);
  void SetAttributeFromFloat(std::string_view szAttributeName, float nValue);

 private:
  mutable int32_t childCount = -1;
  mutable std::unordered_map<int32_t, int32_t> childCounts;
  std::vector<std::unique_ptr<std::string>> stringStore;
  std::vector<std::unique_ptr<CXMLNode>> children;

  std::string value;

  int32_t nLevel;
  rapidxml::xml_node<char>* pNode;
  CXMLDocument* pDoc;
};
