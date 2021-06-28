#include "BaseLib.h"


TBOOL FindEnumByName( EnumNamePair *Pairs, TCHAR *Name, int32_t &Result )
{
  for ( int32_t x = 0; Pairs[ x ].Name; x++ )
  {
    if ( Name == Pairs[ x ].Name )
    {
      Result = Pairs[ x ].Value;
      return true;
    }
  }
  return false;
}

TBOOL FindEnumByName( EnumNamePair *Pairs, CString &Name, int32_t &Result )
{
  return FindEnumByName( Pairs, Name.GetPointer(), Result );
}

const TCHAR *FindNameByEnum( EnumNamePair *Pairs, int32_t Enum )
{
  TCHAR *Name = NULL;
  for ( int32_t x = 0; Pairs[ x ].Name; x++ )
  {
    if ( Enum == Pairs[ x ].Value )
      return Pairs[ x ].Name;
  }
  return Name;
}
