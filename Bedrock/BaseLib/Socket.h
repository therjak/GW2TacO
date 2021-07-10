#pragma once

#include <string_view>

#include "StreamReader.h"
#include "StreamWriter.h"

class CSocket : public CStreamReader, public CStreamWriter {
 protected:
  int64_t LastActivity;
  SOCKET Socket;

  virtual int32_t ReadStream(void *lpBuf, uint32_t nCount);
  virtual int32_t WriteStream(std::string_view);
  static uint32_t Resolve(std::string_view Address);

 public:
  CSocket();
  CSocket(const CSocket &) = delete;
  virtual ~CSocket();

  //////////////////////////////////////////////////////////////////////////
  // socket functions

  virtual int32_t Connect(std::string_view Server, const uint32_t Port);
  int32_t ReadFull(void *lpBuf, uint32_t nCount);

  virtual int32_t Close();

  //////////////////////////////////////////////////////////////////////////
  // streamreader functions

  // returns the currently available bytes in the socket
  virtual int64_t GetLength() const;  
  virtual int64_t GetOffset() const;  // is always 0

  std::string ReadLine();

  bool Peek(void *lpBuf, uint32_t nCount);

  bool IsConnected();

  const bool operator==(const CSocket &b);

  //////////////////////////////////////////////////////////////////////////
  // streamwriter functions
};

int32_t InitWinsock();
void DeinitWinsock();
