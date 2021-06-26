#pragma once

#include <functional>

#ifdef HASHED_STRINGS
#define CALCULATEHASH() CalculateHash()
#else
#define CALCULATEHASH() do {} while (0)
#endif

#define DEFAULTINTFORMAT _T("%d")
#define DEFAULTFLOATFORMAT _T("%.2f")

class CStringArray;
class CStringArrayMarkers;

class CString
{
  friend CString;
  TCHAR *String = nullptr;
  TU32 LengthCached = 0;

  void Initialize();
  void StringChanged();

public:

  CString();
  virtual ~CString();

  CString( const CString &str );
  CString( const wchar_t *str );
  CString( const TS8 *str );

  CString( const CString &str, const TU32 len );
  CString( const TS8 *str, const TU32 len );
  CString( const wchar_t *str, const TU32 len );

  TCHAR *GetPointer() const
  {
    return String;
  }

  void WriteAsMultiByte( TS8 *Copy, TU32 Size ) const;
  void WriteAsWideChar( wchar_t *Copy, TU32 Size ) const;

  //////////////////////////////////////////////////////////////////////////
  //string functions

  const CString &operator=( const CString &str );
  CString operator+( const CString &str ) const;
  const CString &operator+=( const CString &str );

  const CString &Append( const CString &str );
  const CString &Append( const TS8 *str );
  const CString &Append( const wchar_t *str );

  const CString &Append( const CString &str, const TU32 len );
  const CString &Append( const TS8 *str, const TU32 len );
  const CString &Append( const wchar_t *str, const TU32 len );

  //////////////////////////////////////////////////////////////////////////
  //char functions

  const CString &operator=( const TS8* str );
  CString operator+( const TS8 *str ) const;
  CString &operator+=( const TS8 *str );
  const friend CString operator+( const TS8* str, const CString &str2 );

  //////////////////////////////////////////////////////////////////////////
  //wchar_t functions

  const CString &operator=( const wchar_t* str );
  CString operator+( const wchar_t *str ) const;
  CString &operator+=( const wchar_t *str );
  friend CString operator+( const wchar_t* str, const CString &str2 );

  //////////////////////////////////////////////////////////////////////////
  //numeric functions

  CString operator+( const int32_t v ) const;
  CString &operator+=( const int32_t v );
  const friend CString operator+( const int32_t v, const CString &str );

  CString operator+( const long v ) const;
  CString &operator+=( const long v );
  const friend CString operator+( const long v, const CString &str );

  CString operator+( const TU32 v ) const;
  CString &operator+=( const TU32 v );
  const friend CString operator+( const TU32 v, const CString &str );

  CString operator+( const unsigned long v ) const;
  CString &operator+=( const unsigned long v );
  const friend CString operator+( const unsigned long v, const CString &str );

  CString operator+( const TF32 v ) const;
  CString &operator+=( const TF32 v );
  const friend CString operator+( const TF32 v, const CString &str );

  //////////////////////////////////////////////////////////////////////////
  //comparison functions

  TBOOL operator== ( const CString &v ) const;
  TBOOL operator== ( const TS8 *v ) const;
  TBOOL operator== ( const wchar_t *v ) const;

  TBOOL operator!= ( const CString &v ) const;
  TBOOL operator!= ( const TS8 *v ) const;
  TBOOL operator!= ( const wchar_t *v ) const;

  TBOOL operator<( const CString &v ) const;
  TBOOL operator<( const TS8 *v ) const;
  TBOOL operator<( const wchar_t *v ) const;

  TBOOL operator>( const CString &v ) const;
  TBOOL operator>( const TS8 *v ) const;
  TBOOL operator>( const wchar_t *v ) const;

  TCHAR &operator[]( const int32_t idx ) const;

  int32_t Find( const CString &v, TU32 nStart = 0 ) const;
  int32_t Find( const TS8 *v, TU32 nStart = 0 ) const;
  int32_t Find( const wchar_t *v, TU32 nStart = 0 ) const;

  TU32 GetSubstringCount( CString &v ) const;
  TU32 GetSubstringCount( const TS8 *v ) const;
  TU32 GetSubstringCount( const wchar_t *v ) const;

  CString Substring( int32_t nStart ) const;
  CString Substring( int32_t nStart, int32_t nLength ) const;

  CString Trimmed() const;

  static int32_t CompareNoCase( const CString &a, const CString &b );
  static CString EncodeToBase64( TU8 *Data, int32_t Length );
  void DecodeBase64( TU8 *&Data, int32_t &Length );

  //////////////////////////////////////////////////////////////////////////
  // misc

  TU32 Length() const;
  //static CString Format(const wchar_t *format, ...);
  static CString Format( const TCHAR *format, ... );
  //static void FormatVA(const wchar_t *format, va_list list, CString &Result);
  static void FormatVA( const TCHAR *format, va_list list, CString &Result );
  int32_t Scan( const TCHAR *format, ... );
  int32_t Scan( const CString format, ... );
  void ScanVA( const TCHAR *format, va_list list, int32_t &Result );

  CStringArray Explode( CString &sDelimiter ) const;
  CStringArray Explode( TCHAR *sDelimiter ) const;
  CStringArray ExplodeByWhiteSpace() const;

  CStringArrayMarkers GetExplodeMarkers( CString &sDelimiter ) const;
  CStringArrayMarkers GetExplodeMarkers( TCHAR *sDelimiter ) const;
  CStringArrayMarkers GetExplodeMarkersByWhiteSpace() const;

  static int32_t Strcmp( const TCHAR *str, const TCHAR *str2 );
  static int32_t Strncmp( const TCHAR *str, const TCHAR *str2, const int32_t len );
  static TF32 Atof( const TCHAR *str );
  static int32_t Atoi( const TCHAR *str );

  typedef std::function<TBOOL( TU32 )> UTF8CHARCALLBACK;

  void DecodeUtf8( UTF8CHARCALLBACK callback );
  static void DecodeUtf8( const TCHAR* Input, UTF8CHARCALLBACK callback );

  //////////////////////////////////////////////////////////////////////////
  // manipulation functions

  void Insert( int32_t pos, const TCHAR Input );
  void Insert( int32_t pos, const TCHAR *Input );
  void DeleteChar( int32_t pos );
  void DeleteRegion( int32_t pos, int32_t size );
  void ToUnixNewline();
  void ToWindowsNewline();
  void ToLower();
  void ToUpper();
  void RemoveNewLines();
  void URLEncode();

#ifdef HASHED_STRINGS
  //////////////////////////////////////////////////////////////////////////
  //hashing

  TU32 GetHash() const;

private:
  TU32 Hash;
  TU32 LowercaseHash;
  void CalculateHash();
#endif
};

struct CStringMarker
{
  int32_t start = 0;
  int32_t length = 0;

  CStringMarker() = default;
  CStringMarker( int32_t start, int32_t length ) : start( start ), length( length ) {};
};

class CStringArrayMarkers : public CArray<CStringMarker>
{
public:
};

class CStringArray : public CArray<CString>
{
public:
  CString Implode( CString &sDelimiter );
  CString Implode( TCHAR *sDelimiter );
};

TU32 DictionaryHash( const CString &i );
int32_t EstimateStringFormatLength( const wchar_t *szFormat, va_list vlArgList );
int32_t EstimateStringFormatLength( const TS8 *szFormat, va_list vlArgList );
