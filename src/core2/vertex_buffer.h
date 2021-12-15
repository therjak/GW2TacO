#pragma once
#include "src/core2/resource.h"

class CCoreVertexBuffer : public CCoreResource {
  friend class CCoreDevice;
  virtual bool Apply(const uint32_t Offset) = 0;

 public:
  INLINE explicit CCoreVertexBuffer(CCoreDevice* Device)
      : CCoreResource(Device) {}

  virtual bool Create(const uint8_t* Data, const uint32_t Size) = 0;
  virtual bool CreateDynamic(const uint32_t Size) = 0;
  virtual bool Update(const int32_t Offset, const uint8_t* Data,
                      const uint32_t Size) = 0;
  virtual bool Lock(void** Result, const uint32_t Offset, const int32_t size,
                    const int32_t Flags) = 0;
  virtual bool Lock(void** Result) = 0;
  virtual bool UnLock() = 0;
  virtual void* GetHandle() = 0;
};
