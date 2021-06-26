#pragma once

class CArchive;

#pragma pack(push,1)

struct ARCHIVEHEADER
{
  TS8 Signature[ 8 ];
  TU16 ChunkSize;
  int32_t EmptyChunk;
  int32_t FileIndexChunk;
  int32_t FileIndexSize;
};

class CArchiveEntry
{
  friend CArchive;

  int32_t StartChunk;
  int32_t FileSize;
  TU64 Hash;
  CString FileName;

  void Init( const CArchiveEntry &f );
public:

  CArchiveEntry();
  CArchiveEntry( const CArchiveEntry &f );
  virtual ~CArchiveEntry();
  virtual const CArchiveEntry &operator = ( const CArchiveEntry &f );

};

#pragma pack(pop)

class CStreamReaderArchive : public CStreamReaderMemory
{
public:
  CStreamReaderArchive();
  virtual ~CStreamReaderArchive();

  virtual int32_t Open( CArchive *BF, int32_t StartChunk, int32_t FileSize );
};

class CArchive
{
  friend CStreamReaderArchive;

  TBOOL ReadOnly;

  TU8 *TempChunk;
  FILE *Handle;

  TU64 FileSize;
  //TU64 CurrentPos;

  TU16 ChunkSize;
  int32_t EmptySequenceStartChunk;
  int32_t FileIndexChunk;
  int32_t FileIndexSize;

  CArray<CArchiveEntry> FileIndices;

  int32_t ReadChunk( int32_t ChunkID, TU8 *Data, int32_t BufferSize, int32_t &NextChunk );
  TBOOL ReadIndex( CStreamReaderArchive *idx );

  TBOOL SeekToChunk( int32_t Chunk );

  TBOOL UpdateHeader();
  TBOOL UpdateIndex( TBOOL IncludeFilenames = true );
  TBOOL ClearChunkSequence( int32_t StartChunk );
  TBOOL WriteFile( TU8 *Data, int32_t Size, int32_t &startchunk );
  TBOOL WriteChunk( int32_t &Chunk, TU8 *&Data, int32_t &DataSize );

  TU64 CalculateHash( const CString &Filename );

public:

  CArchive();
  virtual ~CArchive();

  TBOOL Open( const CString &FileName, TBOOL ReadOnly = true );
  TBOOL Create( const CString &FileName, TU16 ChunkSize = 1024 );

  TBOOL OpenFile( const CString &FileName, CStreamReaderArchive *&Reader );
  TBOOL AddFile( TU8 *Data, int32_t Size, const CString &FileName );

};
