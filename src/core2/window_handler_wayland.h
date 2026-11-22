#pragma once

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "src/base/win_compat.h"
#include "src/core2/enums.h"
#include "src/core2/window_handler.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

namespace renderer {

class WindowHandlerWayland : public WindowHandler {
 public:
  WindowHandlerWayland();
  ~WindowHandlerWayland() override;

  bool Initialize(const WindowParameters& window_params) override;
  void Destroy() override;
  bool HandleMessages() override;
  bool HandleOSMessages() override;
  bool DeviceOk() override;
  void ToggleFullScreen() override;

  HWND GetHandle() override;

  void FinalizeMouseCursor() override;
  void SetWindowTitle(std::string_view title) override;

  void* GetWaylandDisplay() override { return display_; }
  void* GetWaylandSurface() override { return surface_; }

  virtual LRESULT WindowProc(UINT u_msg, WPARAM w_param, LPARAM l_param);

  // Dynamic input regions for Wayland overlay click-through
  void SetInputRegion(const std::vector<math::Rect>& rects);
  void ClearInputRegion();

  // Registry callbacks
  static void RegistryHandleGlobal(void* data, wl_registry* registry,
                                   uint32_t name, const char* interface,
                                   uint32_t version);
  static void RegistryHandleGlobalRemove(void* data, wl_registry* registry,
                                         uint32_t name);

  // Layer surface callbacks
  static void LayerSurfaceHandleConfigure(void* data,
                                          zwlr_layer_surface_v1* layer_surface,
                                          uint32_t serial, uint32_t width,
                                          uint32_t height);
  static void LayerSurfaceHandleClosed(void* data,
                                       zwlr_layer_surface_v1* layer_surface);

  // Seat callbacks
  static void SeatHandleCapabilities(void* data, wl_seat* seat,
                                     uint32_t capabilities);
  static void SeatHandleName(void* data, wl_seat* seat, const char* name);

  // Pointer callbacks
  static void PointerHandleEnter(void* data, wl_pointer* pointer,
                                 uint32_t serial, wl_surface* surface,
                                 wl_fixed_t sx, wl_fixed_t sy);
  static void PointerHandleLeave(void* data, wl_pointer* pointer,
                                 uint32_t serial, wl_surface* surface);
  static void PointerHandleMotion(void* data, wl_pointer* pointer,
                                  uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
  static void PointerHandleButton(void* data, wl_pointer* pointer,
                                  uint32_t serial, uint32_t time,
                                  uint32_t button, uint32_t state);
  static void PointerHandleAxis(void* data, wl_pointer* pointer, uint32_t time,
                                uint32_t axis, wl_fixed_t value);
  static void PointerHandleFrame(void* data, wl_pointer* pointer);
  static void PointerHandleAxisSource(void* data, wl_pointer* pointer,
                                      uint32_t axis_source);
  static void PointerHandleAxisStop(void* data, wl_pointer* pointer,
                                    uint32_t time, uint32_t axis);
  static void PointerHandleAxisDiscrete(void* data, wl_pointer* pointer,
                                        uint32_t axis, int32_t discrete);

  // Keyboard callbacks
  static void KeyboardHandleKeymap(void* data, wl_keyboard* keyboard,
                                   uint32_t format, int32_t fd, uint32_t size);
  static void KeyboardHandleEnter(void* data, wl_keyboard* keyboard,
                                  uint32_t serial, wl_surface* surface,
                                  wl_array* keys);
  static void KeyboardHandleLeave(void* data, wl_keyboard* keyboard,
                                  uint32_t serial, wl_surface* surface);
  static void KeyboardHandleKey(void* data, wl_keyboard* keyboard,
                                uint32_t serial, uint32_t time, uint32_t key,
                                uint32_t state);
  static void KeyboardHandleModifiers(void* data, wl_keyboard* keyboard,
                                      uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked, uint32_t group);
  static void KeyboardHandleRepeatInfo(void* data, wl_keyboard* keyboard,
                                       int32_t rate, int32_t delay);

 protected:
  void HandleResize() override;
  void HandleAltEnter() override;

 private:
  // Wayland core & shell objects
  wl_display* display_ = nullptr;
  wl_registry* registry_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  wl_surface* surface_ = nullptr;
  wl_shm* shm_ = nullptr;
  wl_seat* seat_ = nullptr;
  wl_pointer* pointer_ = nullptr;
  wl_keyboard* keyboard_ = nullptr;
  zwlr_layer_shell_v1* layer_shell_ = nullptr;
  zwlr_layer_surface_v1* layer_surface_ = nullptr;

  // Cursors
  wl_cursor_theme* cursor_theme_ = nullptr;
  wl_surface* cursor_surface_ = nullptr;
  uint32_t pointer_serial_ = 0;

  // Keyboard mapping
  xkb_context* xkb_context_ = nullptr;
  xkb_keymap* xkb_keymap_ = nullptr;
  xkb_state* xkb_state_ = nullptr;

  bool configured_ = false;
  bool input_region_initialized_ = false;
  std::vector<math::Rect> last_input_region_;

  void UpdateCursor();
};

}  // namespace renderer
