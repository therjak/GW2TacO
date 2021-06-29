#pragma once

class CSocket : public CStreamReader, public CStreamWriter
{
protected:
  int64_t LastActivity;
  SOCKET Socket;

  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );
  static uint32_t Resolve(const TS8 *Address);

 public:

  CSocket();
  CSocket( SOCKET s );
  virtual ~CSocket();

  //////////////////////////////////////////////////////////////////////////
  //socket functions

  virtual int32_t Connect( const CString &Server, const uint32_t Port );
  int32_t ReadFull(void *lpBuf, uint32_t nCount);

  virtual int32_t Close();

  //////////////////////////////////////////////////////////////////////////
  //streamreader functions

  virtual int64_t GetLength() const; //returns the currently available bytes in the socket
  virtual int64_t GetOffset() const; //is always 0

  CString ReadLine();

  bool Peek( void *lpBuf, uint32_t nCount );

  bool IsConnected();

  const bool operator==( const CSocket &b );

  //////////////////////////////////////////////////////////////////////////
  //streamwriter functions

};

int32_t InitWinsock();
void DeinitWinsock();

uint8_t *FetchHTTP( CString host, CString path, int32_t &ContentSize );
