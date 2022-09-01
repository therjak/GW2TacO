#pragma once

#include <cstdint>
#include <memory>

#include "src/base/assert.h"

template <typename ItemType>
class CRingBuffer {
 public:
  explicit CRingBuffer(const uint32_t Size = 500) : Capacity(Size) {
    Array = std::make_unique<ItemType[]>(Size);
  }

  void Add(const ItemType& Item) {
    Array[Count % Capacity] = Item;
    Count++;
  }

  CRingBuffer<ItemType>& operator+=(const ItemType& i) {
    Add(i);
    return *this;
  }

  ItemType const operator[](const int32_t idx) const {
    int32_t start = Count - Capacity;
    if (start < 0) start = 0;
    int32_t realindex = start + idx;
    BASEASSERT(realindex < Count);
    return (const ItemType)Array[realindex % Capacity];
  }

  ItemType& operator[](const int32_t idx) {
    int32_t start = Count - Capacity;
    if (start < 0) start = 0;
    const int32_t realindex = start + idx;
    BASEASSERT(realindex < Count);
    return Array[realindex % Capacity];
  }

  int32_t NumItems() {
    if (Capacity < Count) return Capacity;
    return Count;
  }

  void Flush() {
    Array = std::make_unique<ItemType[]>(Capacity);
    Count = 0;
  }

 private:
  std::unique_ptr<ItemType[]> Array;
  int32_t Capacity;
  int32_t Count = 0;
};
