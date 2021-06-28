#pragma once

class CStreamReader
{
  uint32_t readerBitOffset;
  uint8_t readerLastChar;
  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount ) = NULL; //this reads nCount bytes from the stream

public:

  CStreamReader();
  virtual ~CStreamReader();

  //general purpose reading functions with bitstream support
  //these aren't virtual as they all fall back on ReadStream at one point and should not be overridden
  int32_t Read( void *lpBuf, uint32_t nCount );
  uint64_t ReadQWord();
  uint32_t ReadDWord();
  uint16_t ReadWord();
  uint8_t ReadByte();
  uint32_t ReadBits( uint32_t BitCount );
  TBOOL ReadBit();
  float ReadTF32();
  void ReadRemainingBits();

  CString ReadASCIIZ();
  virtual CString ReadLine();

  virtual int64_t GetLength() const = NULL;
  virtual int64_t GetOffset() const = NULL;
  virtual TBOOL eof();

  virtual void SeekFromStart( uint64_t lOff ) = NULL;
  virtual void SeekRelative( int64_t lOff ) = NULL;
};

class CStreamReaderMemory : public CStreamReader
{
  uint8_t *Data;
  uint64_t DataSize;
  uint64_t Offset;

  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );

public:

  CStreamReaderMemory();
  virtual ~CStreamReaderMemory();

  virtual int32_t Open( uint8_t *data, uint32_t size );
  virtual int32_t Open( const TCHAR *filename );

  virtual uint8_t *GetData() const;
  virtual int64_t GetLength() const;
  virtual int64_t GetOffset() const;

  virtual void SeekFromStart( uint64_t lOff );
  virtual void SeekRelative( int64_t lOff );
};

class CStreamReaderFile : public CStreamReader
{
  HANDLE File;

  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );

public:

  CStreamReaderFile();
  virtual ~CStreamReaderFile();

  int32_t Open( TCHAR *filename );

  virtual int64_t GetLength() const;
  virtual int64_t GetOffset() const;

  virtual void SeekFromStart( uint64_t lOff );
  virtual void SeekRelative( int64_t lOff );
};
