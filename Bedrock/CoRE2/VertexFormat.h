#pragma once
#include "../BaseLib/Array.h"
#include "Enums.h"
#include "Resource.h"

class CCoreVertexShader;

class CCoreVertexFormat : public CCoreResource {
  friend class CCoreDevice;
  virtual TBOOL Apply() = 0;

 public:
  CCoreVertexFormat(CCoreDevice *Device) : CCoreResource(Device) {}

  virtual TBOOL Create(const CArray<COREVERTEXATTRIBUTE> &Attributes,
                       CCoreVertexShader *VxShader = nullptr) = 0;
  virtual int32_t GetSize() = 0;
};
