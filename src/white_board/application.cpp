module;

#include <windows.h>

#include <algorithm>
#include <clocale>
#include <cstdio>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/base/color.h"
#include "src/base/file_list.h"
#include "src/base/logger.h"
#include "src/base/ring_buffer.h"
#include "src/base/stream_reader.h"
#include "src/base/stream_writer.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/core2/enums.h"
#include "src/core2/window_handler.h"
#include "src/util/png_decompressor.h"

module whiteboard;

import :application;
import :box;
import :button;
import :gui_item;
import :label;
import :text_box;
import :window;
import math;
import xml;

using math::Point;
using math::Rect;

namespace gui {

Application::Application()
    : WindowHandlerWin(),
      draw_api_(std::make_unique<DrawAPI>()),
      frame_times_(std::make_unique<RingBuffer<int32_t, 60>>()),
      last_frame_time_(0),
      mouse_capture_item_(nullptr),
      mouse_item_(nullptr),
      skin_(std::make_unique<Skin>()),
      root_(nullptr),
      default_font_(nullptr),
      alt_(false),
      ctrl_(false),
      shift_(false),
      left_(false),
      middle_(false),
      right_(false),
      vsync_(true),
      click_repeater_mode_(MouseClickRepeatMode::WB_MCR_OFF) {
  // initialize default factory calls

  RegisterUIFactoryCallback("window", Window::Factory);
  RegisterUIFactoryCallback("button", Button::Factory);
  RegisterUIFactoryCallback("box", Box::Factory);
  RegisterUIFactoryCallback("label", Label::Factory);
  RegisterUIFactoryCallback("textbox", TextBox::Factory);
}

Application::~Application() {
  items_.clear();
  trash_.clear();
  fonts_.clear();
  skin_.reset();
  atlas_.reset();
  draw_api_.reset();
  layout_repository_.clear();
  root_.reset();
}

bool Application::SendMessageToItem(const Message& message, Widget* Target) {
  if (root_ && message.GetTarget() == root_->GetGuid()) {
    Target = root_.get();
  }

  if (!Target) {
    return false;
  }

  std::vector<Widget*> MessagePath;
  while (Target) {
    MessagePath.push_back(Target);
    Target = Target->GetParent();
  }

  for (int32_t x = MessagePath.size() - 1; x >= 0; x--) {
    if (MessagePath[x]->MessageProc(message)) return true;
  }

  return false;
}

void Application::ProcessMessage(Message& message) {
  if (message.GetTarget()) {
    Widget* Target = FindItemByGuid(message.GetTarget());
    SendMessageToItem(message, Target);
    return;
  }

  // handle messages created by mouse events
  if (message.IsMouseMessage()) {
    if (message.Get() == WBM_MOUSEMOVE) {
      mouse_pos_ = Point(message.GetPosition());
      UpdateMouseItem();
    }

    if (message.Get() == WBM_LEFTBUTTONDOWN) {
      left_down_pos_ = Point(message.GetPosition());
    }
    if (message.Get() == WBM_RIGHTBUTTONDOWN) {
      right_down_pos_ = Point(message.GetPosition());
    }
    if (message.Get() == WBM_MIDDLEBUTTONDOWN) {
      mid_down_pos_ = Point(message.GetPosition());
    }

    if (mouse_capture_item_) {
      // mouse messages are captured by this item, send
      // them directly there
      mouse_capture_item_->MessageProc(message);
      return;
    }

    Widget* mi = GetItemUnderMouse(mouse_pos_, message.Get());

    if (mi) {
      // handle focus change
      if (message.Get() == WBM_LEFTBUTTONDOWN ||
          message.Get() == WBM_MIDDLEBUTTONDOWN ||
          message.Get() == WBM_RIGHTBUTTONDOWN) {
        mi->SetFocus();
      }

      SendMessageToItem(message, mi);
    }

    return;
  }

  // handle messages aimed at the item in focus

  // top to bottom version : Widget* fi = GetRoot();
  // while (fi) {
  //   if (fi->MessageProc(message)) return;
  //   fi = fi->ChildInFocus;
  // }

  // bottom to top version:
  std::vector<Widget*> MessagePath;

  Widget* Target = GetRoot();
  while (Target) {
    MessagePath.push_back(Target);
    Target = Target->GetChildInFocus();
  }

  for (int32_t x = MessagePath.size() - 1; x >= 0; x--) {
    if (MessagePath[x]->MessageProc(message)) return;
  }
}

Widget* Application::GetItemUnderMouse(const Point& Point, MessageType w) {
  if (!root_) {
    return nullptr;
  }
  Rect r = root_->GetScreenRect();
  return root_->GetItemUnderMouse(Point, r, w);
}

void Application::HandleResize() {
  WindowHandlerWin::HandleResize();

  if (root_) {
    root_->SetPosition(Rect(0, 0, x_res_, y_res_));
  }
}

LRESULT Application::InjectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return WindowProc(uMsg, wParam, lParam);
}

void Application::UpdateControlKeyStates() {
  alt_ = (GetKeyState(VK_MENU) < 0) || (GetAsyncKeyState(VK_MENU) & 0x8000) ||
         (GetAsyncKeyState(VK_LMENU) & 0x8000) ||
         (GetAsyncKeyState(VK_RMENU) & 0x8000);
  ctrl_ = (GetKeyState(VK_CONTROL) < 0) ||
          (GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
          (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ||
          (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
  shift_ = (GetKeyState(VK_SHIFT) < 0) ||
           (GetAsyncKeyState(VK_SHIFT) & 0x8000) ||
           (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ||
           (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
}

int32_t Application::GetKeyboardState() {
  return (alt_ * WB_KBSTATE_ALT) | (ctrl_ * WB_KBSTATE_CTRL) |
         (shift_ * WB_KBSTATE_SHIFT);
}

int32_t Application::GetInitialKeyboardDelay() {
  int32_t setting = 0;
  if (!SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &setting, 0)) {
    return (1 + 1) * 250;  // 1 by default
  }
  return (setting + 1) * 250;
}

int32_t Application::GetKeyboardRepeatTime() {
  int32_t setting = 0;
  if (!SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &setting, 0)) {
    return 400 - (31 * 12);  // 31 by default
  }
  return 400 - (setting * 12);
}

LRESULT Application::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_ACTIVATE:
      if (LOWORD(wParam)) UpdateControlKeyStates();
      break;
    case WM_MOUSEMOVE: {
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_MOUSEMOVE, 0, ap.x, ap.y));
    } break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK: {
      left_ = true;

      ::SetCapture(window_handle_);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_LEFTBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_LBUTTONDBLCLK) {
        Send(Message(this, WBM_LEFTBUTTONDBLCLK, 0, ap.x, ap.y));
      }

      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_LEFT;
      next_repeated_click_time_ =
          static_cast<int64_t>(global_timer.GetTime()) +
          static_cast<int64_t>(GetInitialKeyboardDelay());
    } break;
    case WM_LBUTTONUP: {
      left_ = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_LEFTBUTTONUP, 0, ap.x, ap.y));
      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_OFF;
    } break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK: {
      right_ = true;
      ::SetCapture(window_handle_);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_RIGHTBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_RBUTTONDBLCLK) {
        Send(Message(this, WBM_RIGHTBUTTONDBLCLK, 0, ap.x, ap.y));
      }

      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_RIGHT;
      next_repeated_click_time_ =
          static_cast<int64_t>(global_timer.GetTime()) +
          static_cast<int64_t>(GetInitialKeyboardDelay());
    } break;
    case WM_RBUTTONUP: {
      right_ = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_RIGHTBUTTONUP, 0, ap.x, ap.y));
      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_OFF;
    } break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK: {
      middle_ = true;
      ::SetCapture(window_handle_);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_MIDDLEBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_MBUTTONDBLCLK) {
        Send(Message(this, WBM_MIDDLEBUTTONDBLCLK, 0, ap.x, ap.y));
      }

      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_MIDDLE;
      next_repeated_click_time_ =
          static_cast<int64_t>(global_timer.GetTime()) +
          static_cast<int64_t>(GetInitialKeyboardDelay());
    } break;
    case WM_MBUTTONUP: {
      middle_ = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);
      Send(Message(this, WBM_MIDDLEBUTTONUP, 0, ap.x, ap.y));
      click_repeater_mode_ = MouseClickRepeatMode::WB_MCR_OFF;
    } break;
    case WM_MOUSEWHEEL: {
      Send(Message(this, WBM_MOUSEWHEEL, 0,
                   GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA));
    } break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (wParam == VK_CONTROL || wParam == VK_LCONTROL ||
          wParam == VK_RCONTROL) {
        ctrl_ = true;
      }
      if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        shift_ = true;
      }
      if (wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU) {
        alt_ = true;
      }

      Send(Message(
          this, WBM_KEYDOWN, 0,
          Message::keyboard{static_cast<int32_t>(wParam), GetKeyboardState()}));
      break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (wParam == VK_CONTROL || wParam == VK_LCONTROL ||
          wParam == VK_RCONTROL) {
        ctrl_ = false;
      }
      if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT) {
        shift_ = false;
      }
      if (wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU) {
        alt_ = false;
      }

      switch (wParam) {
        case VK_SNAPSHOT:
          TakeScreenshot();
          break;
      }

      Send(Message(
          this, WBM_KEYUP, 0,
          Message::keyboard{static_cast<int32_t>(wParam), GetKeyboardState()}));
      break;
    case WM_SYSCHAR:
    case WM_CHAR:
      Send(Message(
          this, WBM_CHAR, 0,
          Message::keyboard{static_cast<int32_t>(wParam), GetKeyboardState()}));
      break;
    default:
      break;
  }

  return WindowHandlerWin::WindowProc(uMsg, wParam, lParam);
}

bool Application::Initialize() {
  atlas_ = std::make_unique<Atlas>(2048, 2048);
  if (!atlas_->InitializeTexture(device_.get())) {
    Log_Err("[gui] Error creating UI Texture Atlas");
    return false;
  }

  if (!draw_api_->Initialize(this, device_.get(), atlas_.get())) return false;

  root_ = Root::Create(Rect(0, 0, x_res_, y_res_));
  root_->SetApplication(this);

  setlocale(LC_NUMERIC, "C");

  return true;
}

bool Application::Initialize(const renderer::WindowParameters& WindowParams) {
  if (!WindowHandlerWin::Initialize(WindowParams)) return false;
  return Initialize();
}

Widget* Application::GetRoot() { return root_.get(); }

Widget* Application::GetFocusItem() {
  Widget* fi = root_.get();
  while (fi) {
    if (!fi->child_in_focus_) {
      return fi;
    }
    fi = fi->child_in_focus_;
  }
  return root_.get();
}

bool Application::HandleMessages() {
  // as this function is called once every frame, it's the best place to update
  // the timer as well
  global_timer.Update();

  // mouse click repeater
  if (click_repeater_mode_ != MouseClickRepeatMode::WB_MCR_OFF) {
    if (global_timer.GetTime() > next_repeated_click_time_) {
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(window_handle_, &ap);

      switch (click_repeater_mode_) {
        case MouseClickRepeatMode::WB_MCR_LEFT:
          Send(Message(this, WBM_LEFTBUTTONREPEAT, 0, ap.x, ap.y));
          break;
        case MouseClickRepeatMode::WB_MCR_RIGHT:
          Send(Message(this, WBM_RIGHTBUTTONREPEAT, 0, ap.x, ap.y));
          break;
        case MouseClickRepeatMode::WB_MCR_MIDDLE:
          Send(Message(this, WBM_MIDDLEBUTTONREPEAT, 0, ap.x, ap.y));
          break;
      }
      next_repeated_click_time_ += GetKeyboardRepeatTime();
    }
  }

  if (!WindowHandlerWin::HandleMessages()) return false;

  // handle gui messages here
  {
    std::scoped_lock l(message_buffer_mutex_);
    // This stuff is very brittle. It even depends on Messages added during this
    // loop to be executed at the end of this loop. So even changing the for
    // loop to a range loop causes bugs.
    for (size_t i = 0; i < message_buffer_.size(); i++) {
      ProcessMessage(message_buffer_[i]);
    }
    message_buffer_.clear();
  }
  return true;
}

void Application::RegisterItem(Widget* Item) {
  items_[Item->GetGuid()] = Item;

  if (Item->GetScreenRect().Contains(mouse_pos_)) UpdateMouseItem();
}

void Application::UnRegisterItem(Widget* Item) {
  std::scoped_lock l(trash_mutex_);
  auto it = std::find_if(
      trash_.begin(), trash_.end(),
      [Item](const std::unique_ptr<Widget>& i) { return i.get() == Item; });
  if (it != trash_.end()) {
    trash_.erase(it);
  }

  if (mouse_capture_item_ == Item) ReleaseCapture();
  items_.erase(Item->GetGuid());

  if (Item->GetScreenRect().Contains(mouse_pos_)) UpdateMouseItem();
}

void Application::SetDone(bool d) { done_ = d; }

void Application::Display() { Display(draw_api_.get()); }

void Application::Display(DrawAPI* API) {
  CleanTrash();

  FinalizeMouseCursor();
  SelectMouseCursor(renderer::MouseCursor::kArrow);

  draw_api_->SetUIRenderState();
  device_->Clear(true, true, clear_color_);
  device_->BeginScene();

  draw_api_->SetOffset(root_->GetScreenRect().TopLeft());
  draw_api_->SetParentCropRect(root_->GetScreenRect());
  draw_api_->SetCropRect(root_->GetScreenRect());

  atlas_->ClearImageUsageflags();

  root_->DrawTree(API);

  draw_api_->RenderDisplayList();
  device_->EndScene();
  device_->Flip(vsync_);

  // Log_Dbg("End Frame");

  // this is a convenient place to store this:
  logger.ResetEntryCounter();

  // update frame time
  const int32_t frametime = global_timer.GetTime();
  frame_times_->Add(frametime - last_frame_time_);
  last_frame_time_ = frametime;
}

Widget* Application::FindItemByGuid(WBGUID Guid, const TCHAR* type) const {
  if (auto it = items_.find(Guid); it != items_.end()) {
    Widget* i = it->second;
    return (!type || i->InstanceOf(type)) ? i : nullptr;
  }
  return nullptr;
}

void Application::Send(const Message& message) {
  std::scoped_lock l(message_buffer_mutex_);
  message_buffer_.emplace_back(message);
}

Widget* Application::SetCapture(Widget* Capturer) {
  Widget* old = mouse_capture_item_;
  mouse_capture_item_ = Capturer;
  return old;
}

bool Application::ReleaseCapture() {
  mouse_capture_item_ = nullptr;
  return true;
}

void Application::UpdateMouseItem() {
  Widget* MouseItemOld = mouse_item_;
  mouse_item_ = GetItemUnderMouse(mouse_pos_, WBM_MOUSEMOVE);
  if (mouse_item_ != MouseItemOld) {
    // call onmouseleave for the old items
    if (mouse_item_) {
      Widget* olditem = MouseItemOld;
      while (olditem) {
        // find the old item in the tree of the new. if not found the mouse left
        // the item
        if (!mouse_item_->FindItemInParentTree(olditem))
          olditem->OnMouseLeave();
        olditem = olditem->GetParent();
      }
    }

    // call onmouseenter for the new items
    if (MouseItemOld) {
      Widget* newitem = mouse_item_;
      while (newitem) {
        // find the new item in the tree of the old. if not found the mouse
        // entered the item
        if (!MouseItemOld->FindItemInParentTree(newitem)) {
          newitem->OnMouseEnter();
        }
        newitem = newitem->GetParent();
      }
    }
  }
}

void Application::CleanTrash() {
  std::scoped_lock l(trash_mutex_);
  for (auto i = trash_.size(); i > 0; i--) {
    auto& t = trash_[i - 1];
    t.reset();
  }
  trash_.clear();
}

Widget* Application::GetMouseItem() { return mouse_item_; }

Widget* Application::GetMouseCaptureItem() { return mouse_capture_item_; }

bool Application::InitFont(std::string_view FontName, FontDescription* Font) {
  if (!Font) return false;

  auto f = std::make_unique<gui::Font>(atlas_.get());

  if (!f->Initialize(Font)) {
    return false;
  }

  if (fonts_.empty()) default_font_ = f.get();

  fonts_[std::string(FontName)] = std::move(f);

  return true;
}

Font* Application::GetFont(std::string_view FontName) {
  auto f = fonts_.find(std::string(FontName));
  if (f != fonts_.end()) {
    return f->second.get();
  }
  Log_Warn("[gui] Font {:s} not found, falling back to default font", FontName);
  return default_font_;
}

Font* Application::GetDefaultFont() { return default_font_; }

bool Application::SetDefaultFont(std::string_view FontName) {
  Font* f = GetFont(FontName);
  if (!f) return false;
  default_font_ = f;
  return true;
}

void Application::AddToTrash(std::unique_ptr<Widget>&& item) {
  std::scoped_lock l(trash_mutex_);
  auto it = std::find(trash_.begin(), trash_.end(), item);
  if (it == trash_.end()) {
    trash_.emplace_back(std::move(item));
  }
}

bool Application::LoadXMLLayout(std::string_view XML) {
  auto doc = std::make_unique<XMLDocument>();
  if (!doc->LoadFromString(XML)) {
    Log_Err("[gui] Error loading XML Layout: parsing failed");
    return false;
  }

  XMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) {
    Log_Err("[gui] Error loading XML Layout: 'guidescriptor' member missing");
    return false;
  }

  root = root.GetChild("guidescriptor");

  if (!root.GetChildCount("gui")) {
    Log_Err("[gui] Error loading XML Layout: 'gui' member missing");
    return false;
  }

  XMLNode gui = root.GetChild("gui");
  if (!gui.HasAttribute("id")) {
    Log_Err("[gui] Error loading XML Layout: 'id' member missing from 'gui'");
    return false;
  }

  auto id = gui.GetAttribute("id");

  layout_repository_[id] = std::move(doc);

  return true;
}

bool Application::LoadXMLLayoutFromFile(std::string_view FileName) {
  StreamReaderMemory f;
  if (!f.Open(FileName)) {
    Log_Err("[gui] Error loading XML Layout: file '{:s}' not found", FileName);
    return false;
  }

  const std::string_view s(reinterpret_cast<const char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  return LoadXMLLayout(s);
}

bool Application::GenerateGUI(Widget* Root, std::string_view Layout) {
  std::string l(Layout);
  if (layout_repository_.find(l) == layout_repository_.end()) {
    Log_Err("[gui] Error generating UI: layout '{:s}' not loaded", l);
    return false;
  }

  const bool b = GenerateGUIFromXML(Root, layout_repository_[l].get());
  if (!b) return false;

  style_manager_.ApplyStyles(Root);
  Message m = Root->BuildPositionMessage(Root->GetPosition(), true);
  Root->MessageProc(m);
  return true;
}

bool Application::GenerateGUITemplate(Widget* Root, std::string_view Layout,
                                      std::string_view TemplateID) {
  std::string l(Layout);
  if (layout_repository_.find(l) == layout_repository_.end()) {
    Log_Err("[gui] Error generating UI template: layout '{:s}' not loaded", l);
    return false;
  }

  const bool b =
      GenerateGUITemplateFromXML(Root, layout_repository_[l].get(), TemplateID);
  if (!b) return false;

  style_manager_.ApplyStyles(Root);
  Message m = Root->BuildPositionMessage(Root->GetPosition(), true);
  Root->MessageProc(m);

  return true;
}

void Application::ApplyStyle(Widget* Target) {
  style_manager_.ApplyStyles(Target);
  Message m = Target->BuildPositionMessage(Target->GetPosition(), true);
  Target->MessageProc(m);
}

void Application::ReApplyStyle() {
  Widget* Root = GetRoot();
  style_manager_.ApplyStyles(Root);
  Message m = Root->BuildPositionMessage(Root->GetPosition(), true);
  Root->MessageProc(m);
}

bool Application::LoadCSS(std::string_view CSS, bool ResetStyleManager) {
  if (ResetStyleManager) style_manager_.Reset();
  return style_manager_.ParseStyleData(CSS);
}

bool Application::LoadCSSFromFile(std::string_view FileName,
                                  bool ResetStyleManager) {
  StreamReaderMemory f;
  if (!f.Open(FileName)) {
    Log_Err("[gui] Error loading CSS: file '{:s}' not found", FileName);
    return false;
  }

  const std::string_view s(reinterpret_cast<const char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  const bool b = LoadCSS(s, ResetStyleManager);

  return b;
}

Atlas* Application::GetAtlas() { return atlas_.get(); }

Skin* Application::GetSkin() { return skin_.get(); }

bool Application::LoadSkin(std::string_view XML,
                           std::vector<int>& enabledGlyphs) {
  XMLDocument doc;
  if (!doc.LoadFromString(XML)) return false;
  if (!doc.GetDocumentNode().GetChildCount("whiteboardskin")) return false;

  XMLNode r = doc.GetDocumentNode().GetChild("whiteboardskin");

  for (int32_t x = 0; x < r.GetChildCount("image"); x++) {
    XMLNode n = r.GetChild("image", x);
    auto img = n.GetAttributeAsString("image");

    auto data = B64Decode(img);

    std::unique_ptr<uint8_t[]> Image;
    int32_t XRes = 0, YRes = 0;
    if (DecompressPNG((unsigned char*)data.c_str(), data.size(), &Image, &XRes,
                      &YRes)) {
      ARGBtoABGR(Image.get(), XRes, YRes);
      ClearZeroAlpha(Image.get(), XRes, YRes);

      for (int32_t y = 0; y < n.GetChildCount("element"); y++) {
        XMLNode e = n.GetChild("element", y);

        Rect r2;
        e.GetAttributeAsInteger("x1", &r2.x1);
        e.GetAttributeAsInteger("y1", &r2.y1);
        e.GetAttributeAsInteger("x2", &r2.x2);
        e.GetAttributeAsInteger("y2", &r2.y2);

        Point b;
        e.GetAttributeAsInteger("x-behavior", &b.x);
        e.GetAttributeAsInteger("y-behavior", &b.y);

        const AtlasHandle h = atlas_->AddImage(Image.get(), XRes, YRes, r2);
        skin_->AddElement(e.GetAttributeAsString("name"), h,
                          static_cast<SkinElementBehavior>(b.x),
                          static_cast<SkinElementBehavior>(b.y));
      }
    }
  }

  for (int32_t x = 0; x < r.GetChildCount("mosaic"); x++) {
    XMLNode m = r.GetChild("mosaic", x);

    Rect r2;
    m.GetAttributeAsInteger("overshootx1", &r2.x1);
    m.GetAttributeAsInteger("overshooty1", &r2.y1);
    m.GetAttributeAsInteger("overshootx2", &r2.x2);
    m.GetAttributeAsInteger("overshooty2", &r2.y2);

    skin_->AddMosaic(m.GetAttributeAsString("name"),
                     m.GetAttributeAsString("description"), r2.x1, r2.y1, r2.x2,
                     r2.y2);
  }

  for (int32_t x = 0; x < r.GetChildCount("font"); x++) {
    XMLNode n = r.GetChild("font", x);

    auto Name = n.GetAttributeAsString("name");

    auto img = n.GetAttributeAsString("image");
    auto bin = n.GetAttributeAsString("binary");

    const int32_t Sizeimg = 0;
    const int32_t Sizebin = 0;
    auto dataimg = B64Decode(img);
    auto databin = B64Decode(bin);

    int32_t XRes = 0, YRes = 0;
    std::unique_ptr<uint8_t[]> Image;
    if (DecompressPNG((unsigned char*)dataimg.c_str(), dataimg.size(), &Image,
                      &XRes, &YRes)) {
      ARGBtoABGR(Image.get(), XRes, YRes);
      auto fd = std::make_unique<FontDescription>();
      if (fd->LoadBMFontBinary((unsigned char*)databin.c_str(), databin.size(),
                               Image.get(), XRes, YRes, &enabledGlyphs)) {
        InitFont(Name, fd.get());
      } else if (fd->LoadBMFontText((unsigned char*)databin.c_str(),
                                    databin.size(), Image.get(), XRes, YRes,
                                    &enabledGlyphs)) {
        InitFont(Name, fd.get());
      }

      fd.reset();
    }
  }

  return true;
}

bool Application::LoadSkinFromFile(std::string_view FileName,
                                   std::vector<int>& enabledGlyphs) {
  StreamReaderMemory f;
  if (!f.Open(FileName)) {
    Log_Err("[gui] Error loading Skin: file '{:s}' not found", FileName);
    return false;
  }

  const std::string_view s(reinterpret_cast<const char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  const bool b = LoadSkin(s, enabledGlyphs);
  if (b) Log_Nfo("[gui] Successfully loaded Skin '{:s}'", FileName);

  return b;
}

bool Application::GenerateGUIFromXML(Widget* Root, XMLDocument* doc) {
  XMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) return false;

  root = root.GetChild("guidescriptor");

  if (!root.GetChildCount("gui")) return false;

  XMLNode gui = root.GetChild("gui");

  return ProcessGUIXML(Root, gui);
}

bool Application::GenerateGUITemplateFromXML(Widget* Root, XMLDocument* doc,
                                             std::string_view TemplateID) {
  XMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) return false;

  root = root.GetChild("guidescriptor");

  for (int32_t x = 0; x < root.GetChildCount("guitemplate"); x++) {
    XMLNode t = root.GetChild("guitemplate", x);

    if (t.HasAttribute("id")) {
      if (t.GetAttributeAsString("id") == TemplateID) {
        if (t.HasAttribute("class")) {
          auto a = Split(t.GetAttribute("class"), " ");
          for (const auto& s : a) {
            if (s.size() > 1) Root->AddClass(s);
          }
        }

        return ProcessGUIXML(Root, t);
      }
    }
  }

  return false;
}

bool Application::ProcessGUIXML(Widget* Root, const XMLNode& node) {
  Rect Pos(5, 5, 25, 25);

  bool b = true;
  for (int i = 0; i < node.GetChildCount(); i++) {
    b &= GenerateGUIFromXMLNode(Root, node.GetChild(i), &Pos);
    Pos =
        Rect(Pos.BottomLeft() + Point(0, 2), Pos.BottomLeft() + Point(20, 22));
  }
  return b;
}

bool Application::GenerateGUIFromXMLNode(Widget* Root, const XMLNode& node,
                                         Rect* Pos) {
  Widget* NewItem = GenerateUIItem(Root, node, *Pos);
  if (!NewItem) return false;

  if (node.HasAttribute("pos")) {
    const auto& pos = node.GetAttribute("pos");
    if (std::count(pos.begin(), pos.end(), ',') == 3) {
      std::sscanf(node.GetAttribute("pos").c_str(), "%d,%d,%d,%d", &Pos->x1,
                  &Pos->y1, &Pos->x2, &Pos->y2);
    } else {
      uint32_t x = 0, y = 0;
      std::sscanf(node.GetAttribute("pos").c_str(), "%d,%d", &x, &y);
      Pos->MoveTo(x, y);
    }
    NewItem->SetPosition(*Pos);
  }

  if (node.HasAttribute("size")) {
    uint32_t x = 0, y = 0;
    std::sscanf(node.GetAttribute("size").c_str(), "%d,%d", &x, &y);
    Pos->SetSize(x, y);
    NewItem->SetPosition(*Pos);
  }

  if (node.HasAttribute("id")) NewItem->SetID(node.GetAttribute("id"));

  if (node.HasAttribute("class")) {
    auto a = Split(node.GetAttribute("class"), " ");
    for (const auto& s : a) {
      if (s.size() > 1) NewItem->AddClass(s);
    }
  }

  return ProcessGUIXML(NewItem, node);
}

Widget* Application::GenerateUIItem(Widget* Root, const XMLNode& node,
                                    const Rect& Pos) {
  if (factory_callbacks_.find(node.GetNodeName()) != factory_callbacks_.end()) {
    return factory_callbacks_[node.GetNodeName()](Root, node, Pos);
  }

  Log_Err("[xml2gui] Unknown tag: '{:s}'", node.GetNodeName());
  return nullptr;
}

void Application::RegisterUIFactoryCallback(std::string_view ElementName,
                                            FactoryCallback FactoryCallback) {
  factory_callbacks_[std::string(ElementName)] = FactoryCallback;
}

void Application::TakeScreenshot() {
  CreateDirectory("Screenshots", nullptr);

  int32_t maxcnt = 0;
  {
    auto s = screen_shot_name_ + "_*.png";
    FileList fl(s, "Screenshots");

    for (auto& File : fl.Files) {
      if (File.FileName.find(screen_shot_name_) == std::string::npos) {
        auto s2 = screen_shot_name_ + "_%d";

        int32_t no = -1;
        sscanf(File.FileName.c_str(), s2.c_str(), &no);
        maxcnt = std::max(maxcnt, no);
      }
    }
  }

  draw_api_->FlushDrawBuffer();

  auto b = draw_api_->GetDevice()->CreateBlendState();
  b->SetBlendEnable(0, true);
  b->SetIndependentBlend(true);
  b->SetSrcBlend(0, renderer::BlendFactor::kZero);
  b->SetDestBlend(0, renderer::BlendFactor::kOne);
  b->SetSrcBlendAlpha(0, renderer::BlendFactor::kOne);
  b->SetDestBlendAlpha(0, renderer::BlendFactor::kZero);
  draw_api_->GetDevice()->SetRenderState(b.get());

  draw_api_->SetCropRect(Rect(0, 0, x_res_, y_res_));
  draw_api_->DrawRect(Rect(0, 0, x_res_, y_res_), Color{0xff000000});
  draw_api_->FlushDrawBuffer();

  auto fname = std::format("Screenshots\\{:s}_{:04d}.png", screen_shot_name_,
                           maxcnt + 1);
  draw_api_->GetDevice()->TakeScreenShot(fname);

  draw_api_->SetUIRenderState();
}

float Application::GetFrameRate() {
  int32_t FrameTimeAcc = 0;
  int32_t FrameCount = 0;
  for (int32_t x = 0; x < 60; x++) {
    if (frame_times_->NumItems() < x) break;
    FrameTimeAcc += (*frame_times_)[frame_times_->NumItems() - 1 - x];
    FrameCount++;
  }

  if (!FrameCount) return 0;
  if (!FrameTimeAcc) return 9999;
  return 1000.0f / (FrameTimeAcc / static_cast<float>(FrameCount));
}

}  // namespace gui
