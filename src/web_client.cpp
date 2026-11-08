#include <memory>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/base/stream_writer.h"
#include "src/base/string_format.h"

#ifdef _WIN32
#include <Urlmon.h>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Urlmon.lib")
#else
#include <curl/curl.h>
#endif

#ifndef _WIN32
static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb,
                                void* userp) {
  size_t realsize = size * nmemb;
  auto* str = static_cast<std::string*>(userp);
  str->append(static_cast<char*>(contents), realsize);
  return realsize;
}
#endif

extern "C++" std::string FetchHTTPS(std::string_view url,
                                    std::string_view path) {
#ifdef _WIN32
  auto wurl = string2wstring(url);
  auto wpath = string2wstring(path);

  Log_Nfo("[GW2TacO] Fetching URL: {:s}/{:s}", url, path);

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;

  BOOL bResults = FALSE;
  HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;

  hSession =
      WinHttpOpen(L"WinHTTPS Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

  if (hSession) {
    hConnect = WinHttpConnect(hSession, wurl.c_str(), INTERNET_DEFAULT_PORT, 0);
  }

  if (hConnect) {
    hRequest =
        WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(), nullptr,
                           WINHTTP_NO_REFERER, nullptr, WINHTTP_FLAG_SECURE);
  }

  if (hRequest) {
    bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                  WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  }

  if (bResults) {
    bResults = WinHttpReceiveResponse(hRequest, nullptr);
  }

  if (!bResults) {
    return "";
  }

  StreamWriterMemory data;

  do {
    dwSize = 0;
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
      return "";
    }

    auto pszOutBuffer = std::make_unique<char[]>(size_t(dwSize) + 1);

    if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer.get(), dwSize,
                         &dwDownloaded)) {
      return "";
    }

    data.Write(std::string_view(pszOutBuffer.get(), dwSize));
  } while (dwSize > 0);

  if (hRequest) {
    WinHttpCloseHandle(hRequest);
  }
  if (hConnect) {
    WinHttpCloseHandle(hConnect);
  }
  if (hSession) {
    WinHttpCloseHandle(hSession);
  }

  return std::string(reinterpret_cast<const char*>(data.GetData()),
                     data.GetLength());
#else
  std::string full_url = "https://" + std::string(url);
  if (!path.empty() && path[0] != '/') {
    full_url += "/";
  }
  full_url += std::string(path);

  Log_Nfo("[GW2TacO] Fetching URL: {:s}", full_url);

  CURL* curl = curl_easy_init();
  if (!curl) {
    Log_Err("[GW2TacO] Failed to init curl");
    return "";
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "GW2TacO-Linux/1.0");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    Log_Err("[GW2TacO] curl_easy_perform() failed: {:s}",
            curl_easy_strerror(res));
    response.clear();
  }

  curl_easy_cleanup(curl);
  return response;
#endif
}

extern "C++" bool DownloadFile(std::string_view url, StreamWriterMemory& mem) {
#ifdef _WIN32
  LPSTREAM stream = nullptr;

  HRESULT hr = URLOpenBlockingStream(
      nullptr, ("https://" + std::string(url)).c_str(), &stream, 0, nullptr);
  if (FAILED(hr)) {
    hr = URLOpenBlockingStream(nullptr, ("http://" + std::string(url)).c_str(),
                               &stream, 0, nullptr);
  }

  if (FAILED(hr)) {
    return false;
  }

  char buffer[4096];
  do {
    DWORD bytesRead = 0;
    hr = stream->Read(buffer, sizeof(buffer), &bytesRead);
    mem.Write(std::string_view(buffer, bytesRead));
  } while (SUCCEEDED(hr) && hr != S_FALSE);

  stream->Release();

  if (FAILED(hr)) {
    return false;
  }

  return true;
#else
  std::string full_url;
  if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
    full_url = "https://" + std::string(url);
  } else {
    full_url = std::string(url);
  }

  CURL* curl = curl_easy_init();
  if (!curl) return false;

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "GW2TacO-Linux/1.0");
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) return false;

  mem.Write(std::string_view(response.data(), response.size()));
  return true;
#endif
}
