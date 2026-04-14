#pragma once
#include <cstdint>
#include <memory>

#include "src/core2/resource.h"

namespace renderer {

class ConstantBuffer : public Resource {
 public:
  explicit ConstantBuffer(Device* device);
  ~ConstantBuffer() override;

  void Reset();
  void AddData(const void* data, int32_t length);
  virtual void Upload();
  [[nodiscard]] virtual void* GetBufferPointer() const = 0;

 protected:
  std::unique_ptr<uint8_t[]> data_;
  int32_t buffer_length_ = 0;
  int32_t data_length_ = 0;
};

}  // namespace renderer
