#pragma once
#include "CriticalSection.h"
#include "Assert.h"

#include <vector>
#include <algorithm>

#define HASHEXPANSIONTHRESHOLD 0.7f
#define HASHCOLLAPSETHRESHOLD 0.2f

// TODO(therjak): This seems to be a hashmap.
template <typename KeyType, typename ItemType> class CDictionary
{
public:

  class KDPair
  {
  public:
    KeyType Key;
    uint32_t Hash;
    ItemType Data;
    KDPair *Next;

    KDPair( const KeyType &k, const ItemType &i )
    {
      Key = k;
      Data = i;
      Hash = DictionaryHash( k );
    }

    KDPair( const KeyType &k )
    {
      Key = k;
      Hash = DictionaryHash( k );
    }

    virtual ~KDPair()
    {
      delete Next;
      Next = 0;
    }
  };

protected:

  int32_t ItemCount;
  int32_t TableSize;
  KDPair **HashTable;

  ItemType _dummy; //return item for out of bounds requests

  virtual void Insert( KDPair *p )
  {
    int32_t idx = p->Hash%TableSize;
    p->Next = HashTable[ idx ];
    HashTable[ idx ] = p;
  }

  virtual void ResizeTable( const int32_t NewSize )
  {
    if ( NewSize < 8 ) return; //that's small enough

    KDPair **OldTable = HashTable;
    int32_t OldSize = TableSize;

    //expand hash table
    TableSize = NewSize;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );

    //re-add items
    for ( int32_t x = 0; x < OldSize; x++ )
    {
      while ( OldTable[ x ] )
      {
        KDPair *Next = OldTable[ x ]->Next;
        Insert( OldTable[ x ] );
        OldTable[ x ] = Next;
      }
    }

    delete[] OldTable; //no need to delete items as they have been moved
  }

  virtual KDPair *AddNew( const KeyType &Key )
  {
    float Fill = ItemCount / (float)TableSize;
    if ( Fill >= HASHEXPANSIONTHRESHOLD )
      ResizeTable( TableSize * 2 );

    KDPair *p = new KDPair( Key );
    Insert( p );

    ItemCount++;

    return p;
  }

  virtual KDPair *Find( const KeyType &Key ) const
  {
    uint32_t idx = DictionaryHash( Key ) % TableSize;

    KDPair *p = HashTable[ idx ];

    while ( p )
    {
      if ( p->Key == Key ) return p;
      p = p->Next;
    }

    return 0;
  }

public:
  typedef void( __cdecl *DICTIONARYPROCESSCALLBACK )( ItemType &a );

  CDictionary( int32_t tblsize = 8 )
  {
    TableSize = tblsize;
    ItemCount = 0;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );
  }

  virtual ~CDictionary()
  {
    for ( int32_t x = 0; x < TableSize; x++ )
    {
      delete HashTable[ x ];
      HashTable[ x ] = 0;
    }
    delete[] HashTable;
  }

  CDictionary( const CDictionary<KeyType, ItemType> &dict )
  {
    TableSize = 8;
    ItemCount = 0;
    HashTable = new KDPair*[ TableSize ];
    memset( HashTable, 0, sizeof( KDPair* )*TableSize );

    *this += dict;
  }

  virtual ItemType &operator[]( const KeyType &Key )
  {
    KDPair *p = Find( Key );
    if ( p ) return p->Data;
    p = AddNew( Key );
    return p->Data;
  }

  ItemType &GetByKey( const KeyType &Key )
  {
    return ( *this )[ Key ];
  }

  ItemType GetExisting( const KeyType &Key ) //this can only be used on dictionaries where the ItemType is a pointer!!
  {
    KDPair *p = Find( Key );
    if ( p ) return p->Data;
    return 0;
  }

  virtual void Add( const KeyType &Key, const ItemType &Data )
  {
    ( *this )[ Key ] = Data;
  }

  virtual void Delete( const KeyType &Key )
  {
    uint32_t idx = DictionaryHash( Key ) % TableSize;

    KDPair *p = HashTable[ idx ];
    KDPair *Previous = 0;

    while ( p )
    {
      KDPair *PNext = p->Next;

      if ( p->Key == Key )
      {
        if ( Previous )
          Previous->Next = p->Next;
        else
          HashTable[ idx ] = p->Next;

        p->Next = 0;
        delete p;
        ItemCount--;
      }
      else
        Previous = p;
      p = PNext;
    }

    //////////////////////////////////////////////////////////////////////////
    // The following resize functionality was originally at the beginning of 
    // the function. However that resulted in some yet to be explained 
    // crashes. Moving it to after the delete occurred fixed the issue but
    // further investigation might be required.

    float Fill = ItemCount / (float)TableSize;
    if ( Fill < HASHCOLLAPSETHRESHOLD )
      ResizeTable( TableSize / 2 );
  }

  void Free( const KeyType &Key )
  {
    if ( HasKey( Key ) )
    {
      ItemType i = GetByKey( Key );
      if ( i ) delete i;
    }
    Delete( Key );
  }

  void FreeA( const KeyType &Key )
  {
    if ( HasKey( Key ) )
    {
      ItemType i = GetByKey( Key );
      if ( i ) delete[] i;
    }
    Delete( Key );
  }

  void FreeByIndex( const int32_t idx )
  {
    ItemType i = GetByIndex( idx );
    if ( i ) delete i;
    DeleteByIndex( idx );
  }

  void FreeAByIndex( const int32_t idx )
  {
    ItemType i = GetByIndex( idx );
    if ( i ) delete[] i;
    DeleteByIndex( idx );
  }

  void FreeAll()
  {
    for ( int32_t x = NumItems() - 1; x >= 0; x-- )
      FreeByIndex( x );
  }

  void FreeAllA()
  {
    for ( int32_t x = NumItems() - 1; x >= 0; x-- )
      FreeAByIndex( x );
  }

  virtual void DeleteByIndex( const int32_t idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    int32_t cntr = 0;

    for ( int32_t x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
        {
          Delete( p->Key );
          return;
        }
        p = p->Next;
        cntr++;
      }
    }
  }

  TBOOL HasKey( const KeyType &Key ) const
  {
    return Find( Key ) != 0;
  }

  virtual int32_t NumItems() const
  {
    return ItemCount;
  }

  virtual ItemType &GetByIndex( int32_t idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    int32_t cntr = 0;

    for ( int32_t x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
          return p->Data;
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return _dummy;
  }

  virtual KDPair *GetKDPair( int32_t idx )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    int32_t cntr = 0;

    for ( int32_t x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
          return p;
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return 0;
  }

  virtual ItemType &GetByIndex( int32_t idx, KeyType &Key )
  {
    BASEASSERT( idx >= 0 && idx < ItemCount );

    int32_t cntr = 0;

    for ( int32_t x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        if ( cntr == idx )
        {
          Key = p->Key;
          return p->Data;
        }
        p = p->Next;
        cntr++;
      }
    }

    //out of bounds, undefined behavior:
    return _dummy;
  }

  virtual void ForEach( const DICTIONARYPROCESSCALLBACK Callback ) const
  {
    for ( int32_t x = 0; x < TableSize; x++ )
    {
      KDPair *p = HashTable[ x ];
      while ( p )
      {
        Callback( p->Data );
        p = p->Next;
      }
    }
  }

  virtual void Flush()
  {
    for ( int32_t x = 0; x < TableSize; x++ )
    {
      delete HashTable[ x ];
      HashTable[ x ] = 0;
    }
    ItemCount = 0;
  }

  virtual CDictionary<KeyType, ItemType> &operator+= ( const CDictionary<KeyType, ItemType> &i )
  {
    for ( int32_t x = 0; x < i.TableSize; x++ )
    {
      KDPair *p = i.HashTable[ x ];
      while ( p )
      {
        ( *this )[ p->Key ] = p->Data;
        p = p->Next;
      }
    }
    return *this;
  }

  CDictionary<KeyType, ItemType> operator+( const CDictionary<KeyType, ItemType> &d1 ) const
  {
    CDictionary<KeyType, ItemType> dr;
    dr += *this;
    dr += d1;
    return dr;
  }

  const CDictionary<KeyType, ItemType> &operator=( const CDictionary<KeyType, ItemType> &dict )
  {
    if ( &dict == this ) return *this;
    Flush();
    ( *this ) += dict;
    return *this;
  }

};

template <typename KeyType, typename ItemType> class CDictionaryEnumerable : public CDictionary<KeyType, ItemType>
{

#define SORTSTACKSIZE (8*sizeof(void*) - 2)

public:
  typedef int32_t( *KEYSORTCALLBACK )( const KeyType &a, const KeyType &b );
  typedef int32_t( *VALUESORTCALLBACK )( const ItemType &a, const ItemType &b );

private:

  std::vector<KDPair*> IndexMap;

  void swap( int32_t a, int32_t b )
  {
    KDPair *k = IndexMap[ a ];
    IndexMap[ a ] = IndexMap[ b ];
    IndexMap[ b ] = k;
  }

  KDPair *AddNew(const KeyType &Key) override {
    KDPair *p = CDictionary<KeyType, ItemType>::AddNew( Key );
    IndexMap.push_back( p );
    return p;
  }

public:

  CDictionaryEnumerable() : CDictionary<KeyType, ItemType>()
  {
  }


  CDictionaryEnumerable( const CDictionaryEnumerable<KeyType, ItemType> &dict )
  {
    *this += dict;
  }

  void Add(const KeyType &Key, const ItemType &Data) override {
    KDPair *p = Find( Key );
    CDictionary<KeyType, ItemType>::Add( Key, Data );
    if ( !p )
    {
      p = Find( Key );
      IndexMap.push_back( p );
    }
  }

  void Delete(const KeyType &Key) override {
    KDPair *p = Find( Key );
    CDictionary<KeyType, ItemType>::Delete( Key );
    if ( p )
      IndexMap.erase(std::remove(IndexMap.begin(), IndexMap.end(), p));
  }

  ItemType &GetByIndex(int32_t idx) override {
    BASEASSERT(idx >= 0 && idx < this->ItemCount);
    return ( (KDPair*)IndexMap[ idx ] )->Data;
  }

  void DeleteByIndex(const int32_t idx) override {
    BASEASSERT(idx >= 0 && idx < this->ItemCount);
    Delete( IndexMap[ idx ]->Key );
  }

  ItemType &GetByIndex(int32_t idx, KeyType &Key) override {
    BASEASSERT(idx >= 0 && idx < this->ItemCount);
    Key = ( (KDPair*)IndexMap[ idx ] )->Key;
    return ( (KDPair*)IndexMap[ idx ] )->Data;
  }

  virtual KDPair* GetKDPairByIndex( int32_t idx )
  {
    return (KDPair*)IndexMap[ idx ];
  }

  virtual void SortByKey( KEYSORTCALLBACK SortCallback )
  {

    if ( !SortCallback ) return;
    if (this->ItemCount < 2) return;

    //qsort implementation - calling crt qsort isn't viable here due to template hackery
    //implementation taken from crt

    uint32_t lostk[ SORTSTACKSIZE ], histk[ SORTSTACKSIZE ];
    int32_t stkptr = 0;

    uint32_t lo = 0;
    uint32_t hi = this->ItemCount - 1;

  recurse:

    uint32_t size = ( hi - lo ) + 1;

    if ( size <= 8 ) //cutoff = 8
    {
      uint32_t _hi = hi, _lo = lo;

      while ( _hi > _lo )
      {
        uint32_t max = _lo;
        for ( uint32_t p = _lo + 1; p <= _hi; p++ )
          if ( SortCallback( IndexMap[ p ]->Key, IndexMap[ max ]->Key ) > 0 )
            max = p;
        swap( max, _hi );
        _hi--;
      }
    }
    else
    {
      uint32_t mid = lo + ( size / 2 );

      if ( SortCallback( IndexMap[ lo ]->Key, IndexMap[ mid ]->Key ) > 0 ) swap( lo, mid );
      if ( SortCallback( IndexMap[ lo ]->Key, IndexMap[ hi ]->Key ) > 0 ) swap( lo, hi );
      if ( SortCallback( IndexMap[ mid ]->Key, IndexMap[ hi ]->Key ) > 0 ) swap( mid, hi );

      uint32_t loguy = lo;
      uint32_t higuy = hi;

      for ( ;;)
      {
        if ( mid > loguy ) do { loguy++; } while ( loguy < mid && SortCallback( IndexMap[ loguy ]->Key, IndexMap[ mid ]->Key ) <= 0 );
        if ( mid <= loguy ) do { loguy++; } while ( loguy <= hi && SortCallback( IndexMap[ loguy ]->Key, IndexMap[ mid ]->Key ) <= 0 );
        do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) > 0 );
        if ( higuy < loguy ) break;
        swap( loguy, higuy );
        if ( mid == higuy ) mid = loguy;
      }

      higuy++;

      if ( mid < higuy ) do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) == 0 );
      if ( mid >= higuy ) do { higuy--; } while ( higuy > lo && SortCallback( IndexMap[ higuy ]->Key, IndexMap[ mid ]->Key ) == 0 );

      if ( higuy - lo >= hi - loguy )
      {
        if ( lo < higuy ) { lostk[ stkptr ] = lo; histk[ stkptr ] = higuy; ++stkptr; }
        if ( loguy < hi ) { lo = loguy; goto recurse; }
      }
      else
      {
        if ( loguy < hi ) { lostk[ stkptr ] = loguy; histk[ stkptr ] = hi; ++stkptr; }
        if ( lo < higuy ) { hi = higuy; goto recurse; }
      }
    }

    --stkptr;
    if ( stkptr >= 0 ) { lo = lostk[ stkptr ]; hi = histk[ stkptr ]; goto recurse; }

  }

  virtual void SortByValue( VALUESORTCALLBACK SortCallback )
  {
    if ( !SortCallback ) return;

    //qsort implementation - calling crt qsort isn't viable here due to template hackery
    //implementation taken from crt

    uint32_t lostk[ SORTSTACKSIZE ], histk[ SORTSTACKSIZE ];
    int32_t stkptr = 0;

    uint32_t lo = 0;
    uint32_t hi = this->ItemCount - 1;

  recurse:

    uint32_t size = ( hi - lo ) + 1;

    if ( size <= 8 ) //cutoff = 8
    {
      uint32_t _hi = hi, _lo = lo;

      while ( _hi > _lo )
      {
        uint32_t max = _lo;
        for ( uint32_t p = _lo + 1; p <= _hi; p++ )
          if ( SortCallback( IndexMap[ p ]->Data, IndexMap[ max ]->Data ) > 0 )
            max = p;
        swap( max, _hi );
        _hi--;
      }
    }
    else
    {
      uint32_t mid = lo + ( size / 2 );

      if ( SortCallback( IndexMap[ lo ]->Data, IndexMap[ mid ]->Data ) > 0 ) swap( lo, mid );
      if ( SortCallback( IndexMap[ lo ]->Data, IndexMap[ hi ]->Data ) > 0 ) swap( lo, hi );
      if ( SortCallback( IndexMap[ mid ]->Data, IndexMap[ hi ]->Data ) > 0 ) swap( mid, hi );

      uint32_t loguy = lo;
      uint32_t higuy = hi;

      for ( ;;)
      {
        if ( mid > loguy ) do { loguy++; } while ( loguy < mid && SortCallback( IndexMap[ loguy ]->Data, IndexMap[ mid ]->Data ) <= 0 );
        if ( mid <= loguy ) do { loguy++; } while ( loguy <= hi && SortCallback( IndexMap[ loguy ]->Data, IndexMap[ mid ]->Data ) <= 0 );
        do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) > 0 );
        if ( higuy < loguy ) break;
        swap( loguy, higuy );
        if ( mid == higuy ) mid = loguy;
      }

      higuy++;

      if ( mid < higuy ) do { higuy--; } while ( higuy > mid && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) == 0 );
      if ( mid >= higuy ) do { higuy--; } while ( higuy > lo && SortCallback( IndexMap[ higuy ]->Data, IndexMap[ mid ]->Data ) == 0 );

      if ( higuy - lo >= hi - loguy )
      {
        if ( lo < higuy ) { lostk[ stkptr ] = lo; histk[ stkptr ] = higuy; ++stkptr; }
        if ( loguy < hi ) { lo = loguy; goto recurse; }
      }
      else
      {
        if ( loguy < hi ) { lostk[ stkptr ] = loguy; histk[ stkptr ] = hi; ++stkptr; }
        if ( lo < higuy ) { hi = higuy; goto recurse; }
      }
    }

    --stkptr;
    if ( stkptr >= 0 ) { lo = lostk[ stkptr ]; hi = histk[ stkptr ]; goto recurse; }

  }

  void Flush() override {
    CDictionary<KeyType, ItemType>::Flush();
    IndexMap.clear();
  }

  virtual CDictionaryEnumerable<KeyType, ItemType> &operator+= ( const CDictionaryEnumerable<KeyType, ItemType> &i )
  {
    for ( int32_t x = 0; x < i.NumItems(); x++ )
    {
      KDPair *p = i.IndexMap[ x ];
      ( *this )[ p->Key ] = p->Data;
    }
    return *this;
  }

  CDictionaryEnumerable<KeyType, ItemType> operator+( const CDictionaryEnumerable<KeyType, ItemType> &d1 ) const
  {
    CDictionaryEnumerable<KeyType, ItemType> dr;
    dr += *this;
    dr += d1;
    return dr;
  }

  const CDictionaryEnumerable<KeyType, ItemType> &operator=( const CDictionaryEnumerable<KeyType, ItemType> &dict )
  {
    Flush();
    ( *this ) += dict;
    return *this;
  }
};


uint32_t DictionaryHash( const int32_t &i );
uint32_t DictionaryHash( const void *i );
