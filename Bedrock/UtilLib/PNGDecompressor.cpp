#include "PNGDecompressor.h"

#include <string_view>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool DecompressPNG( const uint8_t *IData, int32_t IDataSize, uint8_t *&Image, int32_t &XRes, int32_t &YRes )
{
  int32_t x, y, n;
  uint8_t *Data = stbi_load_from_memory( IData, IDataSize, &x, &y, &n, 4 );

  if ( !Data )
  {
    LOG_ERR( "[png] Image decompression failed: %s", stbi_failure_reason() );
    return false;
  }

  XRes = x;
  YRes = y;
  Image = new uint8_t[ XRes*YRes * 4 ];
  memcpy( Image, Data, XRes*YRes * 4 );

  stbi_image_free( Data );
  return true;
}

void ARGBtoABGR( uint8_t *Image, int32_t XRes, int32_t YRes )
{
  uint8_t *img = Image;

  for ( int32_t x = 0; x < XRes*YRes; x++ )
  {
    uint8_t t = img[ 0 ];
    img[ 0 ] = img[ 2 ];
    img[ 2 ] = t;
    img += 4;
  }
}

void ClearZeroAlpha( uint8_t *Image, int32_t XRes, int32_t YRes )
{
  uint8_t *img = Image;

  for ( int32_t x = 0; x < XRes*YRes; x++ )
  {
    if ( img[ 3 ] == 0 )
      img[ 0 ] = img[ 1 ] = img[ 2 ] = 0;

    img += 4;
  }
}

bool ExportPNG( uint8_t *Image, int32_t XRes, int32_t YRes, bool ClearAlpha, std::string_view OutFile )
{
  auto Data = std::make_unique<uint8_t[]>( XRes*YRes * 4 );
  memcpy( Data.get(), Image, XRes*YRes * 4 );

  if ( ClearAlpha )
    for ( int32_t x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  auto FileName = std::make_unique<TS8[]>(OutFile.size() + 1 );
  _tcsncpy_s(FileName.get(), OutFile.size(), OutFile.data(), OutFile.size());

  bool result = stbi_write_png( FileName.get(), XRes, YRes, 4, Image, XRes * 4 );

  if ( !result )
    LOG_ERR( "[png] PNG export error ('%s')", std::string(OutFile).c_str() );

  return result;
}

bool ExportTga( uint8_t *Image, int32_t XRes, int32_t YRes, bool ClearAlpha, std::string_view OutFile )
{
  if ( ClearAlpha )
    for ( int32_t x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  auto FileName = std::make_unique<TS8[]>( OutFile.size() + 1 );
  _tcsncpy_s(FileName.get(), OutFile.size(), OutFile.data(),
             OutFile.size());

  bool result = stbi_write_tga( FileName.get(), XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] TGA export error ('%s')", std::string(OutFile).c_str() );

  return result;
}

bool ExportBmp( uint8_t *Image, int32_t XRes, int32_t YRes, std::string_view OutFile )
{
  auto FileName = std::make_unique<TS8[]>(OutFile.size() + 1);
  _tcsncpy_s(FileName.get(), OutFile.size(), OutFile.data(),
             OutFile.size());

  bool result = stbi_write_bmp( FileName.get(), XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", std::string(OutFile).c_str() );

  return result;
}

bool ExportRaw( uint8_t *Image, int32_t XRes, int32_t YRes, std::string_view OutFile )
{
  auto FileName = std::make_unique<TS8[]>(OutFile.size() + 1);
  _tcsncpy_s(FileName.get(), OutFile.size(), OutFile.data(),
             OutFile.size());

  bool result = stbi_write_bmp( FileName.get(), XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", std::string(OutFile).c_str() );

  return result;
}
