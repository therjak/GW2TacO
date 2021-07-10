#pragma once
#include "../BaseLib/BaseLib.h"
#include <string_view>

bool DecompressPNG( const uint8_t *IData, int32_t IDataSize, uint8_t *&Image, int32_t &XRes, int32_t &YRes );
void ARGBtoABGR( uint8_t *Image, int32_t XRes, int32_t YRes );
void ClearZeroAlpha( uint8_t *Image, int32_t XRes, int32_t YRes );

bool ExportPNG( uint8_t *IData, int32_t XRes, int32_t YRes, bool ClearAlpha, std::string_view OutFile );
bool ExportTga(uint8_t *IData, int32_t XRes, int32_t YRes, bool ClearAlpha,
               std::string_view OutFile);
bool ExportBmp(uint8_t *Image, int32_t XRes, int32_t YRes,
               std::string_view OutFile);
