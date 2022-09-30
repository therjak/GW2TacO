#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "src/base/assert.h"

template <typename ItemType, int32_t SIZE>
class CRingBuffer {
 public:
  explicit CRingBuffer() = default;

  void Add(const ItemType& Item) {
    Array[Count % SIZE] = Item;
    Count++;
  }

  CRingBuffer<ItemType, SIZE>& operator+=(const ItemType& i) {
    Add(i);
    return *this;
  }

  const ItemType& operator[](const int32_t idx) const {
    int32_t start = Count - SIZE;
    if (start < 0) start = 0;
    int32_t realindex = start + idx;
    BASEASSERT(realindex < Count);
    return Array[realindex % SIZE];
  }

  ItemType& operator[](const int32_t idx) {
    int32_t start = Count - SIZE;
    if (start < 0) start = 0;
    const int32_t realindex = start + idx;
    BASEASSERT(realindex < Count);
    return Array[realindex % SIZE];
  }

  int32_t NumItems() const {
    if (SIZE < Count) return SIZE;
    return Count;
  }

  void Flush() {
    Array.fill({});
    Count = 0;
  }

 private:
  std::array<ItemType, SIZE> Array{};
  int32_t Count = 0;
};
