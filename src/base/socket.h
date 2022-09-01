#pragma once

#include <string>
#include <string_view>

#include "src/base/stream_reader.h"
#include "src/base/stream_writer.h"

typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;
typedef UINT_PTR SOCKET;

class CSocket : public CStreamReader, public CStreamWriter {
 public:
  CSocket();
  CSocket(const CSocket&) = delete;
  ~CSocket() override;

  virtual int32_t Connect(std::string_view Server, const uint32_t Port);
  int32_t ReadFull(void* lpBuf, uint32_t nCount);

  virtual int32_t Close();

  // returns the currently available bytes in the socket
  [[nodiscard]] int64_t GetLength() const override;
  [[nodiscard]] int64_t GetOffset() const override;  // is always 0

  std::string ReadLine();

  bool Peek(void* lpBuf, uint32_t nCount);

  bool IsConnected();

  const bool operator==(const CSocket& b);

 protected:
  int64_t LastActivity;
  SOCKET Socket;

  int32_t ReadStream(void* lpBuf, uint32_t nCount) override;
  int32_t WriteStream(std::string_view) override;
  static uint32_t Resolve(std::string_view Address);
};

int32_t InitWinsock();
void DeinitWinsock();
