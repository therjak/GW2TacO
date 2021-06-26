#pragma once

class CStreamWriter
{
  uint32_t writerBitOffset;
  TU8 writerCurrentChar;
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount ) = 0;

public:
  CStreamWriter();
  virtual ~CStreamWriter();

  TBOOL Write( void* lpBuf, uint32_t nCount );
  TBOOL WriteQWord( uint64_t data );
  TBOOL WriteDWord( uint32_t data );
  TBOOL WriteWord( TU16 data );
  TBOOL WriteByte( TU8 data );
  TBOOL WriteTF32( TF32 data );
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
  TU8 *Data;
  uint32_t BufferSize;
  uint32_t DataLength;

  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );

public:
  CStreamWriterMemory();
  virtual ~CStreamWriterMemory();

  TU8 *GetData();
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
