#pragma once
#include "../BaseLib/BaseLib.h"

TBOOL DecompressPNG( const uint8_t *IData, int32_t IDataSize, uint8_t *&Image, int32_t &XRes, int32_t &YRes );
void ARGBtoABGR( uint8_t *Image, int32_t XRes, int32_t YRes );
void ClearZeroAlpha( uint8_t *Image, int32_t XRes, int32_t YRes );

TBOOL ExportPNG( uint8_t *IData, int32_t XRes, int32_t YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportTga( uint8_t *IData, int32_t XRes, int32_t YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportBmp( uint8_t *Image, int32_t XRes, int32_t YRes, CString OutFile );
