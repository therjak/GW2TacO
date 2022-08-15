#pragma once
#include <cstdint>
#include <memory>

#include "src/core2/resource.h"

class CCoreConstantBuffer : public CCoreResource {
 protected:
  std::unique_ptr<uint8_t[]> Data;
  int32_t BufferLength = 0;
  int32_t DataLength = 0;

 public:
  explicit CCoreConstantBuffer(CCoreDevice* Device);
  ~CCoreConstantBuffer() override;

  void Reset();
  void AddData(const void* Data, int32_t Length);
  virtual void Upload();
  [[nodiscard]] virtual void* GetBufferPointer() const = 0;
};
