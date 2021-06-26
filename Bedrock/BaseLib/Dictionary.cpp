#include "BaseLib.h"

TU32 DictionaryHash( const int32_t &i )
{
  return i;
}

TU32 DictionaryHash( const void *i ) //hash for a pointer
{
  int64_t v = (int64_t)i;

  int32_t c;

  //djb2 hash
  TU32 Hash = 5381;
  while ( c = v & 0xff )
  {
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c
    v = v >> 8;
  }
  return Hash;

}
