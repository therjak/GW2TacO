#pragma once

class CStreamReader
{
  TU32 readerBitOffset;
  TU8 readerLastChar;
  virtual int32_t ReadStream( void *lpBuf, TU32 nCount ) = NULL; //this reads nCount bytes from the stream

public:

  CStreamReader();
  virtual ~CStreamReader();

  //general purpose reading functions with bitstream support
  //these aren't virtual as they all fall back on ReadStream at one point and should not be overridden
  int32_t Read( void *lpBuf, TU32 nCount );
  TU64 ReadQWord();
  TU32 ReadDWord();
  TU16 ReadWord();
  TU8 ReadByte();
  TU32 ReadBits( TU32 BitCount );
  TBOOL ReadBit();
  TF32 ReadTF32();
  void ReadRemainingBits();

  CString ReadASCIIZ();
  virtual CString ReadLine();

  virtual int64_t GetLength() const = NULL;
  virtual int64_t GetOffset() const = NULL;
  virtual TBOOL eof();

  virtual void SeekFromStart( TU64 lOff ) = NULL;
  virtual void SeekRelative( int64_t lOff ) = NULL;
};

class CStreamReaderMemory : public CStreamReader
{
  TU8 *Data;
  TU64 DataSize;
  TU64 Offset;

  virtual int32_t ReadStream( void *lpBuf, TU32 nCount );

public:

  CStreamReaderMemory();
  virtual ~CStreamReaderMemory();

  virtual int32_t Open( TU8 *data, TU32 size );
  virtual int32_t Open( TCHAR *filename );

  virtual TU8 *GetData() const;
  virtual int64_t GetLength() const;
  virtual int64_t GetOffset() const;

  virtual void SeekFromStart( TU64 lOff );
  virtual void SeekRelative( int64_t lOff );
};

class CStreamReaderFile : public CStreamReader
{
  HANDLE File;

  virtual int32_t ReadStream( void *lpBuf, TU32 nCount );

public:

  CStreamReaderFile();
  virtual ~CStreamReaderFile();

  int32_t Open( TCHAR *filename );

  virtual int64_t GetLength() const;
  virtual int64_t GetOffset() const;

  virtual void SeekFromStart( TU64 lOff );
  virtual void SeekRelative( int64_t lOff );
};
