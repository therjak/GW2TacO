#pragma once

#include <string>
#include <string_view>
#include <vector>

class FileListEntry {
 public:
  std::string Path;
  std::string FileName;
  bool isDirectory = false;

  FileListEntry();
  FileListEntry(std::string&& pth, std::string&& fn);
  FileListEntry(std::string pth, std::string fn);

  constexpr friend bool operator==(const FileListEntry& f1,
                                   const FileListEntry& f2) = default;
};

class FileList {
 public:
  std::vector<FileListEntry> Files;

  FileList();
  ~FileList();
  explicit FileList(std::string_view Mask, std::string_view Path = "",
                    bool Recursive = false);

  void ExpandSearch(std::string_view Mask, std::string_view Path,
                    bool Recursive, bool getDirectories = false);
};

bool exists(std::string_view fname);
