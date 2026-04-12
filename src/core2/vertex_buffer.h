#pragma once
#include <cstdint>

#include "src/core2/resource.h"

namespace renderer {

class CCoreVertexBuffer : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreVertexBuffer(CCoreDevice* device) : CCoreResource(device) {}

  virtual bool Create(const uint8_t* data, const uint32_t size) = 0;
  virtual bool CreateDynamic(const uint32_t size) = 0;
  virtual bool Update(const int32_t offset, const uint8_t* data,
                      const uint32_t size) = 0;
  virtual bool Lock(void** result, const uint32_t offset, const int32_t size,
                    const int32_t flags) = 0;
  virtual bool Lock(void** result) = 0;
  virtual bool UnLock() = 0;
  virtual void* GetHandle() = 0;

 private:
  virtual bool Apply(const uint32_t offset) = 0;
};

}  // namespace renderer
