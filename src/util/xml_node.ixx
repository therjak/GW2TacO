module;

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/util/RapidXML/rapidxml.hpp"

export module xml:node;

export class XMLDocument;

export class XMLNode {
 public:
  XMLNode();
  XMLNode(const XMLNode& Original);
  XMLNode(rapidxml::xml_node<char>*, const XMLDocument*, int32_t);
  virtual ~XMLNode();

  XMLNode& operator=(const XMLNode& Original);

  int32_t GetChildCount() const;
  int32_t GetChildCount(const char*) const;
  XMLNode GetChild(int32_t) const;
  XMLNode GetChild(const char*) const;
  XMLNode GetChild(const char*, int32_t) const;

  bool Next(XMLNode* out, const char*);

  int32_t IsValid() const;

  std::string GetNodeName() const;

  void GetText(char*, int32_t) const;
  std::string GetText() const;
  bool GetValue(int32_t* value) const;
  bool GetValue(uint8_t* value) const;
  bool GetValue(bool* value) const;
  bool GetValue(float* value) const;

  bool GetAttribute(std::string_view szAttribute, char* szBuffer,
                    int32_t nBufferSize) const;
  std::string GetAttribute(std::string_view szAttribute) const;
  void GetAttributeAsInteger(std::string_view szAttribute,
                             int32_t* nValue) const;
  void GetAttributeAsFloat(std::string_view szAttribute, float* fValue) const;
  std::string GetAttributeAsString(std::string_view szAttribute) const;
  bool HasAttribute(std::string_view szAttribute) const;

  XMLNode& AddChild(std::string_view);
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
  std::vector<std::unique_ptr<XMLNode>> children;

  std::string value;

  int32_t nLevel;
  rapidxml::xml_node<char>* pNode;
  const XMLDocument* pDoc;
};
