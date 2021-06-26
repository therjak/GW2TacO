#pragma once

class CSocket : public CStreamReader, public CStreamWriter
{
protected:
  int64_t LastActivity;
  SOCKET Socket;

  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );

public:

  CSocket();
  CSocket( SOCKET s );
  virtual ~CSocket();

  //////////////////////////////////////////////////////////////////////////
  //socket functions

  virtual int32_t Connect( const CString &Server, const uint32_t Port );
  virtual int32_t Listen( const uint32_t Port, const TBOOL ReuseAddress = false );

  virtual int32_t Close();
  virtual int32_t AcceptConnection( CSocket &Socket );

  static uint32_t Resolve( const TS8 *Address );

  //////////////////////////////////////////////////////////////////////////
  //streamreader functions

  virtual int64_t GetLength() const; //returns the currently available bytes in the socket
  virtual int64_t GetOffset() const; //is always 0

  virtual void SeekFromStart( uint64_t lOff ); //these do nothing
  virtual void SeekRelative( int64_t lOff ); //these do nothing

  int32_t ReadFull( void *lpBuf, uint32_t nCount );
  virtual CString ReadLine() override;

  TBOOL Peek( void *lpBuf, uint32_t nCount );

  TBOOL IsConnected();

  const TBOOL operator==( const CSocket &b );

  int32_t TimeSinceLastActivity();

  //////////////////////////////////////////////////////////////////////////
  //streamwriter functions

};

int32_t InitWinsock();
void DeinitWinsock();

TU8 *FetchHTTP( CString host, CString path, int32_t &ContentSize );
