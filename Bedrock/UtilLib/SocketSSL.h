#pragma once
#include "../BaseLib/BaseLib.h"

class CSocketSSL : public CSocket
{
public:
  CSocketSSL( void );
  ~CSocketSSL( void );
  virtual int32_t ReadStream( void *lpBuf, TU32 nCount );
  virtual int32_t WriteStream( void* lpBuf, TU32 nCount );
  virtual int32_t Connect( const CString &Server, const TU32 Port );
  virtual int32_t Close();
  void LogSSLError();
private:
  void * ssl;
  void * ctx;
};

