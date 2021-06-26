#pragma once
#include "CriticalSection.h"
#include "Assert.h"

//dynamic array class

#define EXPANSIONRATIO 0.2f

template <typename ItemType> class CArray
{
protected:

  int32_t Capacity;
  int32_t ItemCount;
  ItemType *Array;

  int32_t GetCapacity() const
  {
    return Capacity;
  }

public:
  typedef int32_t( __cdecl *ARRAYSORTCALLBACK )( ItemType *a, ItemType *b );

  CArray()
  {
    Capacity = 0;
    ItemCount = 0;
    Array = 0;
  }

  CArray( int32_t Size )
  {
    Array = new ItemType[ Size ];
    Capacity = Size;
    ItemCount = 0;
  }

  CArray( const CArray<ItemType> &original )
  {
    ItemCount = Capacity = original.NumItems();
    Array = new ItemType[ ItemCount ];
    for ( int32_t x = 0; x < ItemCount; x++ )
      Array[ x ] = original[ x ];
  }

  ~CArray()
  {
    if ( Array ) delete[] Array;
    Capacity = ItemCount = 0;
    Array = 0;
  }

  int32_t NumItems() const
  {
    return ItemCount;
  }

  void Flush()
  {
    if ( Array ) delete[] Array;
    Array = new ItemType[ Capacity ];
    ItemCount = 0;
  }

  void FlushFast()
  {
    ItemCount = 0;
  }

  void Add( const ItemType &Item )
  {
    if ( ItemCount == Capacity || !Array ) Expand( (int32_t)( Capacity*EXPANSIONRATIO + 1 ) );
    Array[ ItemCount ] = Item;
    ItemCount++;
  }

  void InsertFirst( const ItemType &Item )
  {
    Add( Item );
    for ( int x = ItemCount - 2; x >= 0; x-- )
      Array[ x + 1 ] = Array[ x ];
    Array[ 0 ] = Item;
  }

  void AllocateNewUninitialized( const int32_t Count )
  {
    if ( ItemCount + Count > Capacity || !Array ) Expand( (int32_t)( Capacity*EXPANSIONRATIO + Count ) );
    ItemCount += Count;
  }

  CArray<ItemType> &operator= ( const CArray<ItemType> &a )
  {
    if ( &a == this ) return *this;
    Flush();
    for ( int i = 0; i < a.NumItems(); i++ )
      Add( a[ i ] );
    return *this;
  }

  CArray<ItemType> &operator+= ( const ItemType &i )
  {
    Add( i );
    return *this;
  }

  CArray<ItemType> &operator+= ( const CArray<ItemType> &i )
  {
    for ( int32_t x = 0; x < i.NumItems(); x++ )
      Add( i[ x ] );
    return *this;
  }

  CArray<ItemType> &operator-= ( const ItemType &i )
  {
    Delete( i );
    return *this;
  }

  int32_t AddUnique( const ItemType &Item )
  {
    auto idx = Find( Item );
    if ( idx != -1 ) return idx;
    Add( Item );
    return ItemCount - 1;
  }

  ItemType const operator[]( const int32_t idx ) const
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    return (const ItemType)Array[ idx ];
  }

  ItemType &operator[]( const int32_t idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );
    return Array[ idx ];
  }

  ItemType &Last()
  {
    BASEASSERT( ItemCount > 0 );
    return Array[ ItemCount - 1 ];
  }

  int32_t const Find( const ItemType &i ) const
  {
    for ( int32_t x = 0; x < ItemCount; x++ )
      if ( Array[ x ] == i ) return x;
    return -1;
  }

  void DeleteByIndex( const int32_t idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    ItemCount--;
    for ( int32_t x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Delete( const ItemType &i )
  {
    DeleteByIndex( Find( i ) );
  }

  void FreeByIndex( const int32_t idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete Array[ idx ];
    ItemCount--;
    for ( int32_t x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void Free( const ItemType &i )
  {
    FreeByIndex( Find( i ) );
  }

  void FreeAByIndex( const int32_t idx )
  {
    if ( idx < 0 || idx >= ItemCount ) return;
    delete[] Array[ idx ];
    ItemCount--;
    for ( int32_t x = idx; x < ItemCount; x++ )
      Array[ x ] = Array[ x + 1 ];
  }

  void FreeA( const ItemType &i )
  {
    FreeAByIndex( Find( i ) );
  }

  void Swap( const int32_t a, const int32_t b )
  {
    ItemType temp = Array[ a ];
    Array[ a ] = Array[ b ];
    Array[ b ] = temp;
  }

  void Sort( ARRAYSORTCALLBACK SortFunct )
  {
    qsort( Array, ItemCount, sizeof( ItemType ), ( int32_t( _cdecl* )( const void*, const void* ) )SortFunct );
  }

  ItemType *GetPointer( const int32_t idx ) const
  {
    return &Array[ idx ];
  }

  void FreeArray()
  {
    for ( int32_t x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete Array[ x ];
    FlushFast();
  }

  void FreeArrayA()
  {
    for ( int32_t x = NumItems() - 1; x >= 0; x-- )
      if ( Array[ x ] )
        delete[] Array[ x ];
    FlushFast();
  }

  void TrimHead( int32_t count )
  {
    if ( count < 0 ) return;

    if ( count >= ItemCount )
    {
      FlushFast();
      return;
    }

    for ( int32_t x = 0; x < ItemCount - count; x++ )
      Array[ x ] = Array[ x + count ];
    ItemCount -= count;
  }

  void Expand( TU32 AddedItemCount )
  {
    if ( !Array )
    {
      Array = new ItemType[ AddedItemCount ];
      Capacity = AddedItemCount;
      ItemCount = 0;
      return;
    }

    ItemType *NewArray = new ItemType[ ItemCount + AddedItemCount ];
    for ( int32_t x = 0; x < ItemCount; x++ )
      NewArray[ x ] = Array[ x ];

    delete[] Array;
    Array = NewArray;
    Capacity = ItemCount + AddedItemCount;
  }
};

template<typename ItemType> __inline void SimulateAddItem( ItemType *&dataArray, int32_t &numItems, ItemType newItem )
{
  if ( !dataArray )
  {
    dataArray = new ItemType[ 1 ];
    dataArray[ 0 ] = newItem;
    numItems++;
    return;
  }

  ItemType *n = new ItemType[ numItems + 1 ];
  for ( int32_t x = 0; x < numItems; x++ )
    n[ x ] = dataArray[ x ];
  n[ numItems ] = newItem;

  if ( dataArray )
    delete[] dataArray;

  dataArray = n;

  numItems++;
  return;
}

template<typename ItemType> __inline void SimulateDeleteByIndex( ItemType *&dataArray, int32_t &numItems, int32_t index )
{
  if ( !dataArray )
    return;
  if ( index < 0 || index >= numItems )
    return;

  numItems--;
  for ( int x = index; x < numItems; x++ )
    dataArray[ x ] = dataArray[ x + 1 ];

  return;
}

template<typename ItemType> __inline void SimulateFreeArray( ItemType *&dataArray, int32_t &numItems )
{
  if ( !dataArray )
    return;

  for ( int32_t x = numItems - 1; x >= 0; x-- )
    if ( dataArray[ x ] )
      delete dataArray[ x ];

  return;
}


template <typename ItemType> class CArrayThreadSafe
{
  LIGHTWEIGHT_CRITICALSECTION critsec;
  CArray<ItemType> Array;

protected:

  int32_t GetCapacity()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.GetCapacity();
  }

public:
  typedef int32_t( __cdecl *ARRAYSORTCALLBACK )( ItemType *a, ItemType *b );

	CArrayThreadSafe() //: CArray<ItemType>()
  {
    InitializeLightweightCS( &critsec );
  }

	CArrayThreadSafe(int32_t Size) //: CArray<ItemType>(Size)
  {
    InitializeLightweightCS( &critsec );
    Array.Expand( Size );
  }

  CArrayThreadSafe( const CArray<ItemType> &original ) : CArray<ItemType>( original )
  {
    InitializeLightweightCS( &critsec );
  }

  CArrayThreadSafe( const CArrayThreadSafe<ItemType> &original ) : CArray<ItemType>( original )
  {
    InitializeLightweightCS( &critsec );
  }

  ~CArrayThreadSafe()
  {
  }

  int32_t NumItems()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.NumItems();
  }

  void Flush()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Flush();
  }

  void FlushFast()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FlushFast();
  }

  void Add( const ItemType &Item )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Add( Item );
  }

  CArrayThreadSafe<ItemType> &operator= ( const CArrayThreadSafe<ItemType> &a )
  {
    for ( int i = 0; i < a.NumItems(); i++ )
      Add( a[ i ] );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator+= ( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator +=( i );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator+= ( const CArray<ItemType> &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator +=( i );
    return *this;
  }

  CArrayThreadSafe<ItemType> &operator-= ( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.operator -=( i );
    return *this;
  }

  int32_t AddUnique( const ItemType &Item )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.AddUnique( Item );
  }

  ItemType &operator[]( const int32_t idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.operator[]( idx );
  }

  ItemType &Last()
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Last();
  }

  int32_t const Find( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Find( i );
  }

  void DeleteByIndex( const int32_t idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.DeleteByIndex( idx );
  }

  void Delete( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Delete( i );
  }

  void FreeByIndex( const int32_t idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeByIndex( idx );
  }

  void Free( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.Free( i );
  }

  void FreeAByIndex( const int32_t idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeAByIndex( idx );
  }

  void FreeA( const ItemType &i )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.FreeA( i );
  }

  void Swap( const int32_t a, const int32_t b )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Swap( a, b );
  }

  void Sort( ARRAYSORTCALLBACK SortFunct )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Sort( SortFunct );
  }

  ItemType *GetPointer( const int32_t idx )
  {
    CLightweightCriticalSection cs( &critsec );
    return Array.GetPointer( idx );
  }

  void FreeArray()
  {
    CLightweightCriticalSection cs( &critsec );
    FreeArray();
  }

  void FreeArrayA()
  {
    CLightweightCriticalSection cs( &critsec );
    FreeArrayA();
  }

  void Expand( int32_t size )
  {
    CLightweightCriticalSection cs( &critsec );
    Array.Expand( size );
  }
};
