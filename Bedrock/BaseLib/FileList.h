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
  CFileListEntry(const std::string& pth, const std::string& fn);
};

const bool operator==(const CFileListEntry& f1, const CFileListEntry& f2);

class CFileList {
 public:
  std::vector<CFileListEntry> Files;

  CFileList();
  ~CFileList();
  CFileList(std::string_view Mask, std::string_view Path = "",
            bool Recursive = false);

  void ExpandSearch(std::string_view Mask, std::string_view Path,
                    bool Recursive, bool getDirectories = false);
};

bool exists(std::string_view fname);
