#include "src/base/file_list.h"

#ifdef _WIN32
#include <windows.h>
#else
#include "src/base/win_compat.h"
#include <filesystem>
#include <fnmatch.h>
#endif

#include <cstdio>
#include <string>
#include <utility>

bool exists(std::string_view fname) {
  FILE* f = nullptr;
  if (fopen_s(&f, fname.data(), "rb")) return false;
  if (!f) return false;
  fclose(f);
  return true;
}

FileList::FileList(std::string_view Mask, std::string_view Path /* ="" */,
                   bool Recursive /* =false */) {
  ExpandSearch(Mask, Path, Recursive);
}

FileList::FileList() = default;

#ifdef _WIN32
void FileList::ExpandSearch(std::string_view Mask, std::string_view Path,
                            bool Recursive, bool getDirectories) {
  HANDLE hSearch = nullptr;
  WIN32_FIND_DATA FileData;

  std::string ValidPath(Path);
  if (ValidPath[ValidPath.length() - 1] != '/' &&
      ValidPath[ValidPath.length() - 1] != '\\') {
    ValidPath += "/";
  }

  hSearch = FindFirstFile((ValidPath + Mask.data()).c_str(), &FileData);

  if (hSearch != INVALID_HANDLE_VALUE) {
    bool fFinished = false;
    while (!fFinished) {
      std::string FileName = FileData.cFileName;

      if (!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
          FileName != "." && FileName != "..") {
        Files.emplace_back(ValidPath, FileName);
      }

      if (getDirectories &&
          ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
           FileName != "." && FileName != "..")) {
        FileListEntry e(ValidPath, FileName);
        e.isDirectory = true;
        Files.emplace_back(e);
      }

      fFinished = !FindNextFile(hSearch, &FileData);
    }
  }
  FindClose(hSearch);

  // find all directories
  if (Recursive) {
    hSearch = FindFirstFile((ValidPath + "*.*").c_str(), &FileData);
    if (hSearch != INVALID_HANDLE_VALUE) {
      bool fFinished = false;
      while (!fFinished) {
        std::string FileName = FileData.cFileName;

        if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
            FileName != "." && FileName != "..") {
          ExpandSearch(Mask, ValidPath + FileData.cFileName + "/", Recursive);
        }
        fFinished = !FindNextFile(hSearch, &FileData);
      }
    }
    FindClose(hSearch);
  }
}
#else
void FileList::ExpandSearch(std::string_view Mask, std::string_view Path,
                            bool Recursive, bool getDirectories) {
  namespace fs = std::filesystem;
  std::string ValidPath(Path);
  if (ValidPath.empty()) {
    ValidPath = "./";
  } else if (ValidPath.back() != '/' && ValidPath.back() != '\\') {
    ValidPath += "/";
  }

  std::error_code ec;
  if (!fs::exists(ValidPath, ec) || !fs::is_directory(ValidPath, ec)) {
    return;
  }

  std::string mask_str(Mask);
  // Match *.* as * (match all)
  if (mask_str == "*.*") {
    mask_str = "*";
  }

  for (const auto& entry : fs::directory_iterator(ValidPath, ec)) {
    std::string filename = entry.path().filename().string();
    if (filename == "." || filename == "..") continue;

    bool is_dir = entry.is_directory(ec);
    if (!is_dir) {
      if (mask_str.empty() || mask_str == "*" ||
          fnmatch(mask_str.c_str(), filename.c_str(), FNM_CASEFOLD) == 0) {
        Files.emplace_back(ValidPath, filename);
      }
    } else {
      if (getDirectories) {
        FileListEntry e(ValidPath, filename);
        e.isDirectory = true;
        Files.emplace_back(e);
      }
      if (Recursive) {
        ExpandSearch(Mask, ValidPath + filename + "/", Recursive, getDirectories);
      }
    }
  }
}
#endif

FileList::~FileList() = default;

FileListEntry::FileListEntry(std::string&& pth, std::string&& fn)
    :

      Path(std::move(pth)),
      FileName(std::move(fn)) {}

FileListEntry::FileListEntry(std::string pth, std::string fn)
    : Path(std::move(pth)), FileName(std::move(fn)) {}

FileListEntry::FileListEntry() = default;
