module;

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/base/ring_buffer.h"
#include "src/core2/core2_config.h"
#include "src/core2/dx11_device.h"
#include "src/core2/resource.h"
#include "src/core2/texture.h"

export module whiteboard:application;

import :context_menu;
import :draw_api;
import :font;
import :gui_item;
import :message;
import :root;
import :style_manager;
import math;
import xml;

export namespace gui {

enum class MouseClickRepeatMode : uint8_t {
  WB_MCR_OFF = 0,
  WB_MCR_LEFT = 1,
  WB_MCR_RIGHT = 2,
  WB_MCR_MIDDLE = 3
};

typedef Widget*(__cdecl* FactoryCallback)(Widget* Root, const XMLNode& node,
                                          const math::Rect& Pos);

class Application : public renderer::WindowHandlerWin {
  friend class Widget;

 public:
  Application();
  ~Application() override;
  bool Initialize(const renderer::WindowParameters& WindowParams) override;

  Widget* GetRoot();
  Widget* GetFocusItem();
  bool HandleMessages() override;
  virtual bool IsDone() { return done_; }
  virtual void SetDone(bool d);
  virtual void Display();
  virtual void Display(DrawAPI* DrawAPI);

  void RegisterItem(Widget* Item);
  void UnRegisterItem(Widget* Item);
  Widget* FindItemByGuid(WBGUID Guid, const TCHAR* type = nullptr) const;

  template <typename... Args>
  Widget* FindItemByGuids(WBGUID Guid, Args... args) const {
    int len = sizeof...(Args);
    const TCHAR* vals[] = {args...};

    if (!len) return FindItemByGuid(Guid);

    for (int x = 0; x < len; x++) {
      auto item = FindItemByGuid(Guid, vals[x]);
      if (item) return item;
    }

    return nullptr;
  }

  virtual void Send(const Message& message);
  Widget* SetCapture(Widget* Capturer);
  bool ReleaseCapture();

  Widget* GetMouseItem();
  Widget* GetMouseCaptureItem();

  bool InitFont(std::string_view FontName, FontDescription* Font);
  Font* GetFont(std::string_view FontName);
  Font* GetDefaultFont();
  bool SetDefaultFont(std::string_view FontName);
  void AddToTrash(std::unique_ptr<Widget>&& item);

  bool GetCtrlState() const { return ctrl_; }
  bool GetAltState() const { return alt_; }
  bool GetShiftState() const { return shift_; }
  bool GetLeftButtonState() const { return left_; }
  bool GetRightButtonState() const { return right_; }
  bool GetMiddleButtonState() const { return middle_; }

  bool LoadXMLLayout(std::string_view XML);
  bool LoadXMLLayoutFromFile(std::string_view FileName);
  bool LoadCSS(std::string_view CSS, bool ResetStyleManager = true);
  bool LoadCSSFromFile(std::string_view FileName,
                       bool ResetStyleManager = true);
  bool GenerateGUI(Widget* Root, std::string_view Layout);
  bool GenerateGUITemplate(Widget* Root, std::string_view Layout,
                           std::string_view TemplateID);
  bool LoadSkin(std::string_view XML, std::vector<int>& enabledGlyphs);
  bool LoadSkin(std::string_view XML) {
    std::vector<int> eg;
    return LoadSkin(XML, eg);
  }
  bool LoadSkinFromFile(std::string_view FileName,
                        std::vector<int>& enabledGlyphs);
  bool LoadSkinFromFile(std::string_view FileName) {
    std::vector<int> eg;
    return LoadSkinFromFile(FileName, eg);
  }

  Atlas* GetAtlas();
  Skin* GetSkin();

  void ApplyStyle(Widget* Target);
  void ReApplyStyle();

  void RegisterUIFactoryCallback(std::string_view ElementName,
                                 FactoryCallback FactoryCallback);

  virtual void TakeScreenshot();
  void SetScreenshotName(std::string_view s) { screen_shot_name_ = s; }

  void SetVSync(bool vs) { vsync_ = vs; }
  void SetClearColor(Color color) { clear_color_ = color; }

  float GetFrameRate();
  int32_t GetInitialKeyboardDelay();
  int32_t GetKeyboardRepeatTime();

  void HandleResize() override;

  LRESULT InjectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

 protected:
  LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  std::unique_ptr<DrawAPI> draw_api_;

 private:
  bool Initialize();
  bool GenerateGUIFromXML(Widget* Root, XMLDocument* doc);
  bool SendMessageToItem(const Message& Message, Widget* Target);
  void ProcessMessage(Message& message);
  Widget* GetItemUnderMouse(const math::Point& Point, MessageType w);

  virtual void UpdateMouseItem();
  virtual void CleanTrash();
  virtual void UpdateControlKeyStates();  // update ctrl alt shift states
  virtual int32_t GetKeyboardState();
  bool ProcessGUIXML(Widget* Root, const XMLNode& node);
  bool GenerateGUIFromXMLNode(Widget* Root, const XMLNode& node,
                              math::Rect* Pos);
  bool GenerateGUITemplateFromXML(Widget* Root, XMLDocument* doc,
                                  std::string_view TemplateID);
  Widget* GenerateUIItem(Widget* Root, const XMLNode& node,
                         const math::Rect& Pos);

  std::unique_ptr<RingBuffer<int32_t, 60>> frame_times_;
  int32_t last_frame_time_;

  std::unordered_map<WBGUID, Widget*> items_;
  std::recursive_mutex message_buffer_mutex_;
  std::vector<Message> message_buffer_;
  std::recursive_mutex trash_mutex_;
  std::vector<std::unique_ptr<Widget>> trash_;

  Widget* mouse_capture_item_;
  Widget* mouse_item_;

  std::unique_ptr<Skin> skin_;
  std::unordered_map<std::string, std::unique_ptr<Font>> fonts_;
  std::unique_ptr<Atlas> atlas_;
  std::unique_ptr<Root> root_;

  Font* default_font_;
  bool alt_, ctrl_, shift_, left_, middle_, right_;
  bool vsync_;
  std::string screen_shot_name_;
  MouseClickRepeatMode click_repeater_mode_;
  int64_t next_repeated_click_time_{};
  std::unordered_map<std::string, std::unique_ptr<XMLDocument>>
      layout_repository_;
  StyleManager style_manager_;
  std::unordered_map<std::string, FactoryCallback> factory_callbacks_;
  Color clear_color_ = Color(0, 0, 0, 255);
};

}  // namespace gui
