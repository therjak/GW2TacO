#pragma once
#include "../BaseLib/BaseLib.h"

class CSocketSSL : public CSocket
{
public:
  CSocketSSL( void );
  ~CSocketSSL( void );
  virtual int32_t ReadStream( void *lpBuf, uint32_t nCount );
  virtual int32_t WriteStream( void* lpBuf, uint32_t nCount );
  virtual int32_t Connect( const CString &Server, const uint32_t Port );
  virtual int32_t Close();
  void LogSSLError();
private:
  void * ssl;
  void * ctx;
};

