#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "BaseLib.h"
WSADATA wsaData;
#include <MMSystem.h>

CSocket::CSocket() : CStreamReader(), CStreamWriter()
{
  Socket = INVALID_SOCKET;
  LastActivity = GetTickCount64();
}

CSocket::CSocket( SOCKET s ) : CStreamReader(), CStreamWriter()
{
  Socket = s;
  LastActivity = GetTickCount64();
}

CSocket::~CSocket()
{
  //don't close the socket here as any copy of the class will invalidate the socket on destruction
}

int32_t CSocket::ReadStream( void *lpBuf, uint32_t nCount )
{
  if ( Socket == INVALID_SOCKET ) return 0;
  int32_t r = recv( Socket, (TS8*)lpBuf, nCount, NULL );
  if ( r != 0 ) LastActivity = GetTickCount64();
  return r;
}

int32_t CSocket::WriteStream( void* lpBuf, uint32_t nCount )
{
  int32_t res = send( Socket, (TS8*)lpBuf, nCount, NULL );
  if ( res == SOCKET_ERROR ) return 0;
  return res;
}

int32_t CSocket::Connect( const CString &Server, const uint32_t Port )
{
  TS8 Address[ 256 ];
  Server.WriteAsMultiByte( Address, 256 );

  int32_t addr = Resolve( Address );
  if ( addr == INADDR_NONE )
    return 0;

  Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( Socket == INVALID_SOCKET )
    return 0;

  SOCKADDR_IN serverInfo;
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.S_un.S_addr = addr;
  serverInfo.sin_port = htons( Port );

  if ( connect( Socket, (LPSOCKADDR)&serverInfo, sizeof( sockaddr ) ) == SOCKET_ERROR )
    return 0;

  LastActivity = GetTickCount64();
  return 1;
}

int32_t CSocket::Listen( const uint32_t Port, const TBOOL ReuseAddress )
{
  Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

  if ( Socket == INVALID_SOCKET )
    return 0;

  //TS8 yes=1;
  //if (ReuseAddress && setsockopt(Socket,SOL_SOCKET,SO_REUSEADDR,&yes,1)!=0)
  //{
  //	LOG(LOG_ERROR,CString::Format(_T("Winsock Error: %d"),WSAGetLastError()).GetPointer());
  //	return 0;
  //}

  SOCKADDR_IN addr;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;
  addr.sin_port = htons( Port );

  if ( bind( Socket, (SOCKADDR*)&addr, sizeof( SOCKADDR_IN ) ) == SOCKET_ERROR )
  {
    LOG( LOG_ERROR, _T( "[sock] Winsock Error: %d" ), WSAGetLastError() );
    return 0;
  }

  int32_t r = listen( Socket, 10 );
  return r != SOCKET_ERROR;
}

int32_t CSocket::Close()
{
  if ( Socket != INVALID_SOCKET )
  {
    shutdown( Socket, SD_SEND );
    closesocket( Socket );
  }
  Socket = INVALID_SOCKET;

  return 1;
}

int32_t CSocket::AcceptConnection( CSocket &NewConnection )
{
  fd_set Check;
  Check.fd_array[ 0 ] = Socket;
  Check.fd_count = 1;
  TIMEVAL t;
  t.tv_sec = 0;
  t.tv_usec = 1;

  //check if we have an incoming connection
  if ( select( 0, &Check, NULL, NULL, &t ) != 1 ) return 0;

  sockaddr_in addr;
  int32_t l = sizeof( sockaddr_in );

  SOCKET s = accept( Socket, ( struct sockaddr * )&addr, &l );
  if ( s == INVALID_SOCKET ) return 0;

  LOG_NFO( "[connection] New connection accepted from %s:%d", inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );

  NewConnection = CSocket( s );
  return 1;
}

uint32_t CSocket::Resolve( const TS8 *Address )
{
  uint32_t addr = inet_addr( Address );

  if ( addr == INADDR_NONE )
  {
    hostent * hostEntry = gethostbyname( Address );
    if ( hostEntry )
    {
      LPIN_ADDR pa = (LPIN_ADDR)( hostEntry->h_addr );
      addr = pa->S_un.S_addr;
    }
  }

  return addr;
}

//////////////////////////////////////////////////////////////////////////
//streamreader functions

int64_t CSocket::GetLength() const
{
  if ( Socket == INVALID_SOCKET )
    return 0;
  unsigned long count = 0;
  if ( ioctlsocket( Socket, FIONREAD, &count ) == SOCKET_ERROR ) return 0;
  return count;
}

int64_t CSocket::GetOffset() const
{
  return 0;
}

void CSocket::SeekFromStart( uint64_t lOff )
{
}

void CSocket::SeekRelative( int64_t lOff )
{
}

int32_t CSocket::ReadFull( void *data, uint32_t size )
{
  int32_t progress = 0;
  while ( 1 )
  {
    int32_t n = Read( ( (TS8*)data ) + progress, size - progress );
    if ( n == SOCKET_ERROR )
      return SOCKET_ERROR;
    progress += n;
    if ( progress == size ) break;
  }
  return size;
}

TBOOL CSocket::Peek( void *lpBuf, uint32_t nCount )
{
  if ( Socket == INVALID_SOCKET ) return 0;
  int32_t r = recv( Socket, (TS8*)lpBuf, nCount, MSG_PEEK );
  if ( r != 0 ) LastActivity = GetTickCount64();
  if ( r == SOCKET_ERROR )
    Socket = INVALID_SOCKET;
  return r != SOCKET_ERROR;
}

TBOOL CSocket::IsConnected()
{
  if ( Socket == INVALID_SOCKET )
    return false;

  int error = 0;
  int len = sizeof( error );
  int retval = getsockopt( Socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len );

  if ( retval || error )
  {
    Socket = INVALID_SOCKET;
    return false;
  }

  unsigned long count = 0;

  if ( ioctlsocket( Socket, FIONREAD, &count ) == SOCKET_ERROR )
  {
    return false;
  }
  return true;
}

int32_t CSocket::TimeSinceLastActivity()
{
  return (int32_t)( GetTickCount64() - LastActivity );
}

const TBOOL CSocket::operator==( const CSocket &b )
{
  return Socket == b.Socket;
}

CString CSocket::ReadLine()
{
  CString result;

  while ( 1 )
  {
    if ( !IsConnected() )
      return result;
    int32_t len = (int32_t)GetLength();
    if ( len )
    {
      TS8 *dat = new TS8[ len ];
      if ( Peek( dat, len ) )
      {
        for ( int32_t x = 0; x < len; x++ )
          if ( dat[ x ] == '\n' )
          {
            ReadFull( dat, x + 1 );
            result += CString( dat, x );
            SAFEDELETE( dat );
            return result;
          }
      }

      ReadFull( dat, len );
      result += CString( dat, len );
      SAFEDELETE( dat );
    }

    if ( !Peek( &len, 1 ) )
      return result;
  }

  //TS8 s[2];
  //s[0] = s[1] = 0;

  //do
  //{
  //	s[0] = ReadByte();

  //	if (s[0] != '\n' && s[0] != '\r')
  //		result += s;

  //} while (s[0] && s[0] != '\n');

  //return result;
}

int32_t InitWinsock()
{
  return WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) == 0;
}

void DeinitWinsock()
{
  WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
// helper functions

TU8 *FetchHTTP( CString host, CString path, int32_t &ContentSize )
{
  CSocket sock;

  TU8 *result = NULL;
  ContentSize = 0;

  if ( sock.Connect( host, 80 ) )
  {

    CString datastr = _T( "GET " ) + path + _T( " HTTP/1.1\r\nHost: " ) + host + _T( "\r\nConnection: close\r\n\r\n" );
    sock.Write( datastr.GetPointer(), datastr.Length() );

    int64_t time = globalTimer.GetTime();

    while ( !sock.GetLength() )
      if ( globalTimer.GetTime() - time > 1000 ) break;

    int32_t len = (int32_t)sock.GetLength();

    if ( len )
    {
      TU8 *headerdata = new TU8[ len + 1 ];
      memset( headerdata, 0, len + 1 );
      sock.ReadFull( headerdata, len );

      //check http header

      CString head = CString( (TS8*)headerdata );
      int32_t contentlength = -1;
      int32_t contentpos = head.Find( _T( "Content-Length: " ) );

      if ( contentpos >= 0 )
      {
        CString ctlen = CString( (TS8*)headerdata + contentpos );
        if ( ctlen.Scan( "Content-Length: %d", &contentlength ) != 1 )
        {
          SAFEDELETE( headerdata );
          return NULL;
        }
      }

      int32_t contentstart = head.Find( _T( "\r\n\r\n" ) );
      if ( contentstart < 0 )
      {
        SAFEDELETE( headerdata );
        return NULL;
      }

      contentstart += 4;

      int32_t byteslefttoread = contentlength - len;

      if ( byteslefttoread <= 0 && contentlength != -1 )
      {
        SAFEDELETE( headerdata );
        return NULL;
      }

      int32_t currentdatasize = len - contentstart;

      if ( currentdatasize > 0 )
      {
        result = new TU8[ currentdatasize ];
        memcpy( result, headerdata + contentstart, currentdatasize );
      }

      while ( byteslefttoread > 0 || contentlength == -1 )
      {
        int32_t dlen = (int32_t)sock.GetLength();
        if ( dlen )
        {
          TU8 *newdata = new TU8[ currentdatasize + dlen ];
          if ( result )
            memcpy( newdata, result, currentdatasize );
          int32_t numread = sock.ReadFull( newdata + currentdatasize, dlen );
          if ( numread == dlen )
          {
            currentdatasize += numread;
            byteslefttoread -= numread;
          }
          else
          {
            SAFEDELETE( newdata );
            SAFEDELETE( result );
            return NULL;
          }

          SAFEDELETE( result );
          result = newdata;
        }

        int32_t len;
        if ( !sock.Peek( &len, 1 ) )
          break;
      }

      ContentSize = currentdatasize;

    }

    sock.Close();
  }

  return result;
}

