#pragma once
#include "../BaseLib/BaseLib.h"

TBOOL DecompressPNG( const TU8 *IData, int32_t IDataSize, TU8 *&Image, int32_t &XRes, int32_t &YRes );
void ARGBtoABGR( TU8 *Image, int32_t XRes, int32_t YRes );
void ClearZeroAlpha( TU8 *Image, int32_t XRes, int32_t YRes );

TBOOL ExportPNG( TU8 *IData, int32_t XRes, int32_t YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportTga( TU8 *IData, int32_t XRes, int32_t YRes, TBOOL ClearAlpha, CString OutFile );
TBOOL ExportBmp( TU8 *Image, int32_t XRes, int32_t YRes, CString OutFile );
