#include "src/core2/core2_config.h"
#include "src/core2/enums.h"

namespace renderer {

CoreFormat GetFormat(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return CoreFormat::kA8R8G8B8;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return CoreFormat::kA32B32G32R32f;
    case DXGI_FORMAT_R32_FLOAT:
      return CoreFormat::kR32f;
    case DXGI_FORMAT_R16G16_FLOAT:
      return CoreFormat::kG16R16f;
    default:
      return CoreFormat::kUnknown;
  }
}

}  // namespace renderer
