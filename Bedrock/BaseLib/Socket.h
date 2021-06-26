#pragma once

class CSocket : public CStreamReader, public CStreamWriter
{
protected:
  int64_t LastActivity;
  SOCKET Socket;

  virtual TS32 ReadStream( void *lpBuf, TU32 nCount );
  virtual TS32 WriteStream( void* lpBuf, TU32 nCount );

public:

  CSocket();
  CSocket( SOCKET s );
  virtual ~CSocket();

  //////////////////////////////////////////////////////////////////////////
  //socket functions

  virtual TS32 Connect( const CString &Server, const TU32 Port );
  virtual TS32 Listen( const TU32 Port, const TBOOL ReuseAddress = false );

  virtual TS32 Close();
  virtual TS32 AcceptConnection( CSocket &Socket );

  static TU32 Resolve( const TS8 *Address );

  //////////////////////////////////////////////////////////////////////////
  //streamreader functions

  virtual int64_t GetLength() const; //returns the currently available bytes in the socket
  virtual int64_t GetOffset() const; //is always 0

  virtual void SeekFromStart( TU64 lOff ); //these do nothing
  virtual void SeekRelative( int64_t lOff ); //these do nothing

  TS32 ReadFull( void *lpBuf, TU32 nCount );
  virtual CString ReadLine() override;

  TBOOL Peek( void *lpBuf, TU32 nCount );

  TBOOL IsConnected();

  const TBOOL operator==( const CSocket &b );

  TS32 TimeSinceLastActivity();

  //////////////////////////////////////////////////////////////////////////
  //streamwriter functions

};

TS32 InitWinsock();
void DeinitWinsock();

TU8 *FetchHTTP( CString host, CString path, TS32 &ContentSize );
