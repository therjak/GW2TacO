#pragma once
#include <cstdint>

#include "src/core2/resource.h"

class CCoreIndexBuffer : public CCoreResource {
  friend class CCoreDevice;
  virtual bool Apply() = 0;

 public:
  explicit CCoreIndexBuffer(CCoreDevice* Device) : CCoreResource(Device) {}

  virtual bool Create(const uint32_t IndexCount,
                      const uint32_t IndexSize = 2) = 0;
  virtual bool Lock(void** Result, const uint32_t IndexOffset,
                    const int32_t IndexCount) = 0;
  virtual bool Lock(void** Result) = 0;
  virtual bool UnLock() = 0;
  virtual void* GetHandle() = 0;
};
