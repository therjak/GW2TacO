module;
#include <cstdint>

export module renderer:mouse_cursor;

export namespace renderer {

enum class MouseCursor : uint16_t {
  kArrow,
  kCross,
  kSizeWe,
  kSizeNs,
  kSizeNeSw,
  kSizeNwSe,
  kText,
  kWait,
};

}  // namespace renderer
