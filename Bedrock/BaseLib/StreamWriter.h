#pragma once

class CStreamWriter
{
  uint32_t writerBitOffset;
  uint8_t writerCurrentChar;
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount ) = 0;

public:
  CStreamWriter();
  virtual ~CStreamWriter();

  TBOOL Write( void* lpBuf, uint32_t nCount );
  TBOOL WriteQWord( uint64_t data );
  TBOOL WriteDWord( uint32_t data );
  TBOOL WriteWord( uint16_t data );
  TBOOL WriteByte( uint8_t data );
  TBOOL WriteTF32( float data );
  TBOOL WriteBits( uint32_t data, uint32_t bitcount );
  TBOOL WriteBool( TBOOL data );
  TBOOL WriteRemainingBits();
  TBOOL WriteASCIIZ( CString &s );

  //TBOOL WriteFormat(const wchar_t *format, ...);
  TBOOL WriteFormat( const TCHAR *format, ... );
  //TBOOL WriteFormatZT(const wchar_t *format, ...);
  TBOOL WriteFormatZT( const TCHAR *format, ... );
};

class CStreamWriterMemory : public CStreamWriter
{
  uint8_t *Data;
  uint32_t BufferSize;
  uint32_t DataLength;

  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );

public:
  CStreamWriterMemory();
  virtual ~CStreamWriterMemory();

  uint8_t *GetData();
  uint32_t GetLength();

  void Flush();
};

class CStreamWriterFile : public CStreamWriter
{
  HANDLE File;

  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );

public:
  CStreamWriterFile();
  virtual ~CStreamWriterFile();

  int32_t Open( TCHAR *filename );
  TBOOL Flush();
};
