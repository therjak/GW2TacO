#pragma once
#include <cstdint>

#include "src/core2/resource.h"

namespace renderer {
class CCoreIndexBuffer : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreIndexBuffer(CCoreDevice* device) : CCoreResource(device) {}

  virtual bool Create(const uint32_t index_count,
                      const uint32_t index_size = 2) = 0;
  virtual bool Lock(void** result, const uint32_t index_offset,
                    const int32_t index_count) = 0;
  virtual bool Lock(void** result) = 0;
  virtual bool UnLock() = 0;
  virtual void* GetHandle() = 0;

 private:
  virtual bool Apply() = 0;
};
}  // namespace renderer
