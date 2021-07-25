#pragma once
#include <memory>
#include <string_view>

#include "../BaseLib/BaseLib.h"

bool DecompressPNG(const uint8_t* IData, int32_t IDataSize,
                   std::unique_ptr<uint8_t[]>& Image, int32_t& XRes,
                   int32_t& YRes);
void ARGBtoABGR(uint8_t* Image, int32_t XRes, int32_t YRes);
void ClearZeroAlpha(uint8_t* Image, int32_t XRes, int32_t YRes);

bool ExportPNG(uint8_t* IData, int32_t XRes, int32_t YRes, bool ClearAlpha,
               std::string_view OutFile);
bool ExportTga(uint8_t* IData, int32_t XRes, int32_t YRes, bool ClearAlpha,
               std::string_view OutFile);
bool ExportBmp(uint8_t* Image, int32_t XRes, int32_t YRes,
               std::string_view OutFile);
