#include "BaseLib.h"

void CString::Initialize()
{
  String = NULL;
  LengthCached = 0;
#ifdef HASHED_STRINGS
  Hash = LowercaseHash = 0;
#endif
}

void CString::StringChanged()
{
  LengthCached = 0;
  if ( String ) LengthCached = (int32_t)_tcslen( String );
  CALCULATEHASH();
}

CString::CString()
{
  Initialize();
}

CString::~CString()
{
  SAFEDELETEA( String );
}

CString::CString( const CString &str )
{
  Initialize();
  Append( str );
}

CString::CString( const TS8 *str )
{
  Initialize();
  Append( str );
}

CString::CString( const wchar_t *str )
{
  Initialize();
  Append( str );
}

CString::CString( const CString &str, const uint32_t len )
{
  Initialize();
  Append( str, len );
}

CString::CString( const TS8 *str, const uint32_t len )
{
  Initialize();
  Append( str, len );
}

CString::CString( const wchar_t *str, const uint32_t len )
{
  Initialize();
  Append( str, len );
}

void CString::WriteAsMultiByte( TS8 *Copy, uint32_t SizeInChars ) const
{
  if ( !Copy || !SizeInChars ) return;
  int32_t len = min( Length(), SizeInChars - 1 );

#ifdef UNICODE
  WideCharToMultiByte( CP_UTF8, 0, String, -1, Copy, len, NULL, NULL );
#else
  _tcsncpy_s( Copy, SizeInChars, String, len );
#endif

  Copy[ len ] = 0;
}


void CString::WriteAsWideChar( wchar_t *Copy, uint32_t SizeInChars ) const
{
  if ( !Copy || !SizeInChars ) return;
  int32_t len = min( Length(), SizeInChars - 1 );

#ifdef UNICODE
  _tcsncpy_s( Copy, SizeInChars, String, len );
#else
  MultiByteToWideChar( CP_UTF8, 0, String, -1, Copy, len );
#endif

  Copy[ len ] = 0;
}

//////////////////////////////////////////////////////////////////////////
//string functions

const CString &CString::operator=( const CString &str )
{
  SAFEDELETEA( String );
  LengthCached = 0;
  Initialize();
  Append( str );
  return *this;
}

CString CString::operator+( const CString &str ) const
{
  return CString( *this ).Append( str );
}

const CString &CString::operator+=( const CString &str )
{
  Append( str );
  return *this;
}

const CString &CString::Append( const CString &str, const uint32_t len )
{
  int32_t lengthOriginal = Length();
  int32_t lengthIncoming = len;//str.Length();

  TCHAR *newString = new TCHAR[ lengthOriginal + lengthIncoming + 1 ];

  if ( lengthOriginal )	memcpy( newString, String, lengthOriginal * sizeof( TCHAR ) );
  if ( lengthIncoming ) memcpy( newString + lengthOriginal, str.String, lengthIncoming * sizeof( TCHAR ) );
  newString[ lengthOriginal + lengthIncoming ] = 0; //null terminate

  if ( String ) delete[] String;
  String = newString;

  StringChanged();

  return *this;
}

const CString &CString::Append( const CString &str )
{
  return Append( str, str.Length() );
}
//////////////////////////////////////////////////////////////////////////
//char functions

const CString &CString::operator=( const TS8 *str )
{
  SAFEDELETEA( String );
  LengthCached = 0;
  Append( str );
  return *this;
}

CString CString::operator+( const TS8 *str ) const
{
  return CString( *this ).Append( str );
}

CString &CString::operator+=( const TS8 *str )
{
  Append( str );
  return *this;
}

const CString operator+( const TS8 *str, const CString &str2 )
{
  return CString( str ).Append( str2 );
}

const CString &CString::Append( const TS8 *str, const uint32_t len )
{
  int32_t lengthOriginal = Length();
  int32_t lengthIncoming = len;
  int32_t lengthCalculated = lengthIncoming;

#ifdef UNICODE
  lengthCalculated = MultiByteToWideChar( CP_UTF8, 0, str, lengthIncoming, NULL, 0 );
#endif

  TCHAR *newString = new TCHAR[ lengthOriginal + lengthCalculated + 1 ];

  if ( lengthOriginal )	memcpy( newString, String, lengthOriginal * sizeof( TCHAR ) );

  if ( lengthCalculated )
  {
#ifdef UNICODE
    MultiByteToWideChar( CP_UTF8, 0, str, lengthIncoming, newString + lengthOriginal, lengthCalculated );
#else
    memcpy( newString + lengthOriginal, str, lengthCalculated );
#endif
  }

  newString[ lengthOriginal + lengthCalculated ] = 0; //null terminate

  if ( String ) delete[] String;
  String = newString;

  LengthCached = lengthOriginal + lengthCalculated;
  CALCULATEHASH();

  return *this;
}

const CString &CString::Append( const TS8 *str )
{
  if ( !str ) return *this;
  return Append( str, (int32_t)strlen( str ) );
}

//////////////////////////////////////////////////////////////////////////
//wchar_t functions

const CString &CString::operator=( const wchar_t *str )
{
  SAFEDELETEA( String );
  LengthCached = 0;
  Append( str );
  return *this;
}

CString CString::operator+( const wchar_t *str ) const
{
  return CString( *this ).Append( str );
}

CString &CString::operator+=( const wchar_t *str )
{
  Append( str );
  return *this;
}

CString operator+( const wchar_t *str, const CString &str2 )
{
  return CString( str ).Append( str2 );
}

const CString &CString::Append( const wchar_t *str, const uint32_t len )
{
  int32_t lengthOriginal = Length();
  int32_t lengthIncoming = len;
  int32_t lengthCalculated = lengthIncoming;

#ifndef UNICODE
  lengthCalculated = WideCharToMultiByte( CP_UTF8, 0, str, lengthIncoming, NULL, 0, NULL, NULL );
#endif

  TCHAR *newString = new TCHAR[ lengthOriginal + lengthCalculated + 1 ];

  if ( lengthOriginal )	memcpy( newString, String, lengthOriginal * sizeof( TCHAR ) );

  if ( lengthCalculated )
  {
#ifndef UNICODE
    WideCharToMultiByte( CP_UTF8, 0, str, lengthIncoming, newString + lengthOriginal, lengthCalculated, NULL, NULL );
#else
    memcpy( newString + lengthOriginal, str, lengthCalculated * sizeof( TCHAR ) );
#endif
  }

  newString[ lengthOriginal + lengthCalculated ] = 0; //null terminate

  if ( String ) delete[] String;
  String = newString;

  StringChanged();

  return *this;
}

const CString &CString::Append( const wchar_t *str )
{
  if ( !str ) return *this;
  return Append( str, (int32_t)wcslen( str ) );
}


//////////////////////////////////////////////////////////////////////////
// numeric functions

//int
CString CString::operator+( const int32_t v ) const
{
  return ( *this ) + CString::Format( DEFAULTINTFORMAT, v );
}

CString &CString::operator+=( const int32_t v )
{
  Append( CString::Format( DEFAULTINTFORMAT, v ) );
  return *this;
}

const CString operator+( const int32_t v, const CString &str )
{
  return CString::Format( DEFAULTINTFORMAT, v ) + str;
}

//long
CString CString::operator+( const long v ) const
{
  return ( *this ) + CString::Format( DEFAULTINTFORMAT, v );
}

CString &CString::operator+=( const long v )
{
  Append( CString::Format( DEFAULTINTFORMAT, v ) );
  return *this;
}

const CString operator+( const long v, const CString &str )
{
  return CString::Format( DEFAULTINTFORMAT, v ) + str;
}

//unsigned int
CString CString::operator+( const uint32_t v ) const
{
  return ( *this ) + CString::Format( DEFAULTINTFORMAT, v );
}

CString &CString::operator+=( const uint32_t v )
{
  Append( CString::Format( DEFAULTINTFORMAT, v ) );
  return *this;
}

const CString operator+( const uint32_t v, const CString &str )
{
  return CString::Format( DEFAULTINTFORMAT, v ) + str;
}

//unsigned long
CString CString::operator+( const unsigned long v ) const
{
  return ( *this ) + CString::Format( DEFAULTINTFORMAT, v );
}

CString &CString::operator+=( const unsigned long v )
{
  Append( CString::Format( DEFAULTINTFORMAT, v ) );
  return *this;
}

const CString operator+( const unsigned long v, const CString &str )
{
  return CString::Format( DEFAULTINTFORMAT, v ) + str;
}

//float
CString CString::operator+( const float v ) const
{
  return ( *this ) + CString::Format( DEFAULTFLOATFORMAT, v );
}

CString &CString::operator+=( const float v )
{
  Append( CString::Format( DEFAULTFLOATFORMAT, v ) );
  return *this;
}

const CString operator+( const float v, const CString &str )
{
  return CString::Format( DEFAULTFLOATFORMAT, v ) + str;
}

//////////////////////////////////////////////////////////////////////////
//comparison functions

TBOOL CString::operator==( const CString &v ) const
{
  if ( Length() != v.Length() )
    return false;
  if ( Length() == 0 && v.Length() == 0 )
    return true;
  if ( !String && !v.String )
    return true;
  if ( !String || !v.String ) return false;
#ifdef HASHED_STRINGS
  if ( Hash != v.Hash )
    return false;
#endif
  return _tcscmp( String, v.String ) == 0;
}

TBOOL CString::operator==( const TS8 *v ) const
{
  if ( !String ) return false;
#ifndef UNICODE
  return _tcscmp( String, v ) == 0;
#else
#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  TBOOL result = *this == CString( v );
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
  return result;
#endif
}

TBOOL CString::operator==( const wchar_t *v ) const
{
  if ( !String ) return false;
#ifdef UNICODE
  return _tcscmp( String, v ) == 0;
#else
#ifdef MEMORY_TRACKING
  memTracker.Pause();
#endif
  TBOOL result = *this == CString( v );
#ifdef MEMORY_TRACKING
  memTracker.Resume();
#endif
  return result;
#endif
}

TBOOL CString::operator!=( const CString &v ) const
{
  if ( Length() != v.Length() )
    return true;
  if ( Length() == 0 && v.Length() == 0 )
    return false;
  if ( !String && !v.String )
    return false;
  if ( !String ) return true;
#ifdef HASHED_STRINGS
  if ( Hash != v.Hash )
    return true;
#endif
  return _tcscmp( String, v.String ) != 0;
}

TBOOL CString::operator!=( const TS8 *v ) const
{
  return *this != CString( v );
}

TBOOL CString::operator!=( const wchar_t *v ) const
{
  return *this != CString( v );
}

TBOOL CString::operator<( const CString &v ) const
{
  return _tcscmp( String, v.String ) < 0;
}

TBOOL CString::operator<( const TS8 *v ) const
{
  return *this < CString( v );
}

TBOOL CString::operator<( const wchar_t *v ) const
{
  return *this < CString( v );
}

TBOOL CString::operator>( const CString &v ) const
{
  return _tcscmp( String, v.String ) > 0;
}

TBOOL CString::operator>( const TS8 *v ) const
{
  return *this > CString( v );
}

TBOOL CString::operator>( const wchar_t *v ) const
{
  return *this > CString( v );
}

TCHAR &CString::operator[]( const int32_t idx ) const
{
  return String[ idx ];
}

//////////////////////////////////////////////////////////////////////////
//

uint32_t CString::Length() const
{
  return LengthCached;
}

//////////////////////////////////////////////////////////////////////////
// manipulation functions

void CString::Insert( int32_t pos, const TCHAR Input )
{
  TCHAR *start = new TCHAR[ pos + 2 ];
  memcpy( start, String, sizeof( TCHAR )*pos );
  start[ pos ] = Input;
  start[ pos + 1 ] = 0;
  *this = CString( start ) + ( String + pos );
  delete[] start;
}

void CString::Insert( int32_t pos, const TCHAR *Input )
{
  TCHAR *start = new TCHAR[ pos + 1 ];
  memcpy( start, String, sizeof( TCHAR )*pos );
  start[ pos ] = 0;
  *this = CString( start ) + CString( Input ) + CString( String + pos );
  delete[] start;
}

void CString::DeleteChar( int32_t pos )
{
  if ( pos < 0 || pos >= (int32_t)Length() ) return;
  if ( pos == 0 )
  {
    *this = CString( String + 1 );
    return;
  }
  TCHAR *txt = new TCHAR[ Length() ];
  memcpy( txt, String, sizeof( TCHAR )*pos );
  memcpy( txt + pos, String + pos + 1, sizeof( TCHAR )*( Length() - pos - 1 ) );
  txt[ Length() - 1 ] = 0;
  *this = CString( txt );
  delete[] txt;
}

void CString::DeleteRegion( int32_t pos, int32_t size )
{
  if ( !size ) return;
  if ( pos < 0 || pos >= (int32_t)Length() ) return;
  if ( size >= (int32_t)Length() )
  {
    *this = _T( "" );
    return;
  }
  if ( pos + size >= (int32_t)Length() ) //cut the end
  {
    String[ pos ] = 0;
    *this = CString( String );
    return;
  }
  if ( pos == 0 ) //cut the beginning
  {
    *this = CString( String + size );
    return;
  }
  TCHAR *txt = new TCHAR[ Length() - size + 1 ];
  memcpy( txt, String, sizeof( TCHAR )*pos );
  memcpy( txt + pos, String + pos + size, sizeof( TCHAR )*( Length() - pos - size ) );
  txt[ Length() - size ] = 0;
  *this = CString( txt );
  delete[] txt;
}

void CString::ToUnixNewline()
{
  uint32_t cnt = 0;
  for ( int32_t x = 0; x < (int32_t)Length(); x++ )
    if ( String[ x ] == '\r' ) cnt++;
  if ( !cnt ) return;

  uint32_t pos = 0;
  TCHAR *str = new TCHAR[ Length() - cnt + 2 ];
  for ( int32_t x = 0; x < (int32_t)Length(); x++ )
    if ( String[ x ] != '\r' ) str[ pos++ ] = String[ x ];

  str[ pos ] = 0;

  *this = CString( str );
  delete[] str;
}

void CString::ToWindowsNewline()
{
  ToUnixNewline(); //make sure there aren't any '\r's in the string
  int32_t cnt = 0;
  for ( int32_t x = 0; x < (int32_t)Length(); x++ )
    if ( String[ x ] == '\n' ) cnt++;
  if ( !cnt ) return;

  int32_t pos = 0;
  TCHAR *str = new TCHAR[ Length() + cnt + 2 ];
  for ( int32_t x = 0; x < (int32_t)Length(); x++ )
  {
    if ( String[ x ] == '\n' ) str[ pos++ ] = '\r';
    str[ pos++ ] = String[ x ];
  }

  str[ pos ] = 0;

  *this = CString( str );
  delete[] str;
}

void CString::ToLower()
{
  for ( uint32_t x = 0; x < Length(); x++ )
    String[ x ] = _totlower( String[ x ] );
  StringChanged();
}

void CString::ToUpper()
{
  for ( uint32_t x = 0; x < Length(); x++ )
    String[ x ] = _totupper( String[ x ] );
  StringChanged();
}


//////////////////////////////////////////////////////////////////////////
//hash functions

#ifdef HASHED_STRINGS
void CString::CalculateHash()
{
  Hash = 5381;

  if ( !String )
    return;

  int32_t c;
  TCHAR *str = String;

  //djb2 hash
  while ( c = *str++ )
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c
}

uint32_t CString::GetHash() const
{
  return Hash;
}
#endif

uint32_t DictionaryHash( const CString &i )
{
#ifndef HASHED_STRINGS
  int32_t c;
  TCHAR *str = i.GetPointer();

  //djb2 hash
  uint32_t Hash = 5381;
  while ( c = *str++ )
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c
  return Hash;
#else
  return i.GetHash();
#endif
}

//CString CString::Format(const wchar_t *format, ...)
//{
//	if (!format) return CString();
//
//	CString s;
//
//	va_list argList;
//	va_start(argList, format);
//	FormatVA(format, argList, s);
//	va_end(argList);
//
//	return s;
//}

//void CString::FormatVA(const wchar_t *format, va_list argList, CString &Result)
//{
//	if (!format)
//	{
//		Result = CString();
//		return;
//	}
//
//	va_list argListBegin = argList;
//	int32_t n = _vscwprintf(format, argList);
//
//	wchar_t* mText = new wchar_t[n + 1];
//	ZeroMemory(mText, (n + 1)*sizeof(wchar_t));
//	_vsnwprintf_s(mText, n, n, format, argListBegin);
//
//	Result = CString(mText);
//	delete[] mText;
//}
//
CString CString::Format( const TCHAR *format, ... )
{
  if ( !format ) return CString();

  CString s;

  va_list argList;
  va_start( argList, format );
  FormatVA( format, argList, s );
  va_end( argList );

  return s;
}

void CString::FormatVA( const TCHAR *format, va_list argList, CString &Result )
{
  if ( !format )
  {
    Result = CString();
    return;
  }

  int32_t n = _vsctprintf( format, argList );

  TCHAR* mText = new TCHAR[ n + 1 ];
  ZeroMemory( mText, ( n + 1 ) * sizeof( TCHAR ) );
  _vsntprintf_s( mText, n + 1, n, format, argList );

  Result = CString( mText );
  delete[] mText;
}

int32_t CString::Scan( const TCHAR *format, ... )
{
  if ( !format ) return 0;

  int32_t s;

  va_list argList;
  va_start( argList, format );
  ScanVA( format, argList, s );
  va_end( argList );

  return s;
}

int32_t CString::Scan( const CString format, ... )
{
  int32_t s;

  va_list argList;
  va_start( argList, format );
  ScanVA( format.GetPointer(), argList, s );
  va_end( argList );

  return s;
}

int32_t CString::Find( const CString &v, uint32_t nStart ) const
{
  if ( nStart > Length() ) return -1;
  TCHAR * sz = _tcsstr( String + nStart, v.GetPointer() );
  if ( !sz ) return -1;
  return (int32_t)( sz - String );
}

int32_t CString::Find( const TS8 *v, uint32_t nStart ) const
{
  return Find( CString( v ), nStart );
}
int32_t CString::Find( const wchar_t *v, uint32_t nStart ) const
{
  return Find( CString( v ), nStart );
}

uint32_t CString::GetSubstringCount( CString &v ) const
{
  uint32_t nCount = 0;
  uint32_t nStart = 0;
  while ( ( nStart = Find( v, nStart ) + 1 ) != 0 )
    nCount++;
  return nCount;
}

uint32_t CString::GetSubstringCount( const TS8 *v ) const
{
  uint32_t nCount = 0;
  uint32_t nStart = 0;
  while ( ( nStart = Find( v, nStart ) + 1 ) != 0 )
    nCount++;
  return nCount;
}

uint32_t CString::GetSubstringCount( const wchar_t *v ) const
{
  uint32_t nCount = 0;
  uint32_t nStart = 0;
  while ( ( nStart = Find( v, nStart ) + 1 ) != 0 )
    nCount++;
  return nCount;
}


CString CStringArray::Implode( CString &sDelimiter )
{
  CString sOut;
  for ( int i = 0; i < NumItems(); i++ )
  {
    if ( i > 0 )
      sOut += sDelimiter;
    sOut += Array[ i ];
  }
  return sOut;
}

CString CStringArray::Implode( TCHAR *sDelimiter )
{
  return Implode( CString( sDelimiter ) );
}

CStringArray CString::Explode( CString &sDelimiter ) const
{
  CStringArray aOut;
  int nPrevious = 0;
  int nNext = 0;
  if ( !String ) return aOut;
  while ( ( nNext = Find( sDelimiter, nPrevious ) ) > -1 )
  {
    aOut.Add( Substring( nPrevious, nNext - nPrevious ) );
    nPrevious = nNext + sDelimiter.Length();
  }
  if ( Length() - nPrevious >= sDelimiter.Length() )
  {
    aOut.Add( Substring( nPrevious ) );
  }
  return aOut;
}

CStringArray CString::Explode( TCHAR *sDelimiter ) const
{
  return Explode( CString( sDelimiter ) );
}

CStringArray CString::ExplodeByWhiteSpace() const
{
  CStringArray aOut;
  int nPrevious = 0;
  int nNext = 0;

  unsigned int x = 0;

  while ( x < Length() )
  {
    while ( x < Length() && _istspace( String[ x ] ) ) x++;
    nPrevious = x;
    while ( x < Length() && ( !_istspace( String[ x ] ) ) ) x++;
    aOut.Add( Substring( nPrevious, x - nPrevious ) );
  }

  return aOut;
}

CStringArrayMarkers CString::GetExplodeMarkers( CString &sDelimiter ) const
{
  CStringArrayMarkers aOut;
  int nPrevious = 0;
  int nNext = 0;
  if ( !String ) return aOut;
  while ( ( nNext = Find( sDelimiter, nPrevious ) ) > -1 )
  {
    aOut.Add( CStringMarker( nPrevious, nNext - nPrevious ) );
    nPrevious = nNext + sDelimiter.Length();
  }
  if ( Length() - nPrevious >= sDelimiter.Length() )
  {
    aOut.Add( CStringMarker( nPrevious, Length() - nPrevious ) );
  }
  return aOut;
}

CStringArrayMarkers CString::GetExplodeMarkers( TCHAR *sDelimiter ) const
{
  return GetExplodeMarkers( CString( sDelimiter ) );
}

CStringArrayMarkers CString::GetExplodeMarkersByWhiteSpace() const
{
  CStringArrayMarkers aOut;
  int nPrevious = 0;
  int nNext = 0;

  unsigned int x = 0;

  while ( x < Length() )
  {
    while ( x < Length() && _istspace( String[ x ] ) ) x++;
    nPrevious = x;
    while ( x < Length() && ( !_istspace( String[ x ] ) ) ) x++;
    aOut.Add( CStringMarker( nPrevious, x - nPrevious ) );
  }

  return aOut;
}


CString CString::Substring( int32_t nStart ) const
{
  if ( nStart < 0 )
    return Substring( Length() + nStart, -nStart );
  else
    return Substring( nStart, Length() - nStart );
}

CString CString::Substring( int32_t nStart, int32_t nLength ) const
{
  if ( nStart < 0 )
    nStart = Length() + nStart;
  if ( nLength < 0 )
    nLength = ( Length() - nStart ) + nLength;
  if ( nLength < 0 || nStart < 0 )
    return CString( _T( "" ) );
  if ( (uint32_t)nStart > Length() )
    return CString( _T( "" ) );
  if ( (uint32_t)nStart + (uint32_t)nLength > Length() )
    return CString( _T( "" ) );
  return CString( String + nStart, nLength );
}

int32_t CString::CompareNoCase( const CString &a, const CString &b )
{
  return _tcsicmp( a.GetPointer(), b.GetPointer() );
}

CString CString::Trimmed() const
{

  int nStart = 0;
  int nEnd = 0;
  for ( unsigned int i = 0; i < Length(); i++, nStart++ ) if ( !_istspace( String[ i ] ) ) break;
  for ( int i = Length() - 1; i >= 0; i--, nEnd++ ) if ( !_istspace( String[ i ] ) ) break;
  if ( nEnd )
    return Substring( nStart, -nEnd );
  else
    return Substring( nStart );
}

//////////////////////////////////////////////////////////////////////////
// helper stuff

TCHAR *base64_chars = _T( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" );

TBOOL is_base64( TCHAR c )
{
  return ( _istalnum( c ) || ( c == _T( '+' ) ) || ( c == _T( '/' ) ) );
}

TS8 find_base64( TCHAR t )
{
  for ( int32_t x = 0; x < (int32_t)_tcslen( base64_chars ); x++ )
    if ( base64_chars[ x ] == t ) return x;
  return -1;
}

CString CString::EncodeToBase64( uint8_t *Data, int32_t Length )
{
  int32_t i = 0;
  int32_t j = 0;
  uint8_t char_array_3[ 3 ];
  uint8_t char_array_4[ 4 ];

  TCHAR * szret = new TCHAR[ Length * 5 ];
  memset( szret, 0, Length * 5 * sizeof( TCHAR ) );
  TCHAR * p = szret;

  uint8_t * szText = (uint8_t *)Data;
  int32_t nLength = Length;
  while ( nLength-- ) {
    char_array_3[ i++ ] = *( szText++ );
    if ( i == 3 ) {
      char_array_4[ 0 ] = ( char_array_3[ 0 ] & 0xfc ) >> 2;
      char_array_4[ 1 ] = ( ( char_array_3[ 0 ] & 0x03 ) << 4 ) + ( ( char_array_3[ 1 ] & 0xf0 ) >> 4 );
      char_array_4[ 2 ] = ( ( char_array_3[ 1 ] & 0x0f ) << 2 ) + ( ( char_array_3[ 2 ] & 0xc0 ) >> 6 );
      char_array_4[ 3 ] = char_array_3[ 2 ] & 0x3f;

      for ( i = 0; i < 4; i++ ) {
        *( p++ ) = base64_chars[ char_array_4[ i ] ];
      }
      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 3; j++ )
      char_array_3[ j ] = 0;

    char_array_4[ 0 ] = ( char_array_3[ 0 ] & 0xfc ) >> 2;
    char_array_4[ 1 ] = ( ( char_array_3[ 0 ] & 0x03 ) << 4 ) + ( ( char_array_3[ 1 ] & 0xf0 ) >> 4 );
    char_array_4[ 2 ] = ( ( char_array_3[ 1 ] & 0x0f ) << 2 ) + ( ( char_array_3[ 2 ] & 0xc0 ) >> 6 );
    char_array_4[ 3 ] = char_array_3[ 2 ] & 0x3f;

    for ( j = 0; ( j < i + 1 ); j++ ) {
      *( p++ ) = base64_chars[ char_array_4[ j ] ];
    }


    while ( ( i++ < 3 ) )
      *( p++ ) = '=';
  }

  CString ret = szret;
  delete[] szret;

  //#ifdef _DEBUG
  //
  //	uint8_t *Data2 = NULL;
  //	int32_t Length2 = 0;
  //	ret.DecodeBase64(Data2, Length2);
  //	if (Length2 == Length)
  //		if (!memcmp(Data, Data2, Length))
  //		{
  //			SAFEDELETEA(Data2);
  //			return ret;
  //		}
  //
  //	SAFEDELETEA(Data2);
  //	LOG_ERR("[string] Base64 encoding was NOT successfully decoded!");
  //
  //	ret.DecodeBase64(Data2, Length2);
  //	SAFEDELETEA(Data2);
  //
  //#endif

  return ret;
}

void CString::DecodeBase64( uint8_t *&Data, int32_t &outsize )
{
  int in_len = Length();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[ 4 ], char_array_3[ 3 ];
  CStreamWriterMemory ret;

  while ( in_len-- && ( String[ in_ ] != _T( '=' ) ) && is_base64( String[ in_ ] ) )
  {
    char_array_4[ i++ ] = (uint8_t)String[ in_ ]; in_++;
    if ( i == 4 )
    {
      for ( i = 0; i < 4; i++ )
        char_array_4[ i ] = find_base64( char_array_4[ i ] );

      char_array_3[ 0 ] = ( char_array_4[ 0 ] << 2 ) + ( ( char_array_4[ 1 ] & 0x30 ) >> 4 );
      char_array_3[ 1 ] = ( ( char_array_4[ 1 ] & 0xf ) << 4 ) + ( ( char_array_4[ 2 ] & 0x3c ) >> 2 );
      char_array_3[ 2 ] = ( ( char_array_4[ 2 ] & 0x3 ) << 6 ) + char_array_4[ 3 ];

      for ( i = 0; ( i < 3 ); i++ )
        ret.WriteByte( char_array_3[ i ] );
      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 4; j++ )
      char_array_4[ j ] = 0;

    for ( j = 0; j < 4; j++ )
      char_array_4[ j ] = find_base64( char_array_4[ j ] );

    char_array_3[ 0 ] = ( char_array_4[ 0 ] << 2 ) + ( ( char_array_4[ 1 ] & 0x30 ) >> 4 );
    char_array_3[ 1 ] = ( ( char_array_4[ 1 ] & 0xf ) << 4 ) + ( ( char_array_4[ 2 ] & 0x3c ) >> 2 );
    char_array_3[ 2 ] = ( ( char_array_4[ 2 ] & 0x3 ) << 6 ) + char_array_4[ 3 ];

    for ( j = 0; ( j < i - 1 ); j++ )
      ret.WriteByte( char_array_3[ j ] );
  }

  Data = NULL;
  outsize = 0;

  if ( ret.GetLength() )
  {
    Data = new uint8_t[ ret.GetLength() ];
    memcpy( Data, ret.GetData(), ret.GetLength() );
    outsize = ret.GetLength();
  }
}

#include <stdio.h>
#include <wchar.h>

//static int vsscanf_proxy(const char *buffer, const char *format, va_list argPtr)
//{
//#ifndef _WIN64
//	size_t count = 0;
//	const char *p = format;
//	while (1)
//	{
//		char c = *(p++);
//		if (c == 0) break;
//		if (c == '%' && (p[0] != '*' && p[0] != '%')) ++count;
//	}
//
//	if (count <= 0) return 0;
//
//	int result;
//	void *savedESP;
//	_asm
//	{
//		mov savedESP, esp;
//
//	loopstart:
//		cmp count, 0;
//		jz loopend;
//		dec count;
//
//		mov eax, DWORD ptr[count];
//		mov ecx, DWORD ptr[argPtr];
//		mov edx, DWORD ptr[ecx + eax * 4]; //TODO: make this architecture independent for 64bit builds
//		push edx;
//
//		jmp loopstart;
//	loopend:
//
//
//		mov eax, DWORD ptr format;
//		push eax;
//		mov eax, DWORD ptr buffer;
//		push eax;
//
//		call DWORD PTR[sscanf_s];
//		mov esp, savedESP;
//		mov result, eax;
//	}
//	return result;
//#else
//  return vsscanf_s( buffer, format, argPtr );
//#endif
//}
//
//static int vsscanfw_proxy(const wchar_t *buffer, const wchar_t *format, va_list argPtr)
//{
//#ifndef _WIN64
//  size_t count = 0;
//	const wchar_t *p = format;
//	while (1)
//	{
//		wchar_t c = *(p++);
//		if (c == 0) break;
//		if (c == _T('%') && (p[0] != _T('*') && p[0] != _T('%'))) ++count;
//	}
//
//	if (count <= 0) return 0;
//
//	int result;
//	void *savedESP;
//	_asm
//	{
//		mov savedESP, esp;
//
//	loopstart:
//		cmp count, 0;
//		jz loopend;
//		dec count;
//
//		mov eax, DWORD ptr[count];
//		mov ecx, DWORD ptr[argPtr];
//		mov edx, DWORD ptr[ecx + eax * 4]; //TODO: make this architecture independent for 64bit builds
//		push edx;
//
//		jmp loopstart;
//	loopend:
//
//
//		mov eax, DWORD ptr format;
//		push eax;
//		mov eax, DWORD ptr buffer;
//		push eax;
//
//		call DWORD PTR[swscanf_s];
//		mov esp, savedESP;
//		mov result, eax;
//	}
//	return result;
//#else
//  return 0;
//#endif
//}

void CString::ScanVA( const TCHAR *format, va_list argList, int32_t &Result )
{
  if ( !format )
  {
    Result = 0;
    return;
  }

#ifndef UNICODE
  Result = vsscanf( GetPointer(), format, argList );
#else
  Result = vswscanf( GetPointer(), format, argList );
#endif
}

int32_t CString::Strcmp( const TCHAR *str, const TCHAR *str2 )
{
  return _tcscmp( str, str2 );
}

int32_t CString::Strncmp( const TCHAR *str, const TCHAR *str2, const int32_t len )
{
  return _tcsncmp( str, str2, len );
}

float CString::Atof( const TCHAR *str )
{
  return (float)_tstof( str );
}

int32_t CString::Atoi( const TCHAR *str )
{
  return _tstoi( str );
}

void CString::DecodeUtf8( const TCHAR* Input, UTF8CHARCALLBACK callback )
{
  const TCHAR* Text = Input;

  while ( *Text )
  {
    int Char = *Text;
    if ( ( Char & 0x80 ) ) // decode utf-8
    {
      if ( ( Char & 0xe0 ) == 0xc0 )
      {
        Char = Char & ( ( 1 << 5 ) - 1 );
        for ( int z = 0; z < 1; z++ )
        {
          if ( *( Text + 1 ) )
          {
            Char = ( Char << 6 ) + ( ( *( Text + 1 ) ) & 0x3f );
            Text++;
          }
        }
      }
      else
        if ( ( Char & 0xf0 ) == 0xe0 )
        {
          Char = Char & ( ( 1 << 4 ) - 1 );
          for ( int z = 0; z < 2; z++ )
          {
            if ( *( Text + 1 ) )
            {
              Char = ( Char << 6 ) + ( ( *( Text + 1 ) ) & 0x3f );
              Text++;
            }
          }
        }
        else
          if ( ( Char & 0xf8 ) == 0xf0 )
          {
            Char = Char & ( ( 1 << 3 ) - 1 );
            for ( int z = 0; z < 3; z++ )
            {
              if ( *( Text + 1 ) )
              {
                Char = ( Char << 6 ) + ( ( *( Text + 1 ) ) & 0x3f );
                Text++;
              }
            }
          }
          else
            if ( ( Char & 0xfc ) == 0xf8 )
            {
              Char = Char & ( ( 1 << 2 ) - 1 );
              for ( int z = 0; z < 4; z++ )
              {
                if ( *( Text + 1 ) )
                {
                  Char = ( Char << 6 ) + ( ( *( Text + 1 ) ) & 0x3f );
                  Text++;
                }
              }
            }
            else
              if ( ( Char & 0xfe ) == 0xfc )
              {
                Char = Char & ( ( 1 << 1 ) - 1 );
                for ( int z = 0; z < 5; z++ )
                {
                  if ( *( Text + 1 ) )
                  {
                    Char = ( Char << 6 ) + ( ( *( Text + 1 ) ) & 0x3f );
                    Text++;
                  }
                }
              }
    }

    if ( !callback( Char ) )
      return;
    Text++;
  }
}

void CString::DecodeUtf8( UTF8CHARCALLBACK callback )
{
  DecodeUtf8( String, callback );
}

void CString::RemoveNewLines()
{
  TCHAR *txt = new TCHAR[ Length() + 1 ];
  int32_t pos = 0;
  for ( uint32_t x = 0; x < Length(); x++ )
  {
    if ( String[ x ] == '\n' || String[ x ] == '\r' ) continue;
    txt[ pos++ ] = String[ x ];
  }
  txt[ pos ] = 0;
  *this = CString( txt );
  delete[] txt;
}

#define GECCO_ESCAPECHARS   _T("#,:\\=[|]$")

void CString::URLEncode()
{
  if ( !String || !Length() ) return;

  int nLen = Length();
  int nELen = 0;

  const TCHAR hex[] = _T( "0123456789abcdef" );

  TCHAR escchars[] = GECCO_ESCAPECHARS;
  TCHAR * escapestring = new TCHAR[ nLen * 3 + 1 ];
  ZeroMemory( escapestring, nLen * 3 + 1 );

  int pos = 0;
  for ( int j = 0; j < nLen; j++ )
  {
    TCHAR c = String[ j ];
    if ( !isalnum( c ) )
    {
      escapestring[ pos++ ] = '%';
      escapestring[ pos++ ] = hex[ c >> 4 ];
      escapestring[ pos++ ] = hex[ c & 15 ];
    }
    else
    {
      escapestring[ pos++ ] = c;
    }
  }

  *this = CString( escapestring );
  delete[] escapestring;
}
