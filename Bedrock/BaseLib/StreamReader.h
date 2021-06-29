#pragma once

class CStreamReader
{
  uint32_t readerBitOffset;
  uint8_t readerLastChar;
  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount ) = NULL; //this reads nCount bytes from the stream
  virtual int64_t GetOffset() const = NULL;
  uint32_t ReadBits( uint32_t BitCount );

 public:

  CStreamReader();
  virtual ~CStreamReader();

  //general purpose reading functions with bitstream support
  //these aren't virtual as they all fall back on ReadStream at one point and should not be overridden
  int32_t Read( void *lpBuf, uint32_t nCount );
  uint32_t ReadDWord();
  uint8_t ReadByte();


  virtual int64_t GetLength() const = NULL;
  bool eof();
};

class CStreamReaderMemory : public CStreamReader
{
  uint8_t *Data;
  uint64_t DataSize;
  uint64_t Offset;

  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );
  virtual int64_t GetOffset() const;

 public:

  CStreamReaderMemory();
  virtual ~CStreamReaderMemory();

  int32_t Open( uint8_t *data, uint32_t size );
  int32_t Open( const TCHAR *filename );
  CString ReadLine();

  uint8_t *GetData() const;
  virtual int64_t GetLength() const;
};
