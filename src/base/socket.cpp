#include "src/base/socket.h"

#include <winsock2.h>

WSADATA wsaData;
#include <mmsystem.h>
#include <ws2tcpip.h>

#include <memory>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

CSocket::CSocket() : CStreamReader(), CStreamWriter() {
  Socket = INVALID_SOCKET;
  LastActivity = GetTickCount64();
}

CSocket::~CSocket() {
  if (Socket != INVALID_SOCKET) {
    closesocket(Socket);
  }
}

int32_t CSocket::ReadStream(void* lpBuf, uint32_t nCount) {
  if (Socket == INVALID_SOCKET) return 0;
  int32_t r = recv(Socket, static_cast<char*>(lpBuf), nCount, NULL);
  if (r != 0) LastActivity = GetTickCount64();
  return r;
}

int32_t CSocket::WriteStream(std::string_view data) {
  int32_t res = send(Socket, data.data(), data.size(), NULL);
  if (res == SOCKET_ERROR) return 0;
  return res;
}

int32_t CSocket::Connect(std::string_view Server, const uint32_t Port) {
  int32_t addr = Resolve(Server);
  if (addr == INADDR_NONE) return 0;

  Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Socket == INVALID_SOCKET) return 0;

  SOCKADDR_IN serverInfo;
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.S_un.S_addr = addr;
  serverInfo.sin_port = htons(Port);

  if (connect(Socket, reinterpret_cast<LPSOCKADDR>(&serverInfo),
              sizeof(sockaddr)) == SOCKET_ERROR) {
    return 0;
  }

  LastActivity = GetTickCount64();
  return 1;
}

int32_t CSocket::Close() {
  if (Socket != INVALID_SOCKET) {
    shutdown(Socket, SD_SEND);
    closesocket(Socket);
  }
  Socket = INVALID_SOCKET;

  return 1;
}

uint32_t CSocket::Resolve(std::string_view a) {
  std::string Address(a);
  uint32_t addr = INADDR_NONE;
  if (inet_pton(AF_INET, Address.c_str(), &addr) != 0) {
    addr = INADDR_NONE;
  }

  if (addr == INADDR_NONE) {
    hostent* hostEntry = gethostbyname(Address.c_str());
    if (hostEntry) {
      auto pa = reinterpret_cast<LPIN_ADDR>(hostEntry->h_addr);
      addr = pa->S_un.S_addr;
    }
  }

  return addr;
}

//////////////////////////////////////////////////////////////////////////
// streamreader functions

int64_t CSocket::GetLength() const {
  if (Socket == INVALID_SOCKET) return 0;
  unsigned long count = 0;
  if (ioctlsocket(Socket, FIONREAD, &count) == SOCKET_ERROR) return 0;
  return count;
}

int64_t CSocket::GetOffset() const { return 0; }

int32_t CSocket::ReadFull(void* data, uint32_t size) {
  int32_t progress = 0;
  while (true) {
    int32_t n = Read((static_cast<char*>(data)) + progress, size - progress);
    if (n == SOCKET_ERROR) return SOCKET_ERROR;
    progress += n;
    if (progress == size) break;
  }
  return size;
}

bool CSocket::Peek(void* lpBuf, uint32_t nCount) {
  if (Socket == INVALID_SOCKET) return false;
  int32_t r = recv(Socket, static_cast<char*>(lpBuf), nCount, MSG_PEEK);
  if (r != 0) LastActivity = GetTickCount64();
  if (r == SOCKET_ERROR) Socket = INVALID_SOCKET;
  return r != SOCKET_ERROR;
}

bool CSocket::IsConnected() {
  if (Socket == INVALID_SOCKET) return false;

  int error = 0;
  int len = sizeof(error);
  int retval = getsockopt(Socket, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char*>(&error), &len);

  if (retval || error) {
    Socket = INVALID_SOCKET;
    return false;
  }

  unsigned long count = 0;

  if (ioctlsocket(Socket, FIONREAD, &count) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

const bool CSocket::operator==(const CSocket& b) { return Socket == b.Socket; }

std::string CSocket::ReadLine() {
  std::string result;

  while (true) {
    if (!IsConnected()) return result;
    auto len = static_cast<int32_t>(GetLength());
    if (len) {
      auto dat = std::vector<char>(len);
      if (Peek(&dat[0], len)) {
        for (int32_t x = 0; x < len; x++) {
          if (dat[x] == '\n') {
            ReadFull(&dat[0], x + 1);
            result += std::string(&dat[0], x);
            return result;
          }
        }
      }

      ReadFull(&dat[0], len);
      result += std::string(&dat[0], len);
    }

    if (!Peek(&len, 1)) return result;
  }
}

int32_t InitWinsock() { return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0; }

void DeinitWinsock() { WSACleanup(); }
