#include "src/core2/core2_config.h"
#include "src/core2/enums.h"

COREFORMAT GetFormat(DXGI_FORMAT Format) {
  switch (Format) {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return COREFORMAT::A8R8G8B8;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return COREFORMAT::A32B32G32R32F;
    case DXGI_FORMAT_R32_FLOAT:
      return COREFORMAT::R32F;
    case DXGI_FORMAT_R16G16_FLOAT:
      return COREFORMAT::G16R16F;
    default:
      return COREFORMAT::UNKNOWN;
  }
}
