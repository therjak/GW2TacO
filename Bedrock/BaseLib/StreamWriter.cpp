#include "BaseLib.h"

CStreamWriter::CStreamWriter()
{
  writerBitOffset = 0;
  writerCurrentChar = 0;
}

CStreamWriter::~CStreamWriter()
{

}

bool CStreamWriter::Write( void* lpBuf, uint32_t nCount )
{
  if ( writerBitOffset == 0 ) //non bitstream mode
    return WriteStream( lpBuf, nCount ) == nCount;

  //bitstream mode
  for ( uint32_t x = 0; x < nCount; x++ )
    BASEASSERT( WriteBits( ( (uint8_t *)lpBuf )[ x ], 8 ) == 1 );

  return true;
}

bool CStreamWriter::WriteByte( uint8_t data )
{
  return Write( &data, 1 );
}

bool CStreamWriter::WriteDWord( uint32_t data )
{
  return Write( &data, 4 );
}

bool CStreamWriter::WriteBits( uint32_t data, uint32_t BitCount )
{
  BASEASSERT( BitCount <= 64 );

  while ( BitCount > 0 )
  {
    uint32_t count = min( 8 - writerBitOffset, BitCount );
    uint32_t mask = ( 1 << count ) - 1;

    writerCurrentChar = (uint8_t)( ( writerCurrentChar & ( ~( mask << writerBitOffset ) ) ) | ( ( ( data >> ( BitCount - count ) ) & mask ) << writerBitOffset ) );

    BitCount -= count;
    writerBitOffset += count;

    if ( writerBitOffset >= 8 )
    {
      BASEASSERT( WriteStream( &writerCurrentChar, 1 ) );
      writerCurrentChar = 0;
    }

    writerBitOffset &= 7;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////
// streamwritermemory

CStreamWriterMemory::CStreamWriterMemory() : CStreamWriter()
{
  Data = new uint8_t[ 1024 ];
  BufferSize = 1024;
  DataLength = 0;
}

CStreamWriterMemory::~CStreamWriterMemory()
{
  SAFEDELETEA( Data );
  BufferSize = 0;
}

int32_t CStreamWriterMemory::WriteStream( void* lpBuf, uint32_t nCount )
{
  if ( DataLength + nCount > BufferSize )
  {
    BufferSize = (uint32_t)( ( BufferSize + nCount )*1.2f );
    uint8_t *temp = Data;
    Data = new uint8_t[ BufferSize ];
    memcpy( Data, temp, DataLength );
    SAFEDELETEA( temp );
  }

  memcpy( Data + DataLength, lpBuf, nCount );
  DataLength += nCount;

  return nCount;
}

uint8_t *CStreamWriterMemory::GetData()
{
  return Data;
}

uint32_t CStreamWriterMemory::GetLength()
{
  return DataLength;
}

void CStreamWriterMemory::Flush()
{
  SAFEDELETEA( Data );
  BufferSize = 0;
  DataLength = 0;

  Data = new uint8_t[ 1024 ];
  BufferSize = 1024;
  DataLength = 0;
}

//////////////////////////////////////////////////////////////////////////
// streamwriterfile

CStreamWriterFile::CStreamWriterFile() : CStreamWriter()
{
  File = NULL;
}

CStreamWriterFile::~CStreamWriterFile()
{
  if ( File )
    CloseHandle( File );
}

int32_t CStreamWriterFile::WriteStream( void *lpBuf, uint32_t nCount )
{
  DWORD nWritten = 0;
  BOOL b = WriteFile( File, lpBuf, nCount, &nWritten, NULL );
  if ( !b ) return 0;
  return nWritten;
}

int32_t CStreamWriterFile::Open( TCHAR *Filename )
{
  if ( File )
    CloseHandle( File ); //close previous handle

  File = CreateFile( Filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL );
  CloseHandle( File );

  File = CreateFile( Filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS | TRUNCATE_EXISTING, NULL, NULL );
  if ( File == INVALID_HANDLE_VALUE )
  {
    LPTSTR pMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                   (LPTSTR)&pMsgBuf, 0, NULL );

    LOG_ERR( "[writer] Error opening file '%s': %s", Filename, pMsgBuf );
    LocalFree( pMsgBuf );
    return 0;
  }
  return 1;
}

void CStreamWriterFile::Flush()
{
  SetFilePointer( File, 0, NULL, FILE_BEGIN );
  SetEndOfFile( File );
}
