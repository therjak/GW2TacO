#include "src/core2/core2_config.h"
#include "src/core2/enums.h"

namespace renderer {

Format GetFormat(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return Format::kA8R8G8B8;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      return Format::kA32B32G32R32f;
    case DXGI_FORMAT_R32_FLOAT:
      return Format::kR32f;
    case DXGI_FORMAT_R16G16_FLOAT:
      return Format::kG16R16f;
    default:
      return Format::kUnknown;
  }
}

}  // namespace renderer
