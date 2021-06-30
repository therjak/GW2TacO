#include "PNGDecompressor.h"

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

bool ExportPNG( uint8_t *Image, int32_t XRes, int32_t YRes, bool ClearAlpha, CString OutFile )
{
  uint8_t *Data = new uint8_t[ XRes*YRes * 4 ];
  memcpy( Data, Image, XRes*YRes * 4 );

  if ( ClearAlpha )
    for ( int32_t x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  bool result = stbi_write_png( FileName, XRes, YRes, 4, Image, XRes * 4 );

  if ( !result )
    LOG_ERR( "[png] PNG export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( Data );
  SAFEDELETEA( FileName );

  return result;
}

bool ExportTga( uint8_t *Image, int32_t XRes, int32_t YRes, bool ClearAlpha, CString OutFile )
{
  if ( ClearAlpha )
    for ( int32_t x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  bool result = stbi_write_tga( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] TGA export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}

bool ExportBmp( uint8_t *Image, int32_t XRes, int32_t YRes, CString OutFile )
{
  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  bool result = stbi_write_bmp( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}

bool ExportRaw( uint8_t *Image, int32_t XRes, int32_t YRes, CString OutFile )
{
  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  bool result = stbi_write_bmp( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}
