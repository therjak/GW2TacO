#pragma once

class CStreamWriter
{
  uint32_t writerBitOffset;
  uint8_t writerCurrentChar;
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount ) = 0;
  bool WriteBits(uint32_t data, uint32_t bitcount);

 public:
  CStreamWriter();
  virtual ~CStreamWriter();

  bool Write( void* lpBuf, uint32_t nCount );
  bool WriteDWord( uint32_t data );
  bool WriteByte( uint8_t data );
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
  void Flush();
};
