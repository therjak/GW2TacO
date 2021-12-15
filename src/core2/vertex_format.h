#pragma once

#include <vector>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

class CCoreVertexShader;

class CCoreVertexFormat : public CCoreResource {
  friend class CCoreDevice;
  virtual bool Apply() = 0;

 public:
  explicit CCoreVertexFormat(CCoreDevice* Device) : CCoreResource(Device) {}

  virtual bool Create(const std::vector<COREVERTEXATTRIBUTE>& Attributes,
                      CCoreVertexShader* VxShader = nullptr) = 0;
  virtual int32_t GetSize() = 0;
};
