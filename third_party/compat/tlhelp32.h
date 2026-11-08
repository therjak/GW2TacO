#pragma once

#include <dirent.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "src/base/win_compat.h"

constexpr DWORD TH32CS_SNAPPROCESS = 0x00000002;

struct PROCESSENTRY32 {
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ProcessID;
  ULONG_PTR th32DefaultHeapID;
  DWORD th32ModuleID;
  DWORD cntThreads;
  DWORD th32ParentProcessID;
  LONG pcPriClassBase;
  DWORD dwFlags;
  char szExeFile[260];
};

struct ToolhelpSnapshot {
  std::vector<PROCESSENTRY32> processes;
  size_t current_index = 0;
};

inline HANDLE CreateToolhelp32Snapshot(DWORD dwFlags, DWORD /*th32ProcessID*/) {
  if (!(dwFlags & TH32CS_SNAPPROCESS)) {
    return INVALID_HANDLE_VALUE;
  }

  auto snap = std::make_unique<ToolhelpSnapshot>();

  DIR* proc_dir = opendir("/proc");
  if (!proc_dir) {
    return INVALID_HANDLE_VALUE;
  }

  struct dirent* entry;
  while ((entry = readdir(proc_dir)) != nullptr) {
    if (!std::isdigit(entry->d_name[0])) continue;

    DWORD pid = static_cast<DWORD>(std::atoi(entry->d_name));
    std::string comm_path = std::string("/proc/") + entry->d_name + "/comm";
    FILE* f = fopen(comm_path.c_str(), "r");
    char comm[260] = {0};
    if (f) {
      if (fgets(comm, sizeof(comm), f)) {
        size_t len = strlen(comm);
        while (len > 0 && (comm[len - 1] == '\n' || comm[len - 1] == '\r')) {
          comm[len - 1] = '\0';
          len--;
        }
      }
      fclose(f);
    }

    std::string cmdline_path = std::string("/proc/") + entry->d_name + "/cmdline";
    FILE* f_cmd = fopen(cmdline_path.c_str(), "r");
    if (f_cmd) {
      char cmdline[1024] = {0};
      size_t read_bytes = fread(cmdline, 1, sizeof(cmdline) - 1, f_cmd);
      fclose(f_cmd);
      if (read_bytes > 0) {
        if (strstr(cmdline, "Gw2-64.exe") || strstr(cmdline, "gw2-64.exe")) {
          strncpy_s(comm, sizeof(comm), "Gw2-64.exe", _TRUNCATE);
        } else if (strstr(cmdline, "Gw2.exe") || strstr(cmdline, "gw2.exe")) {
          strncpy_s(comm, sizeof(comm), "Gw2.exe", _TRUNCATE);
        }
      }
    }

    PROCESSENTRY32 pe{};
    pe.dwSize = sizeof(PROCESSENTRY32);
    pe.th32ProcessID = pid;
    strncpy_s(pe.szExeFile, sizeof(pe.szExeFile), comm, _TRUNCATE);
    snap->processes.push_back(pe);
  }
  closedir(proc_dir);

  return reinterpret_cast<HANDLE>(snap.release());
}

inline BOOL Process32First(HANDLE hSnapshot, PROCESSENTRY32* lppe) {
  if (!hSnapshot || hSnapshot == INVALID_HANDLE_VALUE || !lppe) return FALSE;
  auto* snap = reinterpret_cast<ToolhelpSnapshot*>(hSnapshot);
  snap->current_index = 0;
  if (snap->processes.empty()) return FALSE;
  *lppe = snap->processes[0];
  snap->current_index = 1;
  return TRUE;
}

inline BOOL Process32Next(HANDLE hSnapshot, PROCESSENTRY32* lppe) {
  if (!hSnapshot || hSnapshot == INVALID_HANDLE_VALUE || !lppe) return FALSE;
  auto* snap = reinterpret_cast<ToolhelpSnapshot*>(hSnapshot);
  if (snap->current_index >= snap->processes.size()) return FALSE;
  *lppe = snap->processes[snap->current_index++];
  return TRUE;
}
