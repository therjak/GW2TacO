#pragma once

template <typename ItemType> class CRingBuffer
{
  ItemType *Array;
  int32_t Capacity;
  int32_t Count;

public:

	CRingBuffer(const uint32_t Size = 500)
  {
    Capacity = Size;
    Array = new ItemType[ Size ];
    Count = 0;
  }

  virtual ~CRingBuffer()
  {
    SAFEDELETEA( Array );
  }

  virtual void Add( const ItemType &Item )
  {
    Array[ Count%Capacity ] = Item;
    Count++;
  }

  virtual CRingBuffer<ItemType> &operator+= ( const ItemType &i )
  {
    Add( i );
    return *this;
  }

  virtual ItemType const operator[]( const int32_t idx ) const
  {
    int32_t start = Count - Capacity;
    if ( start < 0 ) start = 0;
    int32_t realindex = start + idx;
    BASEASSERT( realindex < Count );
    return (const ItemType)Array[ realindex%Capacity ];
  }

  virtual ItemType &operator[]( const int32_t idx )
  {
    int32_t start = Count - Capacity;
    if ( start < 0 ) start = 0;
    int32_t realindex = start + idx;
    BASEASSERT( realindex < Count );
    return Array[ realindex%Capacity ];
  }

  int32_t NumItems()
  {
    if ( Capacity < Count ) return Capacity;
    return Count;
  }

  void Flush()
  {
    SAFEDELETEA( Array );
    Array = new ItemType[ Capacity ];
    Count = 0;
  }

};
