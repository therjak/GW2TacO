#include "src/white_board/application.h"

#include <tchar.h>

#include <clocale>
#include <cstdio>
#include <format>

#include "src/base/file_list.h"
#include "src/base/logger.h"
#include "src/base/stream_reader.h"
#include "src/base/stream_writer.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/util/png_decompressor.h"
#include "src/white_board/box.h"
#include "src/white_board/button.h"
#include "src/white_board/label.h"
#include "src/white_board/text_box.h"
#include "src/white_board/window.h"

using math::CPoint;
using math::CRect;

CWBApplication::CWBApplication() : CCoreWindowHandlerWin() {
  Root = nullptr;
  MouseCaptureItem = nullptr;
  MouseItem = nullptr;
  DrawAPI = std::make_unique<CWBDrawAPI>();
  Skin = std::make_unique<CWBSkin>();
  DefaultFont = nullptr;
  Alt = Ctrl = Shift = Left = Middle = Right = false;
  Vsync = true;
  FrameTimes = std::make_unique<CRingBuffer<int32_t>>(60);
  LastFrameTime = 0;

  // initialize default factory calls

  RegisterUIFactoryCallback("window", CWBWindow::Factory);
  RegisterUIFactoryCallback("button", CWBButton::Factory);
  RegisterUIFactoryCallback("box", CWBBox::Factory);
  RegisterUIFactoryCallback("label", CWBLabel::Factory);
  RegisterUIFactoryCallback("textbox", CWBTextBox::Factory);
}

CWBApplication::~CWBApplication() {
  Items.clear();
  Trash.clear();
  Fonts.clear();
  Skin.reset();
  Atlas.reset();
  DrawAPI.reset();
  LayoutRepository.clear();
}

bool CWBApplication::SendMessageToItem(const CWBMessage& Message,
                                       CWBItem* Target) {
  if (Root && Message.GetTarget() == Root->GetGuid()) {
    Target = Root.get();
  }

  if (!Target) {
    // LOG(LOG_WARNING,"[gui] Message target item %d not found. Message type:
    // %d",Message.GetTarget(),Message.GetMessage());
    return false;
  }

  std::vector<CWBItem*> MessagePath;
  while (Target) {
    MessagePath.push_back(Target);
    Target = Target->GetParent();
  }

  for (int32_t x = MessagePath.size() - 1; x >= 0; x--)
    if (MessagePath[x]->MessageProc(Message)) return true;

  return false;
}

void CWBApplication::ProcessMessage(CWBMessage& Message) {
  if (Message.GetTarget()) {
    CWBItem* Target = FindItemByGuid(Message.GetTarget());
    SendMessageToItem(Message, Target);
    return;
  }

  // handle messages created by mouse events
  if (Message.IsMouseMessage()) {
    if (Message.GetMessage() == WBM_MOUSEMOVE) {
      MousePos = CPoint(Message.Position);
      UpdateMouseItem();
    }

    if (Message.GetMessage() == WBM_LEFTBUTTONDOWN)
      LeftDownPos = CPoint(Message.Position);
    if (Message.GetMessage() == WBM_RIGHTBUTTONDOWN)
      RightDownPos = CPoint(Message.Position);
    if (Message.GetMessage() == WBM_MIDDLEBUTTONDOWN)
      MidDownPos = CPoint(Message.Position);

    if (MouseCaptureItem)  // mouse messages are captured by this item, send
                           // them directly there
    {
      MouseCaptureItem->MessageProc(Message);
      return;
    }

    CWBItem* mi = GetItemUnderMouse(MousePos, Message.GetMessage());

    if (mi) {
      // handle focus change
      if (Message.GetMessage() == WBM_LEFTBUTTONDOWN ||
          Message.GetMessage() == WBM_MIDDLEBUTTONDOWN ||
          Message.GetMessage() == WBM_RIGHTBUTTONDOWN)
        mi->SetFocus();

      SendMessageToItem(Message, mi);
    }

    return;
  }

  // handle messages aimed at the item in focus

  // top to bottom version:
  // CWBItem *fi=GetRoot();
  // while (fi)
  //{
  //	if (fi->MessageProc(Message)) return;
  //	fi=fi->ChildInFocus;
  //}

  // bottom to top version:
  std::vector<CWBItem*> MessagePath;

  CWBItem* Target = GetRoot();
  while (Target) {
    MessagePath.push_back(Target);
    Target = Target->GetChildInFocus();
  }

  for (int32_t x = MessagePath.size() - 1; x >= 0; x--)
    if (MessagePath[x]->MessageProc(Message)) return;
}

CWBItem* CWBApplication::GetItemUnderMouse(CPoint& Point, WBMESSAGE w) {
  CRect r = Root->GetScreenRect();
  return Root->GetItemUnderMouse(Point, r, w);
}

void CWBApplication::HandleResize() {
  CCoreWindowHandlerWin::HandleResize();

  if (Root) Root->SetPosition(CRect(0, 0, XRes, YRes));
}

LRESULT CWBApplication::InjectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  return WindowProc(uMsg, wParam, lParam);
}

void CWBApplication::UpdateControlKeyStates() {
  Alt = (GetKeyState(VK_MENU) < 0) || (GetAsyncKeyState(VK_MENU) & 0x8000) ||
        (GetAsyncKeyState(VK_LMENU) & 0x8000) ||
        (GetAsyncKeyState(VK_RMENU) & 0x8000);
  Ctrl = (GetKeyState(VK_CONTROL) < 0) ||
         (GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
         (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ||
         (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
  Shift = (GetKeyState(VK_SHIFT) < 0) ||
          (GetAsyncKeyState(VK_SHIFT) & 0x8000) ||
          (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ||
          (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
}

int32_t CWBApplication::GetKeyboardState() {
  return (Alt * WB_KBSTATE_ALT) | (Ctrl * WB_KBSTATE_CTRL) |
         (Shift * WB_KBSTATE_SHIFT);
}

int32_t CWBApplication::GetInitialKeyboardDelay() {
  int32_t setting = 0;
  if (!SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &setting, 0))
    return (1 + 1) * 250;  // 1 by default
  return (setting + 1) * 250;
}

int32_t CWBApplication::GetKeyboardRepeatTime() {
  int32_t setting = 0;
  if (!SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &setting, 0))
    return 400 - (31 * 12);  // 31 by default
  return 400 - (setting * 12);
}

LRESULT CWBApplication::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // LOG(LOG_DEBUG,"[gui] WM_%d %d %d",uMsg,wParam,lParam);

  switch (uMsg) {
    case WM_ACTIVATE:
      if (LOWORD(wParam)) UpdateControlKeyStates();
      break;
    case WM_MOUSEMOVE: {
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_MOUSEMOVE, 0, ap.x, ap.y));
    } break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK: {
      Left = true;

      ::SetCapture(hWnd);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_LEFTBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_LBUTTONDBLCLK)
        SendMessage(CWBMessage(this, WBM_LEFTBUTTONDBLCLK, 0, ap.x, ap.y));

      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_LEFT;
      NextRepeatedClickTime =
          int64_t(globalTimer.GetTime()) + int64_t(GetInitialKeyboardDelay());
    } break;
    case WM_LBUTTONUP: {
      Left = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_LEFTBUTTONUP, 0, ap.x, ap.y));
      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_OFF;
    } break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK: {
      Right = true;
      ::SetCapture(hWnd);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_RIGHTBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_RBUTTONDBLCLK)
        SendMessage(CWBMessage(this, WBM_RIGHTBUTTONDBLCLK, 0, ap.x, ap.y));

      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_RIGHT;
      NextRepeatedClickTime =
          int64_t(globalTimer.GetTime()) + int64_t(GetInitialKeyboardDelay());
    } break;
    case WM_RBUTTONUP: {
      Right = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_RIGHTBUTTONUP, 0, ap.x, ap.y));
      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_OFF;
    } break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK: {
      Middle = true;
      ::SetCapture(hWnd);
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_MIDDLEBUTTONDOWN, 0, ap.x, ap.y));

      if (uMsg == WM_MBUTTONDBLCLK)
        SendMessage(CWBMessage(this, WBM_MIDDLEBUTTONDBLCLK, 0, ap.x, ap.y));

      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_MIDDLE;
      NextRepeatedClickTime =
          int64_t(globalTimer.GetTime()) + int64_t(GetInitialKeyboardDelay());
    } break;
    case WM_MBUTTONUP: {
      Middle = false;
      ::ReleaseCapture();
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);
      SendMessage(CWBMessage(this, WBM_MIDDLEBUTTONUP, 0, ap.x, ap.y));
      ClickRepeaterMode = WBMOUSECLICKREPEATMODE::WB_MCR_OFF;
    } break;
    case WM_MOUSEWHEEL: {
      SendMessage(CWBMessage(this, WBM_MOUSEWHEEL, 0,
                             GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA));
    } break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      // LOG_ERR( "[wndproc] WM_SYSKEYDOWN %d %d", wParam, lParam );

      if (wParam == VK_CONTROL || wParam == VK_LCONTROL ||
          wParam == VK_RCONTROL)
        Ctrl = true;
      if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT)
        Shift = true;
      if (wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU)
        Alt = true;

      // UpdateControlKeyStates();
      // LOG_ERR( "[wndproc] WM_KEYDOWN/WM_SYSKEYDOWN: %d %d %d %d %d %d", uMsg,
      // wParam, lParam, Alt, Ctrl, Shift );
      SendMessage(CWBMessage(this, WBM_KEYDOWN, 0, wParam, GetKeyboardState()));
      break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      // LOG_DBG("[app] Keyup: %d",wParam);
      if (wParam == VK_CONTROL || wParam == VK_LCONTROL ||
          wParam == VK_RCONTROL)
        Ctrl = false;
      if (wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT)
        Shift = false;
      if (wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU)
        Alt = false;

      // LOG_ERR( "[wndproc] WM_KEYUP/WM_SYSKEYUP: %d %d %d", uMsg, wParam,
      // lParam );
      switch (wParam) {
        case VK_SNAPSHOT:
          TakeScreenshot();
          break;
      }

      // UpdateControlKeyStates();
      SendMessage(CWBMessage(this, WBM_KEYUP, 0, wParam, GetKeyboardState()));
      break;
    case WM_SYSCHAR:
    case WM_CHAR:
      // LOG_DBG("[app] Char: %d",wParam);
      // LOG_ERR( "[wndproc] WM_CHAR/WM_SYSCHAR %d %d", uMsg, wParam, lParam );
      // UpdateControlKeyStates();
      SendMessage(CWBMessage(this, WBM_CHAR, 0, wParam, GetKeyboardState()));
      break;
    default:
      break;
  }

  return CCoreWindowHandlerWin::WindowProc(uMsg, wParam, lParam);
}

bool CWBApplication::Initialize() {
  Atlas = std::make_unique<CAtlas>(2048, 2048);
  if (!Atlas->InitializeTexture(Device.get())) {
    LOG_ERR("[gui] Error creating UI Texture Atlas");
    return false;
  }

  if (!DrawAPI->Initialize(this, Device.get(), Atlas.get())) return false;

  Root = CWBRoot::Create(CRect(0, 0, XRes, YRes));
  Root->SetApplication(this);

  setlocale(LC_NUMERIC, "C");

  return true;
}

bool CWBApplication::Initialize(const CCoreWindowParameters& WindowParams) {
  if (!CCoreWindowHandlerWin::Initialize(WindowParams)) return false;
  return Initialize();
}

CWBItem* CWBApplication::GetRoot() { return Root.get(); }

CWBItem* CWBApplication::GetFocusItem() {
  CWBItem* fi = Root.get();
  while (fi) {
    if (!fi->ChildInFocus) {
      return fi;
    }
    fi = fi->ChildInFocus;
  }
  return Root.get();
}

bool CWBApplication::HandleMessages() {
  // as this function is called once every frame, it's the best place to update
  // the timer as well
  globalTimer.Update();

  // mouse click repeater
  if (ClickRepeaterMode != WBMOUSECLICKREPEATMODE::WB_MCR_OFF) {
    if (globalTimer.GetTime() > NextRepeatedClickTime) {
      POINT ap;
      GetCursorPos(&ap);
      ScreenToClient(hWnd, &ap);

      switch (ClickRepeaterMode) {
        case WBMOUSECLICKREPEATMODE::WB_MCR_LEFT:
          SendMessage(CWBMessage(this, WBM_LEFTBUTTONREPEAT, 0, ap.x, ap.y));
          break;
        case WBMOUSECLICKREPEATMODE::WB_MCR_RIGHT:
          SendMessage(CWBMessage(this, WBM_RIGHTBUTTONREPEAT, 0, ap.x, ap.y));
          break;
        case WBMOUSECLICKREPEATMODE::WB_MCR_MIDDLE:
          SendMessage(CWBMessage(this, WBM_MIDDLEBUTTONREPEAT, 0, ap.x, ap.y));
          break;
      }
      NextRepeatedClickTime += GetKeyboardRepeatTime();
    }
  }

  if (!CCoreWindowHandlerWin::HandleMessages()) return false;

  // handle gui messages here
  {
    std::scoped_lock l(MessageBufferMutex);
    // This stuff is very brittle. It even depends on Messages added during this
    // loop to be executed at the end of this loop. So even changing the for
    // loop to a range loop causes bugs.
    for (size_t i = 0; i < MessageBuffer.size(); i++) {
      ProcessMessage(MessageBuffer[i]);
    }
    MessageBuffer.clear();
  }
  return true;
}

void CWBApplication::RegisterItem(CWBItem* Item) {
  Items[Item->GetGuid()] = Item;

  if (Item->GetScreenRect().Contains(MousePos)) UpdateMouseItem();
}

void CWBApplication::UnRegisterItem(CWBItem* Item) {
  std::scoped_lock l(TrashMutex);
  auto it = std::find_if(
      Trash.begin(), Trash.end(),
      [Item](const std::shared_ptr<CWBItem>& i) { return i.get() == Item; });
  if (it != Trash.end()) {
    Trash.erase(it);
  }

  if (MouseCaptureItem == Item) ReleaseCapture();
  Items.erase(Item->GetGuid());

  if (Item->GetScreenRect().Contains(MousePos)) UpdateMouseItem();
}

void CWBApplication::SetDone(bool d) { Done = d; }

void CWBApplication::Display() { Display(DrawAPI.get()); }

void CWBApplication::Display(CWBDrawAPI* API) {
  // LOG_DBG("Begin Frame");

  CleanTrash();

  FinalizeMouseCursor();
  SelectMouseCursor(COREMOUSECURSOR::CM_ARROW);

  DrawAPI->SetUIRenderState();
  Device->Clear(true, true, ClearColor);
  Device->BeginScene();

  DrawAPI->SetOffset(Root->GetScreenRect().TopLeft());
  DrawAPI->SetParentCropRect(Root->GetScreenRect());
  DrawAPI->SetCropRect(Root->GetScreenRect());

  Atlas->ClearImageUsageflags();

  Root->DrawTree(API);

  DrawAPI->RenderDisplayList();
  Device->EndScene();
  Device->Flip(Vsync);

  // LOG_DBG("End Frame");

  // this is a convenient place to store this:
  Logger.ResetEntryCounter();

  // update frame time
  const int32_t frametime = globalTimer.GetTime();
  FrameTimes->Add(frametime - LastFrameTime);
  LastFrameTime = frametime;
}

CWBItem* CWBApplication::FindItemByGuid(WBGUID Guid, const TCHAR* type) {
  if (Items.find(Guid) == Items.end()) return nullptr;
  CWBItem* i = Items[Guid];

  if (type) return i->InstanceOf(type) ? i : nullptr;

  return i;
}

void CWBApplication::SendMessage(const CWBMessage& Message) {
  std::scoped_lock l(MessageBufferMutex);
  MessageBuffer.emplace_back(Message);
}

CWBItem* CWBApplication::SetCapture(CWBItem* Capturer) {
  CWBItem* old = MouseCaptureItem;
  MouseCaptureItem = Capturer;
  return old;
}

bool CWBApplication::ReleaseCapture() {
  MouseCaptureItem = nullptr;
  return true;
}

void CWBApplication::UpdateMouseItem() {
  CWBItem* MouseItemOld = MouseItem;
  MouseItem = GetItemUnderMouse(MousePos, WBM_MOUSEMOVE);
  if (MouseItem != MouseItemOld) {
    // call onmouseleave for the old items
    if (MouseItem) {
      CWBItem* olditem = MouseItemOld;
      while (olditem) {
        // find the old item in the tree of the new. if not found the mouse left
        // the item
        if (!MouseItem->FindItemInParentTree(olditem)) olditem->OnMouseLeave();
        olditem = olditem->Parent;
      }
    }

    // call onmouseenter for the new items
    if (MouseItemOld) {
      CWBItem* newitem = MouseItem;
      while (newitem) {
        // find the new item in the tree of the old. if not found the mouse
        // entered the item
        if (!MouseItemOld->FindItemInParentTree(newitem))
          newitem->OnMouseEnter();
        newitem = newitem->Parent;
      }
    }
  }
}

void CWBApplication::CleanTrash() {
  std::scoped_lock l(TrashMutex);
  for (auto i = Trash.size(); i > 0; i--) {
    auto& t = Trash[i - 1];
    if (t->Parent) {
      t->Parent->RemoveChild(t);
    }
    t.reset();
  }
  Trash.clear();
}

CWBItem* CWBApplication::GetMouseItem() { return MouseItem; }

CWBItem* CWBApplication::GetMouseCaptureItem() { return MouseCaptureItem; }

bool CWBApplication::CreateFont(std::string_view FontName,
                                CWBFontDescription* Font) {
  if (!Font) return false;

  auto f = std::make_unique<CWBFont>(Atlas.get());

  if (!f->Initialize(Font)) {
    return false;
  }

  if (Fonts.empty()) DefaultFont = f.get();

  Fonts[std::string(FontName)] = std::move(f);

  return true;
}

CWBFont* CWBApplication::GetFont(std::string_view FontName) {
  auto f = Fonts.find(std::string(FontName));
  if (f != Fonts.end()) {
    return f->second.get();
  }
  LOG_WARN("[gui] Font %s not found, falling back to default font",
           std::string(FontName).c_str());
  return DefaultFont;
}

CWBFont* CWBApplication::GetDefaultFont() { return DefaultFont; }

bool CWBApplication::SetDefaultFont(std::string_view FontName) {
  CWBFont* f = GetFont(FontName);
  if (!f) return false;
  DefaultFont = f;
  return true;
}

void CWBApplication::AddToTrash(const std::shared_ptr<CWBItem>& item) {
  std::scoped_lock l(TrashMutex);
  auto it = std::find(Trash.begin(), Trash.end(), item);
  if (it == Trash.end()) {
    Trash.emplace_back(item);
  }
}

bool CWBApplication::LoadXMLLayout(std::string_view XML) {
  auto doc = std::make_unique<CXMLDocument>();
  if (!doc->LoadFromString(XML)) {
    LOG_ERR("[gui] Error loading XML Layout: parsing failed");
    return false;
  }

  CXMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) {
    LOG_ERR("[gui] Error loading XML Layout: 'guidescriptor' member missing");
    return false;
  }

  root = root.GetChild("guidescriptor");

  if (!root.GetChildCount("gui")) {
    LOG_ERR("[gui] Error loading XML Layout: 'gui' member missing");
    return false;
  }

  CXMLNode gui = root.GetChild("gui");
  if (!gui.HasAttribute("id")) {
    LOG_ERR("[gui] Error loading XML Layout: 'id' member missing from 'gui'");
    return false;
  }

  auto id = gui.GetAttribute("id");

  LayoutRepository[id] = std::move(doc);

  return true;
}

bool CWBApplication::LoadXMLLayoutFromFile(std::string_view FileName) {
  CStreamReaderMemory f;
  if (!f.Open(FileName)) {
    LOG_ERR("[gui] Error loading XML Layout: file '%s' not found",
            std::string(FileName).c_str());
    return false;
  }

  const std::string_view s(reinterpret_cast<char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  return LoadXMLLayout(s);
}

bool CWBApplication::GenerateGUI(CWBItem* Root, std::string_view Layout) {
  std::string l(Layout);
  if (LayoutRepository.find(l) == LayoutRepository.end()) {
    LOG_ERR("[gui] Error generating UI: layout '%s' not loaded", l.c_str());
    return false;
  }

  const bool b = GenerateGUIFromXML(Root, LayoutRepository[l].get());
  if (!b) return false;

  StyleManager.ApplyStyles(Root);
  CWBMessage m = Root->BuildPositionMessage(Root->GetPosition());
  m.Resized = true;
  Root->MessageProc(m);
  return true;
}

bool CWBApplication::GenerateGUITemplate(CWBItem* Root, std::string_view Layout,
                                         std::string_view TemplateID) {
  std::string l(Layout);
  if (LayoutRepository.find(l) == LayoutRepository.end()) {
    LOG_ERR("[gui] Error generating UI template: layout '%s' not loaded",
            l.c_str());
    return false;
  }

  const bool b =
      GenerateGUITemplateFromXML(Root, LayoutRepository[l].get(), TemplateID);
  if (!b) return false;

  StyleManager.ApplyStyles(Root);
  CWBMessage m = Root->BuildPositionMessage(Root->GetPosition());
  m.Resized = true;
  Root->MessageProc(m);

  return true;
}

void CWBApplication::ApplyStyle(CWBItem* Target) {
  StyleManager.ApplyStyles(Target);
  CWBMessage m = Target->BuildPositionMessage(Target->GetPosition());
  m.Resized = true;
  Target->MessageProc(m);
}

void CWBApplication::ReApplyStyle() {
  CWBItem* Root = GetRoot();
  StyleManager.ApplyStyles(Root);
  CWBMessage m = Root->BuildPositionMessage(Root->GetPosition());
  m.Resized = true;
  Root->MessageProc(m);
}

bool CWBApplication::LoadCSS(std::string_view CSS, bool ResetStyleManager) {
  if (ResetStyleManager) StyleManager.Reset();
  return StyleManager.ParseStyleData(CSS);
}

bool CWBApplication::LoadCSSFromFile(std::string_view FileName,
                                     bool ResetStyleManager) {
  CStreamReaderMemory f;
  if (!f.Open(FileName)) {
    LOG_ERR("[gui] Error loading CSS: file '%s' not found",
            std::string(FileName).c_str());
    return false;
  }

  const std::string_view s(reinterpret_cast<char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  const bool b = LoadCSS(s, ResetStyleManager);

  return b;
}

CAtlas* CWBApplication::GetAtlas() { return Atlas.get(); }

CWBSkin* CWBApplication::GetSkin() { return Skin.get(); }

bool CWBApplication::LoadSkin(std::string_view XML,
                              std::vector<int>& enabledGlyphs) {
  CXMLDocument doc;
  if (!doc.LoadFromString(XML)) return false;
  if (!doc.GetDocumentNode().GetChildCount("whiteboardskin")) return false;

  CXMLNode r = doc.GetDocumentNode().GetChild("whiteboardskin");

  for (int32_t x = 0; x < r.GetChildCount("image"); x++) {
    CXMLNode n = r.GetChild("image", x);
    auto img = n.GetAttributeAsString("image");

    auto data = B64Decode(img);

    std::unique_ptr<uint8_t[]> Image;
    int32_t XRes, YRes;
    if (DecompressPNG((unsigned char*)data.c_str(), data.size(), Image, XRes,
                      YRes)) {
      ARGBtoABGR(Image.get(), XRes, YRes);
      ClearZeroAlpha(Image.get(), XRes, YRes);

      for (int32_t y = 0; y < n.GetChildCount("element"); y++) {
        CXMLNode e = n.GetChild("element", y);

        CRect r2;
        e.GetAttributeAsInteger("x1", &r2.x1);
        e.GetAttributeAsInteger("y1", &r2.y1);
        e.GetAttributeAsInteger("x2", &r2.x2);
        e.GetAttributeAsInteger("y2", &r2.y2);

        CPoint b;
        e.GetAttributeAsInteger("x-behavior", &b.x);
        e.GetAttributeAsInteger("y-behavior", &b.y);

        const WBATLASHANDLE h = Atlas->AddImage(Image.get(), XRes, YRes, r2);
        Skin->AddElement(e.GetAttributeAsString("name"), h,
                         static_cast<WBSKINELEMENTBEHAVIOR>(b.x),
                         static_cast<WBSKINELEMENTBEHAVIOR>(b.y));
      }
    }
  }

  for (int32_t x = 0; x < r.GetChildCount("mosaic"); x++) {
    CXMLNode m = r.GetChild("mosaic", x);

    CRect r2;
    m.GetAttributeAsInteger("overshootx1", &r2.x1);
    m.GetAttributeAsInteger("overshooty1", &r2.y1);
    m.GetAttributeAsInteger("overshootx2", &r2.x2);
    m.GetAttributeAsInteger("overshooty2", &r2.y2);

    Skin->AddMosaic(m.GetAttributeAsString("name"),
                    m.GetAttributeAsString("description"), r2.x1, r2.y1, r2.x2,
                    r2.y2);
  }

  for (int32_t x = 0; x < r.GetChildCount("font"); x++) {
    CXMLNode n = r.GetChild("font", x);

    auto Name = n.GetAttributeAsString("name");

    auto img = n.GetAttributeAsString("image");
    auto bin = n.GetAttributeAsString("binary");

    // uint8_t* Dataimg = nullptr;
    // uint8_t* Databin = nullptr;
    const int32_t Sizeimg = 0;
    const int32_t Sizebin = 0;
    auto dataimg = B64Decode(img);
    auto databin = B64Decode(bin);

    int32_t XRes, YRes;
    std::unique_ptr<uint8_t[]> Image;
    if (DecompressPNG((unsigned char*)dataimg.c_str(), dataimg.size(), Image,
                      XRes, YRes)) {
      ARGBtoABGR(Image.get(), XRes, YRes);
      auto fd = std::make_unique<CWBFontDescription>();
      if (fd->LoadBMFontBinary((unsigned char*)databin.c_str(), databin.size(),
                               Image.get(), XRes, YRes, enabledGlyphs)) {
        const bool f = CreateFont(Name, fd.get());
      } else if (fd->LoadBMFontText((unsigned char*)databin.c_str(),
                                    databin.size(), Image.get(), XRes, YRes,
                                    enabledGlyphs)) {
        const bool f = CreateFont(Name, fd.get());
      }

      fd.reset();
    }
  }

  return true;
}

bool CWBApplication::LoadSkinFromFile(std::string_view FileName,
                                      std::vector<int>& enabledGlyphs) {
  CStreamReaderMemory f;
  if (!f.Open(FileName)) {
    LOG_ERR("[gui] Error loading Skin: file '%s' not found",
            std::string(FileName).c_str());
    return false;
  }

  const std::string_view s(reinterpret_cast<char*>(f.GetData()),
                           static_cast<int32_t>(f.GetLength()));
  const bool b = LoadSkin(s, enabledGlyphs);
  if (b)
    LOG_NFO("[gui] Successfully loaded Skin '%s'",
            std::string(FileName).c_str());

  return b;
}

bool CWBApplication::GenerateGUIFromXML(CWBItem* Root, CXMLDocument* doc) {
  CXMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) return false;

  root = root.GetChild("guidescriptor");

  if (!root.GetChildCount("gui")) return false;

  CXMLNode gui = root.GetChild("gui");

  return ProcessGUIXML(Root, gui);
}

bool CWBApplication::GenerateGUITemplateFromXML(CWBItem* Root,
                                                CXMLDocument* doc,
                                                std::string_view TemplateID) {
  CXMLNode root = doc->GetDocumentNode();

  if (!root.GetChildCount("guidescriptor")) return false;

  root = root.GetChild("guidescriptor");

  for (int32_t x = 0; x < root.GetChildCount("guitemplate"); x++) {
    CXMLNode t = root.GetChild("guitemplate", x);

    if (t.HasAttribute("id"))
      if (t.GetAttributeAsString("id") == TemplateID) {
        if (t.HasAttribute("class")) {
          auto a = Split(t.GetAttribute("class"), " ");
          for (const auto& s : a)
            if (s.size() > 1) Root->AddClass(s);
        }

        return ProcessGUIXML(Root, t);
      }
  }

  return false;
}

bool CWBApplication::ProcessGUIXML(CWBItem* Root, const CXMLNode& node) {
  CRect Pos(5, 5, 25, 25);

  bool b = true;
  for (int i = 0; i < node.GetChildCount(); i++) {
    b &= GenerateGUIFromXMLNode(Root, node.GetChild(i), Pos);
    Pos = CRect(Pos.BottomLeft() + CPoint(0, 2),
                Pos.BottomLeft() + CPoint(20, 22));
  }
  return b;
}

bool CWBApplication::GenerateGUIFromXMLNode(CWBItem* Root, const CXMLNode& node,
                                            CRect& Pos) {
  CWBItem* NewItem = GenerateUIItem(Root, node, Pos);
  if (!NewItem) return false;

  if (node.HasAttribute("pos")) {
    const auto& pos = node.GetAttribute("pos");
    if (std::count(pos.begin(), pos.end(), ',') == 3) {
      std::sscanf(node.GetAttribute("pos").c_str(), "%d,%d,%d,%d", &Pos.x1,
                  &Pos.y1, &Pos.x2, &Pos.y2);
    } else {
      uint32_t x = 0, y = 0;
      std::sscanf(node.GetAttribute("pos").c_str(), "%d,%d", &x, &y);
      Pos.MoveTo(x, y);
    }
    NewItem->SetPosition(Pos);
  }

  if (node.HasAttribute("size")) {
    uint32_t x = 0, y = 0;
    std::sscanf(node.GetAttribute("size").c_str(), "%d,%d", &x, &y);
    Pos.SetSize(x, y);
    NewItem->SetPosition(Pos);
  }

  if (node.HasAttribute("id")) NewItem->SetID(node.GetAttribute("id"));

  if (node.HasAttribute("class")) {
    auto a = Split(node.GetAttribute("class"), " ");
    for (const auto& s : a)
      if (s.size() > 1) NewItem->AddClass(s);
  }

  return ProcessGUIXML(NewItem, node);
}

CWBItem* CWBApplication::GenerateUIItem(CWBItem* Root, const CXMLNode& node,
                                        CRect& Pos) {
  if (FactoryCallbacks.find(node.GetNodeName()) != FactoryCallbacks.end()) {
    return FactoryCallbacks[node.GetNodeName()](Root, node, Pos);
  }

  LOG_ERR("[xml2gui] Unknown tag: '%s'", node.GetNodeName().c_str());
  return nullptr;
}

void CWBApplication::RegisterUIFactoryCallback(
    std::string_view ElementName, WBFACTORYCALLBACK FactoryCallback) {
  FactoryCallbacks[std::string(ElementName)] = FactoryCallback;
}

void CWBApplication::TakeScreenshot() {
  CreateDirectory("Screenshots", nullptr);

  int32_t maxcnt = 0;
  {
    auto s = ScreenShotName + "_*.png";
    CFileList fl(s, "Screenshots");

    for (auto& File : fl.Files) {
      if (File.FileName.find(ScreenShotName) == std::string::npos) {
        auto s2 = ScreenShotName + "_%d";

        int32_t no = -1;
        sscanf(File.FileName.c_str(), s2.c_str(), &no);
        maxcnt = std::max(maxcnt, no);
      }
    }
  }

  DrawAPI->FlushDrawBuffer();

  auto b = DrawAPI->GetDevice()->CreateBlendState();
  b->SetBlendEnable(0, true);
  b->SetIndependentBlend(true);
  b->SetSrcBlend(0, COREBLENDFACTOR::COREBLEND_ZERO);
  b->SetDestBlend(0, COREBLENDFACTOR::COREBLEND_ONE);
  b->SetSrcBlendAlpha(0, COREBLENDFACTOR::COREBLEND_ONE);
  b->SetDestBlendAlpha(0, COREBLENDFACTOR::COREBLEND_ZERO);
  DrawAPI->GetDevice()->SetRenderState(b.get());

  DrawAPI->SetCropRect(CRect(0, 0, XRes, YRes));
  DrawAPI->DrawRect(CRect(0, 0, XRes, YRes), CColor{0xff000000});
  DrawAPI->FlushDrawBuffer();

  auto fname =
      std::format("Screenshots\\{:s}_{:04d}.png", ScreenShotName, maxcnt + 1);
  DrawAPI->GetDevice()->TakeScreenShot(fname);

  DrawAPI->SetUIRenderState();
}

float CWBApplication::GetFrameRate() {
  int32_t FrameTimeAcc = 0;
  int32_t FrameCount = 0;
  for (int32_t x = 0; x < 60; x++) {
    if (FrameTimes->NumItems() < x) break;
    FrameTimeAcc += (*FrameTimes)[FrameTimes->NumItems() - 1 - x];
    FrameCount++;
  }

  if (!FrameCount) return 0;
  if (!FrameTimeAcc) return 9999;
  return 1000.0f / (FrameTimeAcc / static_cast<float>(FrameCount));
}
