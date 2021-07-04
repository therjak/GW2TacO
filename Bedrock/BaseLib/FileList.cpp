#include "BaseLib.h"

const bool operator==(const CFileListEntry &f1, const CFileListEntry &f2) {
  return f1.Path == f2.Path && f1.FileName == f2.FileName;
}

bool exists( std::string_view fname )
{
  FILE *f = NULL;
  if ( fopen_s( &f, fname.data(), "rb" ) )
    return false;
  if ( !f ) return false;
  fclose( f );
  return true;
}

CFileList::CFileList( std::string_view Mask, std::string_view Path/* ="" */, bool Recursive/* =false */ )
{
  ExpandSearch( Mask, Path, Recursive );
}

CFileList::CFileList()
{

}

void CFileList::ExpandSearch( std::string_view Mask, std::string_view Path, bool Recursive, bool getDirectories )
{
  HANDLE hSearch;
  WIN32_FIND_DATA FileData;

  std::string ValidPath(Path);
  if ( ValidPath[ ValidPath.length() - 1 ] != '/' && ValidPath[ ValidPath.length() - 1 ] != '\\' )
    ValidPath += "/";

  hSearch = FindFirstFile( ( ValidPath + Mask.data() ).c_str(), &FileData );

  if ( hSearch != INVALID_HANDLE_VALUE )
  {
    BOOL fFinished = FALSE;
    while ( !fFinished )
    {
      std::string FileName = FileData.cFileName;

      if ( !( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) && FileName != "." && FileName != ".." )
      {
        Files.emplace_back( CFileListEntry( ValidPath, FileName ) );
      }

      if ( getDirectories && ( ( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY ) && FileName != "." && FileName != ".." ) )
      {
        CFileListEntry e( ValidPath, FileName );
        e.isDirectory = true;
        Files.emplace_back(e);
      }

      fFinished = !FindNextFile( hSearch, &FileData );
    }
  }
  FindClose( hSearch );

  //find all directories
  if ( Recursive )
  {
    hSearch = FindFirstFile( ( ValidPath + "*.*" ).c_str(), &FileData );
    if ( hSearch != INVALID_HANDLE_VALUE )
    {
      BOOL fFinished = FALSE;
      while ( !fFinished )
      {
        CString FileName = FileData.cFileName;

        if ( FileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && FileName != "." && FileName != ".." )
        {
          ExpandSearch( Mask, ValidPath + FileData.cFileName + "/", Recursive );
        }
        fFinished = !FindNextFile( hSearch, &FileData );
      }
    }
    FindClose( hSearch );
  }
}

CFileList::~CFileList() {}

CFileListEntry::CFileListEntry(std::string &&pth, std::string &&fn)
    :

      Path(std::move(pth)),
      FileName(std::move(fn)) {}

CFileListEntry::CFileListEntry(const std::string &pth, const std::string &fn)
    : Path(pth), FileName(fn) {}

CFileListEntry::CFileListEntry() {}
