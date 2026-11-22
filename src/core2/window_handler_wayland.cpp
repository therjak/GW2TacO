#include "src/core2/window_handler_wayland.h"

#include <linux/input-event-codes.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

#include "src/base/logger.h"
#include "src/base/timer.h"
#include "src/base/win_compat.h"
#include "src/core2/device.h"
#include "src/core2/enums.h"
#include "src/core2/window_handler.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

import math;

using math::Point;
using math::Rect;

namespace renderer {

namespace {

uint32_t XkbKeysymToVk(xkb_keysym_t sym) {
  switch (sym) {
    case XKB_KEY_Escape:
      return VK_ESCAPE;
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
      return VK_RETURN;
    case XKB_KEY_BackSpace:
      return VK_BACK;
    case XKB_KEY_Tab:
      return VK_TAB;
    case XKB_KEY_space:
      return VK_SPACE;
    case XKB_KEY_Shift_L:
      return VK_LSHIFT;
    case XKB_KEY_Shift_R:
      return VK_RSHIFT;
    case XKB_KEY_Control_L:
      return VK_LCONTROL;
    case XKB_KEY_Control_R:
      return VK_RCONTROL;
    case XKB_KEY_Alt_L:
      return VK_LMENU;
    case XKB_KEY_Alt_R:
      return VK_RMENU;
    case XKB_KEY_Left:
      return VK_LEFT;
    case XKB_KEY_Right:
      return VK_RIGHT;
    case XKB_KEY_Up:
      return VK_UP;
    case XKB_KEY_Down:
      return VK_DOWN;
    case XKB_KEY_Home:
      return VK_HOME;
    case XKB_KEY_End:
      return VK_END;
    case XKB_KEY_Page_Up:
      return VK_PRIOR;
    case XKB_KEY_Page_Down:
      return VK_NEXT;
    case XKB_KEY_Insert:
      return VK_INSERT;
    case XKB_KEY_Delete:
      return VK_DELETE;
    case XKB_KEY_Print:
      return VK_SNAPSHOT;
    case XKB_KEY_F1:
      return VK_F1;
    case XKB_KEY_F2:
      return VK_F2;
    case XKB_KEY_F3:
      return VK_F3;
    case XKB_KEY_F4:
      return VK_F4;
    case XKB_KEY_F5:
      return VK_F5;
    case XKB_KEY_F6:
      return VK_F6;
    case XKB_KEY_F7:
      return VK_F7;
    case XKB_KEY_F8:
      return VK_F8;
    case XKB_KEY_F9:
      return VK_F9;
    case XKB_KEY_F10:
      return VK_F10;
    case XKB_KEY_F11:
      return VK_F11;
    case XKB_KEY_F12:
      return VK_F12;
    default:
      if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) {
        return static_cast<uint32_t>('A' + (sym - XKB_KEY_a));
      }
      if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) {
        return static_cast<uint32_t>(sym);
      }
      if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) {
        return static_cast<uint32_t>(sym);
      }
      return 0;
  }
}

const struct wl_registry_listener kRegistryListener = {
    WindowHandlerWayland::RegistryHandleGlobal,
    WindowHandlerWayland::RegistryHandleGlobalRemove,
};

const struct zwlr_layer_surface_v1_listener kLayerSurfaceListener = {
    WindowHandlerWayland::LayerSurfaceHandleConfigure,
    WindowHandlerWayland::LayerSurfaceHandleClosed,
};

const struct wl_seat_listener kSeatListener = {
    WindowHandlerWayland::SeatHandleCapabilities,
    WindowHandlerWayland::SeatHandleName,
};

const struct wl_pointer_listener kPointerListener = {
    WindowHandlerWayland::PointerHandleEnter,
    WindowHandlerWayland::PointerHandleLeave,
    WindowHandlerWayland::PointerHandleMotion,
    WindowHandlerWayland::PointerHandleButton,
    WindowHandlerWayland::PointerHandleAxis,
    WindowHandlerWayland::PointerHandleFrame,
    WindowHandlerWayland::PointerHandleAxisSource,
    WindowHandlerWayland::PointerHandleAxisStop,
    WindowHandlerWayland::PointerHandleAxisDiscrete,
};

const struct wl_keyboard_listener kKeyboardListener = {
    WindowHandlerWayland::KeyboardHandleKeymap,
    WindowHandlerWayland::KeyboardHandleEnter,
    WindowHandlerWayland::KeyboardHandleLeave,
    WindowHandlerWayland::KeyboardHandleKey,
    WindowHandlerWayland::KeyboardHandleModifiers,
    WindowHandlerWayland::KeyboardHandleRepeatInfo,
};

}  // namespace

WindowHandlerWayland::WindowHandlerWayland() : WindowHandler() {
  xkb_context_ = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

WindowHandlerWayland::~WindowHandlerWayland() {
  Destroy();
  if (xkb_state_) {
    xkb_state_unref(xkb_state_);
    xkb_state_ = nullptr;
  }
  if (xkb_keymap_) {
    xkb_keymap_unref(xkb_keymap_);
    xkb_keymap_ = nullptr;
  }
  if (xkb_context_) {
    xkb_context_unref(xkb_context_);
    xkb_context_ = nullptr;
  }
}

bool WindowHandlerWayland::Initialize(const WindowParameters& window_params) {
  x_res_ = window_params.x_res_;
  y_res_ = window_params.y_res_;
  init_parameters_ = window_params;

  display_ = wl_display_connect(nullptr);
  if (!display_) {
    Log_Err("[wayland] Failed to connect to Wayland display");
    return false;
  }

  registry_ = wl_display_get_registry(display_);
  wl_registry_add_listener(registry_, &kRegistryListener, this);
  wl_display_roundtrip(display_);

  if (!compositor_) {
    Log_Err("[wayland] Compositor not available");
    return false;
  }

  if (!layer_shell_) {
    Log_Err("[wayland] wlr-layer-shell not supported by compositor");
    return false;
  }

  surface_ = wl_compositor_create_surface(compositor_);
  if (!surface_) {
    Log_Err("[wayland] Failed to create Wayland surface");
    return false;
  }
  window_handle_ = reinterpret_cast<HWND>(surface_);

  layer_surface_ = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell_, surface_, nullptr, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
      "gw2taco");
  if (!layer_surface_) {
    Log_Err("[wayland] Failed to create layer surface");
    return false;
  }

  zwlr_layer_surface_v1_add_listener(layer_surface_, &kLayerSurfaceListener,
                                     this);

  // Full-screen overlay spanning all 4 anchors with no margins or exclusive
  // zone
  const uint32_t anchors =
      ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
      ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
  zwlr_layer_surface_v1_set_anchor(layer_surface_, anchors);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface_, -1);
  zwlr_layer_surface_v1_set_size(layer_surface_, 0, 0);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface_, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

  // Start click-through (empty input region) until interactive UI widgets are
  // laid out, so the overlay never blocks the game by default.
  struct wl_region* empty_region = wl_compositor_create_region(compositor_);
  wl_surface_set_input_region(surface_, empty_region);
  wl_region_destroy(empty_region);
  last_input_region_.clear();
  input_region_initialized_ = true;

  // Commit surface to request initial configure event from compositor
  wl_surface_commit(surface_);

  // Dispatch until initial configure arrives
  while (!configured_ && !done_) {
    if (wl_display_dispatch(display_) < 0) {
      Log_Err("[wayland] Display dispatch failed during configure wait");
      return false;
    }
  }

  if (shm_) {
    cursor_theme_ = wl_cursor_theme_load(nullptr, 24, shm_);
    cursor_surface_ = wl_compositor_create_surface(compositor_);
  }

  // Create graphics device
  device_ = window_params.CreateDevice();
  if (!device_) {
    Log_Err("[init] Device object is NULL during init");
    return false;
  }

  if (!device_->Initialize(this)) {
    Log_Err("[init] Failed to initialize device");
    device_.reset();
    return false;
  }

  active_ = true;
  return true;
}

void WindowHandlerWayland::Destroy() {
  done_ = true;
  if (device_) {
    device_.reset();
  }
  if (cursor_surface_) {
    wl_surface_destroy(cursor_surface_);
    cursor_surface_ = nullptr;
  }
  if (cursor_theme_) {
    wl_cursor_theme_destroy(cursor_theme_);
    cursor_theme_ = nullptr;
  }
  if (pointer_) {
    wl_pointer_destroy(pointer_);
    pointer_ = nullptr;
  }
  if (keyboard_) {
    wl_keyboard_destroy(keyboard_);
    keyboard_ = nullptr;
  }
  if (seat_) {
    wl_seat_destroy(seat_);
    seat_ = nullptr;
  }
  if (layer_surface_) {
    zwlr_layer_surface_v1_destroy(layer_surface_);
    layer_surface_ = nullptr;
  }
  if (surface_) {
    wl_surface_destroy(surface_);
    surface_ = nullptr;
  }
  if (layer_shell_) {
    zwlr_layer_shell_v1_destroy(layer_shell_);
    layer_shell_ = nullptr;
  }
  if (compositor_) {
    wl_compositor_destroy(compositor_);
    compositor_ = nullptr;
  }
  if (shm_) {
    wl_shm_destroy(shm_);
    shm_ = nullptr;
  }
  if (registry_) {
    wl_registry_destroy(registry_);
    registry_ = nullptr;
  }
  if (display_) {
    wl_display_disconnect(display_);
    display_ = nullptr;
  }
}

bool WindowHandlerWayland::HandleMessages() { return HandleOSMessages(); }

bool WindowHandlerWayland::HandleOSMessages() {
  if (!display_ || done_) return false;

  wl_display_dispatch_pending(display_);
  if (wl_display_flush(display_) < 0 && errno != EAGAIN) {
    done_ = true;
    return false;
  }

  struct pollfd pfd = {wl_display_get_fd(display_), POLLIN, 0};
  const int ret = poll(&pfd, 1, 0);
  if (ret > 0) {
    if (wl_display_prepare_read(display_) == 0) {
      wl_display_read_events(display_);
      wl_display_dispatch_pending(display_);
    } else {
      wl_display_dispatch_pending(display_);
    }
  }

  return !done_;
}

bool WindowHandlerWayland::DeviceOk() {
  if (!device_) return false;
  return device_->DeviceOk();
}

void WindowHandlerWayland::ToggleFullScreen() {
  // Layer surface on OVERLAY layer is always fullscreen overlay
}

HWND WindowHandlerWayland::GetHandle() {
  return reinterpret_cast<HWND>(surface_);
}

void WindowHandlerWayland::HandleResize() {
  if (device_) {
    device_->Resize(x_res_, y_res_);
  }
}

void WindowHandlerWayland::HandleAltEnter() {}

LRESULT WindowHandlerWayland::WindowProc(UINT /*u_msg*/, WPARAM /*w_param*/,
                                         LPARAM /*l_param*/) {
  return 0;
}

void WindowHandlerWayland::SetWindowTitle(std::string_view /*title*/) {}

void WindowHandlerWayland::SetInputRegion(
    const std::vector<math::Rect>& rects) {
  if (!compositor_ || !surface_) return;

  if (input_region_initialized_ && rects == last_input_region_) {
    return;
  }

  struct wl_region* region = wl_compositor_create_region(compositor_);
  for (const auto& r : rects) {
    const int32_t w = r.x2 - r.x1;
    const int32_t h = r.y2 - r.y1;
    if (w > 0 && h > 0) {
      wl_region_add(region, r.x1, r.y1, w, h);
    }
  }
  wl_surface_set_input_region(surface_, region);
  wl_region_destroy(region);
  wl_surface_commit(surface_);

  last_input_region_ = rects;
  input_region_initialized_ = true;
}

void WindowHandlerWayland::ClearInputRegion() { SetInputRegion({}); }

void WindowHandlerWayland::FinalizeMouseCursor() { UpdateCursor(); }

void WindowHandlerWayland::UpdateCursor() {
  if (!cursor_theme_ || !cursor_surface_ || !pointer_) return;

  const char* cursor_name = "default";
  switch (current_mouse_cursor_) {
    case MouseCursor::kArrow:
      cursor_name = "default";
      break;
    case MouseCursor::kCross:
      cursor_name = "crosshair";
      break;
    case MouseCursor::kSizeWe:
      cursor_name = "ew-resize";
      break;
    case MouseCursor::kSizeNs:
      cursor_name = "ns-resize";
      break;
    case MouseCursor::kSizeNeSw:
      cursor_name = "nesw-resize";
      break;
    case MouseCursor::kSizeNwSe:
      cursor_name = "nwse-resize";
      break;
    case MouseCursor::kText:
      cursor_name = "text";
      break;
    case MouseCursor::kWait:
      cursor_name = "wait";
      break;
  }

  struct wl_cursor* cursor =
      wl_cursor_theme_get_cursor(cursor_theme_, cursor_name);
  if (!cursor) {
    cursor = wl_cursor_theme_get_cursor(cursor_theme_, "left_ptr");
  }
  if (cursor && cursor->image_count > 0) {
    struct wl_cursor_image* image = cursor->images[0];
    struct wl_buffer* buffer = wl_cursor_image_get_buffer(image);
    if (buffer) {
      wl_pointer_set_cursor(pointer_, pointer_serial_, cursor_surface_,
                            image->hotspot_x, image->hotspot_y);
      wl_surface_attach(cursor_surface_, buffer, 0, 0);
      wl_surface_damage(cursor_surface_, 0, 0, image->width, image->height);
      wl_surface_commit(cursor_surface_);
    }
  }
}

// Registry Callbacks
void WindowHandlerWayland::RegistryHandleGlobal(void* data,
                                                wl_registry* registry,
                                                uint32_t name,
                                                const char* interface,
                                                uint32_t /*version*/) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
    self->compositor_ = static_cast<wl_compositor*>(
        wl_registry_bind(registry, name, &wl_compositor_interface, 4));
  } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
    self->shm_ = static_cast<wl_shm*>(
        wl_registry_bind(registry, name, &wl_shm_interface, 1));
  } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
    self->seat_ = static_cast<wl_seat*>(
        wl_registry_bind(registry, name, &wl_seat_interface, 5));
    wl_seat_add_listener(self->seat_, &kSeatListener, self);
  } else if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    self->layer_shell_ = static_cast<zwlr_layer_shell_v1*>(
        wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1));
  }
}

void WindowHandlerWayland::RegistryHandleGlobalRemove(void* /*data*/,
                                                      wl_registry* /*registry*/,
                                                      uint32_t /*name*/) {}

// Layer Surface Callbacks
void WindowHandlerWayland::LayerSurfaceHandleConfigure(
    void* data, zwlr_layer_surface_v1* layer_surface, uint32_t serial,
    uint32_t width, uint32_t height) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

  if (width > 0 && height > 0) {
    if (self->x_res_ != static_cast<int32_t>(width) ||
        self->y_res_ != static_cast<int32_t>(height)) {
      self->x_res_ = static_cast<int32_t>(width);
      self->y_res_ = static_cast<int32_t>(height);
      self->client_rect_ = Rect(0, 0, self->x_res_, self->y_res_);
      self->HandleResize();
    }
  }
  self->configured_ = true;
}

void WindowHandlerWayland::LayerSurfaceHandleClosed(
    void* data, zwlr_layer_surface_v1* /*layer_surface*/) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  self->done_ = true;
}

// Seat Callbacks
void WindowHandlerWayland::SeatHandleCapabilities(void* data, wl_seat* seat,
                                                  uint32_t capabilities) {
  auto* self = static_cast<WindowHandlerWayland*>(data);

  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !self->pointer_) {
    self->pointer_ = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(self->pointer_, &kPointerListener, self);
  } else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && self->pointer_) {
    wl_pointer_destroy(self->pointer_);
    self->pointer_ = nullptr;
  }

  if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !self->keyboard_) {
    self->keyboard_ = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(self->keyboard_, &kKeyboardListener, self);
  } else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && self->keyboard_) {
    wl_keyboard_destroy(self->keyboard_);
    self->keyboard_ = nullptr;
  }
}

void WindowHandlerWayland::SeatHandleName(void* /*data*/, wl_seat* /*seat*/,
                                          const char* /*name*/) {}

// Pointer Callbacks
void WindowHandlerWayland::PointerHandleEnter(void* data,
                                              wl_pointer* /*pointer*/,
                                              uint32_t serial,
                                              wl_surface* /*surface*/,
                                              wl_fixed_t sx, wl_fixed_t sy) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  self->pointer_serial_ = serial;
  const int32_t x = wl_fixed_to_int(sx);
  const int32_t y = wl_fixed_to_int(sy);
  g_cursor_pos.x = x;
  g_cursor_pos.y = y;
  self->mouse_pos_ = Point(x, y);
  self->UpdateCursor();
  self->WindowProc(WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
}

void WindowHandlerWayland::PointerHandleLeave(void* data,
                                              wl_pointer* /*pointer*/,
                                              uint32_t serial,
                                              wl_surface* /*surface*/) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  self->pointer_serial_ = serial;
}

void WindowHandlerWayland::PointerHandleMotion(void* data,
                                               wl_pointer* /*pointer*/,
                                               uint32_t /*time*/, wl_fixed_t sx,
                                               wl_fixed_t sy) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  const int32_t x = wl_fixed_to_int(sx);
  const int32_t y = wl_fixed_to_int(sy);
  g_cursor_pos.x = x;
  g_cursor_pos.y = y;
  self->mouse_pos_ = Point(x, y);
  self->WindowProc(WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
}

void WindowHandlerWayland::PointerHandleButton(
    void* data, wl_pointer* /*pointer*/, uint32_t serial, uint32_t /*time*/,
    uint32_t button, uint32_t state) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  self->pointer_serial_ = serial;

  const int32_t x = self->mouse_pos_.x;
  const int32_t y = self->mouse_pos_.y;

  if (button == BTN_LEFT) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      self->left_down_pos_ = self->mouse_pos_;
      self->WindowProc(WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
    } else {
      self->WindowProc(WM_LBUTTONUP, 0, MAKELPARAM(x, y));
    }
  } else if (button == BTN_RIGHT) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      self->right_down_pos_ = self->mouse_pos_;
      self->WindowProc(WM_RBUTTONDOWN, 0, MAKELPARAM(x, y));
    } else {
      self->WindowProc(WM_RBUTTONUP, 0, MAKELPARAM(x, y));
    }
  } else if (button == BTN_MIDDLE) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      self->mid_down_pos_ = self->mouse_pos_;
      self->WindowProc(WM_MBUTTONDOWN, 0, MAKELPARAM(x, y));
    } else {
      self->WindowProc(WM_MBUTTONUP, 0, MAKELPARAM(x, y));
    }
  }
}

void WindowHandlerWayland::PointerHandleAxis(void* data,
                                             wl_pointer* /*pointer*/,
                                             uint32_t /*time*/, uint32_t axis,
                                             wl_fixed_t value) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    const int32_t delta = -wl_fixed_to_int(value) * 12;
    self->WindowProc(WM_MOUSEWHEEL, MAKEWPARAM(0, delta), 0);
  }
}

void WindowHandlerWayland::PointerHandleFrame(void* /*data*/,
                                              wl_pointer* /*pointer*/) {}

void WindowHandlerWayland::PointerHandleAxisSource(void* /*data*/,
                                                   wl_pointer* /*pointer*/,
                                                   uint32_t /*axis_source*/) {}

void WindowHandlerWayland::PointerHandleAxisStop(void* /*data*/,
                                                 wl_pointer* /*pointer*/,
                                                 uint32_t /*time*/,
                                                 uint32_t /*axis*/) {}

void WindowHandlerWayland::PointerHandleAxisDiscrete(void* /*data*/,
                                                     wl_pointer* /*pointer*/,
                                                     uint32_t /*axis*/,
                                                     int32_t /*discrete*/) {}

// Keyboard Callbacks
void WindowHandlerWayland::KeyboardHandleKeymap(void* data,
                                                wl_keyboard* /*keyboard*/,
                                                uint32_t format, int32_t fd,
                                                uint32_t size) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    char* map_shm =
        static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
    if (map_shm != MAP_FAILED) {
      if (self->xkb_keymap_) {
        xkb_keymap_unref(self->xkb_keymap_);
      }
      self->xkb_keymap_ = xkb_keymap_new_from_string(
          self->xkb_context_, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
          XKB_KEYMAP_COMPILE_NO_FLAGS);
      munmap(map_shm, size);

      if (self->xkb_state_) {
        xkb_state_unref(self->xkb_state_);
      }
      self->xkb_state_ = xkb_state_new(self->xkb_keymap_);
    }
  }
  close(fd);
}

void WindowHandlerWayland::KeyboardHandleEnter(void* /*data*/,
                                               wl_keyboard* /*keyboard*/,
                                               uint32_t /*serial*/,
                                               wl_surface* /*surface*/,
                                               wl_array* /*keys*/) {}

void WindowHandlerWayland::KeyboardHandleLeave(void* /*data*/,
                                               wl_keyboard* /*keyboard*/,
                                               uint32_t /*serial*/,
                                               wl_surface* /*surface*/) {}

void WindowHandlerWayland::KeyboardHandleKey(void* data,
                                             wl_keyboard* /*keyboard*/,
                                             uint32_t /*serial*/,
                                             uint32_t /*time*/, uint32_t key,
                                             uint32_t state) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  const xkb_keycode_t keycode = key + 8;
  const xkb_keysym_t sym =
      self->xkb_state_ ? xkb_state_key_get_one_sym(self->xkb_state_, keycode)
                       : 0;
  const uint32_t vk = XkbKeysymToVk(sym);

  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    if (vk != 0) {
      self->WindowProc(WM_KEYDOWN, vk, 0);
    }
    if (self->xkb_state_) {
      const uint32_t utf32 = xkb_state_key_get_utf32(self->xkb_state_, keycode);
      if (utf32 > 0 && utf32 < 0x10000) {
        self->WindowProc(WM_CHAR, utf32, 0);
      }
    }
  } else if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
    if (vk != 0) {
      self->WindowProc(WM_KEYUP, vk, 0);
    }
  }
}

void WindowHandlerWayland::KeyboardHandleModifiers(
    void* data, wl_keyboard* /*keyboard*/, uint32_t /*serial*/,
    uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
    uint32_t group) {
  auto* self = static_cast<WindowHandlerWayland*>(data);
  if (self->xkb_state_) {
    xkb_state_update_mask(self->xkb_state_, mods_depressed, mods_latched,
                          mods_locked, 0, 0, group);
  }
}

void WindowHandlerWayland::KeyboardHandleRepeatInfo(void* /*data*/,
                                                    wl_keyboard* /*keyboard*/,
                                                    int32_t /*rate*/,
                                                    int32_t /*delay*/) {}

}  // namespace renderer
