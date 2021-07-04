#include "BaseLib.h"

#include <string_view>

//////////////////////////////////////////////////////////////////////////
// base class implementation

CStreamReader::CStreamReader()
{
  readerBitOffset = 0;
  readerLastChar = 0;
}

CStreamReader::~CStreamReader()
{

}

int32_t CStreamReader::Read( void *lpBuf, uint32_t nCount )
{
  if ( readerBitOffset == 0 ) //non bitstream mode
    return ReadStream( lpBuf, nCount );

  //bitstream mode
  for ( uint32_t x = 0; x < nCount; x++ )
    ( (uint8_t*)lpBuf )[ x ] = ReadBits( 8 );

  return nCount;
}

uint32_t CStreamReader::ReadDWord()
{
  uint32_t i = 0;
  BASEASSERT( Read( &i, sizeof( uint32_t ) ) == sizeof( uint32_t ) );
  return i;
}

uint8_t CStreamReader::ReadByte()
{
  uint8_t i = 0;
  BASEASSERT( Read( &i, sizeof( uint8_t ) ) == sizeof( uint8_t ) );
  return i;
}

uint32_t CStreamReader::ReadBits( uint32_t BitCount )
{
  BASEASSERT( BitCount <= 64 );

  uint32_t result = 0;

  while ( BitCount > 0 )
  {
    //read next byte if needed
    if ( readerBitOffset == 0 )
      BASEASSERT( ReadStream( &readerLastChar, 1 ) == 1 );

    uint32_t count = min( 8 - readerBitOffset, BitCount );
    uint32_t mask = ( 1 << count ) - 1;

    uint8_t bits = readerLastChar >> readerBitOffset;
    result |= ( bits&mask ) << ( BitCount - count );
    BitCount -= count;

    readerBitOffset = ( readerBitOffset + count ) & 7;
  }

  return result;
}

CString CStreamReaderMemory::ReadLine() {
  CString result;

  TS8 s[ 2 ];
  s[ 0 ] = s[ 1 ] = 0;

  do
  {
    if ( GetOffset() == GetLength() )
      break;

    s[ 0 ] = ReadByte();

    if ( s[ 0 ] != '\n' && s[ 0 ] != '\r' )
      result += s;

  } while ( s[ 0 ] && s[ 0 ] != '\n' );

  return result;
}

bool CStreamReader::eof()
{
  return GetOffset() >= GetLength();
}

//////////////////////////////////////////////////////////////////////////
// streamreader memory

CStreamReaderMemory::CStreamReaderMemory() : CStreamReader()
{
  Data = NULL;
  DataSize = 0;
  Offset = 0;
}

CStreamReaderMemory::~CStreamReaderMemory()
{
  SAFEDELETEA( Data );
}

int32_t CStreamReaderMemory::ReadStream( void *lpBuf, uint32_t nCount )
{
  int64_t bytestoread = max( 0, min( nCount, DataSize - Offset ) );
  memcpy( lpBuf, Data + Offset, (size_t)bytestoread );
  Offset += bytestoread;
  return (int32_t)bytestoread;
}

int32_t CStreamReaderMemory::Open( uint8_t *data, uint32_t size )
{
  if ( !data || !size ) return 0;

  SAFEDELETEA( Data );
  DataSize = size;
  Offset = 0;

  Data = new uint8_t[ size ];
  memcpy( Data, data, size );

  return 1;
}

int32_t CStreamReaderMemory::Open(std::string_view Filename) {
  HANDLE hFile = CreateFile( Filename.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL );
  if ( hFile == INVALID_HANDLE_VALUE ) return 0;

  int32_t tDataSize = GetFileSize( hFile, NULL );

  uint8_t *tData = new uint8_t[ tDataSize ];
  DWORD nRead = 0;
  BOOL b = ReadFile( hFile, tData, tDataSize, &nRead, NULL );

  if ( b && nRead == tDataSize ) //all ok
  {
    SAFEDELETEA( Data );
    Data = tData;
    DataSize = tDataSize;
    Offset = 0;
  }
  else
  {
    SAFEDELETEA( tData ); //couldn't read complete file - fail.
  }

  CloseHandle( hFile );
  return nRead == tDataSize;
}

uint8_t *CStreamReaderMemory::GetData() const
{
  return Data;
}

int64_t CStreamReaderMemory::GetLength() const
{
  return DataSize;
}

int64_t CStreamReaderMemory::GetOffset() const
{
  return Offset;
}
