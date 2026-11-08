#pragma once

#include <cwchar>

#include "src/base/win_compat.h"

using HINTERNET = void*;

constexpr DWORD WINHTTP_ACCESS_TYPE_DEFAULT_PROXY = 0;
#define WINHTTP_NO_PROXY_NAME nullptr
#define WINHTTP_NO_PROXY_BYPASS nullptr
constexpr int INTERNET_DEFAULT_PORT = 0;
#define WINHTTP_NO_ADDITIONAL_HEADERS nullptr
#define WINHTTP_NO_REQUEST_DATA nullptr
#define WINHTTP_NO_REFERER nullptr
constexpr DWORD WINHTTP_FLAG_SECURE = 0x00800000;

inline HINTERNET WinHttpOpen(LPCWSTR /*pszAgentW*/, DWORD /*dwAccessType*/,
                             LPCWSTR /*pszProxyW*/, LPCWSTR /*pszProxyBypassW*/,
                             DWORD /*dwFlags*/) {
  return nullptr;
}

inline HINTERNET WinHttpConnect(HINTERNET /*hSession*/, LPCWSTR /*pswzServerName*/,
                                int /*nServerPort*/, DWORD /*dwReserved*/) {
  return nullptr;
}

inline HINTERNET WinHttpOpenRequest(HINTERNET /*hConnect*/, LPCWSTR /*pwszVerb*/,
                                    LPCWSTR /*pwszObjectName*/,
                                    LPCWSTR /*pwszVersion*/,
                                    LPCWSTR /*pwszReferrer*/,
                                    LPCWSTR* /*ppwszAcceptTypes*/,
                                    DWORD /*dwFlags*/) {
  return nullptr;
}

inline BOOL WinHttpSendRequest(HINTERNET /*hRequest*/, LPCWSTR /*lpszHeaders*/,
                               DWORD /*dwHeadersLength*/, LPVOID /*lpOptional*/,
                               DWORD /*dwOptionalLength*/,
                               DWORD /*dwTotalLength*/, DWORD_PTR /*dwContext*/) {
  return FALSE;
}

inline BOOL WinHttpReceiveResponse(HINTERNET /*hRequest*/, LPVOID /*lpReserved*/) {
  return FALSE;
}

inline BOOL WinHttpQueryDataAvailable(HINTERNET /*hRequest*/, LPDWORD lpdwNumberOfBytesAvailable) {
  if (lpdwNumberOfBytesAvailable) *lpdwNumberOfBytesAvailable = 0;
  return FALSE;
}

inline BOOL WinHttpReadData(HINTERNET /*hRequest*/, LPVOID /*lpBuffer*/,
                            DWORD /*dwNumberOfBytesToRead*/,
                            LPDWORD lpdwNumberOfBytesRead) {
  if (lpdwNumberOfBytesRead) *lpdwNumberOfBytesRead = 0;
  return FALSE;
}

inline BOOL WinHttpCloseHandle(HINTERNET /*hInternet*/) {
  return TRUE;
}
