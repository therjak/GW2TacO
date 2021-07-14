#include "ConstantBuffer.h"

#include <cstring>
#include <memory>

CCoreConstantBuffer::CCoreConstantBuffer(CCoreDevice *Device)
    : CCoreResource(Device) {}

CCoreConstantBuffer::~CCoreConstantBuffer() {}

void CCoreConstantBuffer::Reset() { DataLength = 0; }

void CCoreConstantBuffer::AddData(void *DataIn, int32_t Length) {
  if (DataLength + Length > BufferLength) {
    std::unique_ptr<uint8_t[]> OldData;
    OldData.swap(Data);
    Data = std::make_unique<uint8_t[]>(DataLength + Length);

    if (OldData) {
      memcpy(Data.get(), OldData.get(), DataLength);
    }

    BufferLength = DataLength + Length;
  }

  memcpy(Data.get() + DataLength, DataIn, Length);
  DataLength += Length;
}

void CCoreConstantBuffer::Upload() {}
