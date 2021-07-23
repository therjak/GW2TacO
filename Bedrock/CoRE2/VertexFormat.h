#pragma once

#include <vector>

#include "Enums.h"
#include "Resource.h"

class CCoreVertexShader;

class CCoreVertexFormat : public CCoreResource {
  friend class CCoreDevice;
  virtual bool Apply() = 0;

public:
  CCoreVertexFormat(CCoreDevice* Device) : CCoreResource(Device) {}

  virtual bool Create(const std::vector<COREVERTEXATTRIBUTE>& Attributes,
                      CCoreVertexShader* VxShader = nullptr) = 0;
  virtual int32_t GetSize() = 0;
};
