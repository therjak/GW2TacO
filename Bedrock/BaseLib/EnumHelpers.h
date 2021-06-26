#pragma once

struct EnumNamePair
{
  int32_t Value;
  const TCHAR *Name;
};

TBOOL FindEnumByName( EnumNamePair *Pairs, TCHAR *Name, int32_t &Result );
TBOOL FindEnumByName( EnumNamePair *Pairs, CString &Name, int32_t &Result );
const TCHAR *FindNameByEnum( EnumNamePair *Pairs, int32_t Enum );
