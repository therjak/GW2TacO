#include "src/base/socket.h"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>

WSADATA wsaData;
#include <mmsystem.h>
#include <ws2tcpip.h>  // This is where getaddrinfo is declared

#include <memory>
#include <string>
#include <vector>
#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

CSocket::CSocket() : StreamReader(), StreamWriter() {
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
  int32_t r = recv(Socket, static_cast<char*>(lpBuf),
                   static_cast<int32_t>(nCount), NULL);
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

  // First, try to parse as an IP address directly (IPv4)
  if (inet_pton(AF_INET, Address.c_str(), &addr) != 1) {
    // If not a valid IP address, try to resolve the hostname using getaddrinfo
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;        // We only want IPv4 addresses
    hints.ai_socktype = SOCK_STREAM;  // We are using TCP sockets

    struct addrinfo* result = nullptr;
    // The second parameter (service name) can be nullptr if we are only
    // resolving the host
    int iResult = getaddrinfo(Address.c_str(), nullptr, &hints, &result);

    if (iResult == 0 && result != nullptr) {
      // Successfully resolved. Iterate through results to find an IPv4 address.
      // We are interested in the first valid IPv4 address found.
      for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET && ptr->ai_socktype == SOCK_STREAM) {
          SOCKADDR_IN* ipv4Addr = reinterpret_cast<SOCKADDR_IN*>(ptr->ai_addr);
          addr = ipv4Addr->sin_addr.s_addr;
          break;  // Found an IPv4 address, exit loop
        }
      }
      freeaddrinfo(result);  // Free the address info structure
    } else {
      // getaddrinfo failed. The 'addr' remains INADDR_NONE.
      // Optionally, log the error: WSAGetLastError() could provide more info.
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

    auto available_bytes = static_cast<int32_t>(GetLength());

    if (available_bytes > 0) {
      auto data_buffer = std::vector<char>(available_bytes);

      if (Peek(&data_buffer[0], available_bytes)) {
        for (int32_t x = 0; x < available_bytes; x++) {
          if (data_buffer[x] == '\n') {
            ReadFull(&data_buffer[0], x + 1);
            result += std::string(&data_buffer[0], x);
            return result;
          }
        }
      }

      ReadFull(&data_buffer[0], available_bytes);
      result += std::string(&data_buffer[0], available_bytes);
    }
    if (!Peek(&available_bytes, 1)) return result;
  }
}

int32_t InitWinsock() { return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0; }

void DeinitWinsock() { WSACleanup(); }

#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

static int64_t GetCurrentTickCount64() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

CSocket::CSocket() : StreamReader(), StreamWriter() {
  Socket = INVALID_SOCKET;
  LastActivity = GetCurrentTickCount64();
}

CSocket::~CSocket() {
  if (Socket != INVALID_SOCKET) {
    close(Socket);
  }
}

int32_t CSocket::ReadStream(void* lpBuf, uint32_t nCount) {
  if (Socket == INVALID_SOCKET) return 0;
  ssize_t r = recv(Socket, lpBuf, nCount, 0);
  if (r > 0) LastActivity = GetCurrentTickCount64();
  return (r > 0) ? static_cast<int32_t>(r) : 0;
}

int32_t CSocket::WriteStream(std::string_view data) {
  if (Socket == INVALID_SOCKET) return 0;
  ssize_t res = send(Socket, data.data(), data.size(), 0);
  if (res == SOCKET_ERROR) return 0;
  return static_cast<int32_t>(res);
}

int32_t CSocket::Connect(std::string_view Server, const uint32_t Port) {
  uint32_t addr = Resolve(Server);
  if (addr == INADDR_NONE) return 0;

  Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Socket == INVALID_SOCKET) return 0;

  struct sockaddr_in serverInfo;
  std::memset(&serverInfo, 0, sizeof(serverInfo));
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.s_addr = addr;
  serverInfo.sin_port = htons(Port);

  if (connect(Socket, reinterpret_cast<struct sockaddr*>(&serverInfo),
              sizeof(serverInfo)) == SOCKET_ERROR) {
    close(Socket);
    Socket = INVALID_SOCKET;
    return 0;
  }

  LastActivity = GetCurrentTickCount64();
  return 1;
}

int32_t CSocket::Close() {
  if (Socket != INVALID_SOCKET) {
    close(Socket);
    Socket = INVALID_SOCKET;
  }
  return 1;
}

uint32_t CSocket::Resolve(std::string_view a) {
  struct addrinfo hints;
  struct addrinfo* res = nullptr;
  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  std::string addr_str(a);
  int status = getaddrinfo(addr_str.c_str(), nullptr, &hints, &res);
  if (status != 0 || !res) {
    return INADDR_NONE;
  }

  uint32_t ip = reinterpret_cast<struct sockaddr_in*>(res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  return ip;
}

int64_t CSocket::GetLength() const {
  if (Socket == INVALID_SOCKET) return 0;
  int bytes_available = 0;
  if (ioctl(Socket, FIONREAD, &bytes_available) == SOCKET_ERROR) {
    return 0;
  }
  return bytes_available;
}

int64_t CSocket::GetOffset() const { return 0; }

int32_t CSocket::ReadFull(void* data, uint32_t size) {
  if (Socket == INVALID_SOCKET) return 0;
  uint32_t total = 0;
  char* ptr = static_cast<char*>(data);
  while (total < size) {
    int32_t read = ReadStream(ptr + total, size - total);
    if (read <= 0) break;
    total += read;
  }
  return total;
}

bool CSocket::Peek(void* lpBuf, uint32_t nCount) {
  if (Socket == INVALID_SOCKET) return false;
  ssize_t r = recv(Socket, lpBuf, nCount, MSG_PEEK);
  return r >= 0;
}

bool CSocket::IsConnected() {
  if (Socket == INVALID_SOCKET) return false;
  int error = 0;
  socklen_t len = sizeof(error);
  int retval = getsockopt(Socket, SOL_SOCKET, SO_ERROR, &error, &len);
  if (retval || error) {
    close(Socket);
    Socket = INVALID_SOCKET;
    return false;
  }
  int count = 0;
  if (ioctl(Socket, FIONREAD, &count) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

const bool CSocket::operator==(const CSocket& b) { return Socket == b.Socket; }

std::string CSocket::ReadLine() {
  std::string result;
  while (true) {
    if (!IsConnected()) return result;
    auto available_bytes = static_cast<int32_t>(GetLength());
    if (available_bytes > 0) {
      std::vector<char> data_buffer(available_bytes);
      if (Peek(data_buffer.data(), available_bytes)) {
        for (int32_t x = 0; x < available_bytes; x++) {
          if (data_buffer[x] == '\n') {
            ReadFull(data_buffer.data(), x + 1);
            result.append(data_buffer.data(), x);
            return result;
          }
        }
      }
      ReadFull(data_buffer.data(), available_bytes);
      result.append(data_buffer.data(), available_bytes);
    }
    char c;
    if (!Peek(&c, 1)) return result;
  }
}

int32_t InitWinsock() { return 1; }
void DeinitWinsock() {}
#endif
