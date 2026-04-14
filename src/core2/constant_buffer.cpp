#include "src/core2/constant_buffer.h"

#include <cstring>
#include <memory>

namespace renderer {

ConstantBuffer::ConstantBuffer(Device* device) : Resource(device) {}

ConstantBuffer::~ConstantBuffer() = default;

void ConstantBuffer::Reset() { data_length_ = 0; }

void ConstantBuffer::AddData(const void* data_in, int32_t length) {
  if (data_length_ + length > buffer_length_) {
    std::unique_ptr<uint8_t[]> old_data;
    old_data.swap(data_);
    data_ = std::make_unique<uint8_t[]>(data_length_ + length);

    if (old_data) {
      std::memcpy(data_.get(), old_data.get(), data_length_);
    }

    buffer_length_ = data_length_ + length;
  }

  std::memcpy(data_.get() + data_length_, data_in, length);
  data_length_ += length;
}

void ConstantBuffer::Upload() {}

}  // namespace renderer
