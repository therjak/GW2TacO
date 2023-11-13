#pragma once

#include <string>
#include <string_view>
#include <vector>

class CFileListEntry {
 public:
  std::string Path;
  std::string FileName;
  bool isDirectory = false;

  CFileListEntry();
  CFileListEntry(std::string&& pth, std::string&& fn);
  CFileListEntry(std::string pth, std::string fn);

  constexpr friend bool operator==(const CFileListEntry& f1,
                                   const CFileListEntry& f2) = default;
};

class CFileList {
 public:
  std::vector<CFileListEntry> Files;

  CFileList();
  ~CFileList();
  explicit CFileList(std::string_view Mask, std::string_view Path = "",
                     bool Recursive = false);

  void ExpandSearch(std::string_view Mask, std::string_view Path,
                    bool Recursive, bool getDirectories = false);
};

bool exists(std::string_view fname);
