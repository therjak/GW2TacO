#pragma once

#include <string>
#include <string_view>
#include <vector>

class IWBCSS {
  std::vector<std::string> aClasses;
  std::string sID;

 public:
  IWBCSS();
  virtual ~IWBCSS();

  void SetID(std::string_view s);
  std::string& GetID();
  void AddClass(std::string_view s);
  void RemoveClass(std::string_view s);
  void ToggleClass(std::string_view s);
  bool HasClass(std::string_view s);
  bool IsFitForSelector(std::string_view selector);

  [[nodiscard]] virtual const std::string& GetType() const = 0;
  static const std::string& GetClassName() {
    static const std::string type = "IWBCSS";
    return type;
  }
  [[nodiscard]] virtual bool InstanceOf(std::string_view name) const = 0;

  virtual bool ApplyStyle(std::string_view prop, std::string_view value,
                          const std::vector<std::string>& Pseudo);
  std::string GetClassString();
};
