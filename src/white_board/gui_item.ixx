module;

#include <tchar.h>

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/base/color.h"

export module whiteboard:gui_item;

import :css_item;
import :draw_api;
import :font;
import :message_enum;
import :skin;

export namespace gui {

typedef uint32_t WBGUID;
class DrawAPI;
class Application;
class CWBMessage;

constexpr int32_t WBMarginKeep = (INT_MAX);

class ContextMenu;

enum class WBALIGNMENT : uint8_t {
  TOP = 0,
  LEFT = 0,
  CENTER = 1,
  MIDDLE = 1,
  RIGHT = 2,
  BOTTOM = 2,
};

enum WBITEMSTATE {
  WB_STATE_NORMAL = 0,
  WB_STATE_ACTIVE = 1,
  WB_STATE_HOVER = 2,
  WB_STATE_DISABLED = 3,
  WB_STATE_DISABLED_ACTIVE = 4,

  WB_STATE_COUNT,  // don't remove this, used as array size
};

enum WBITEMVISUALCOMPONENT {
  WB_ITEM_BACKGROUNDCOLOR = 0,
  WB_ITEM_FOREGROUNDCOLOR,
  WB_ITEM_BORDERCOLOR,
  WB_ITEM_FONTCOLOR,
  WB_ITEM_BACKGROUNDIMAGE,
  WB_ITEM_SUBSKIN,
  // WB_ITEM_SELECTIONCOLOR,
  // WB_ITEM_SELECTIONFONTCOLOR,
  WB_ITEM_OPACITY,
  WB_ITEM_BACKGROUNDALIGNMENT_X,
  WB_ITEM_BACKGROUNDALIGNMENT_Y,
  WB_ITEM_TEXTTRANSFORM,

  // scrollbar stuff
  WB_ITEM_SCROLL_UP,
  WB_ITEM_SCROLL_DOWN,
  WB_ITEM_SCROLL_LEFT,
  WB_ITEM_SCROLL_RIGHT,
  WB_ITEM_SCROLL_HBAR,
  WB_ITEM_SCROLL_VBAR,
  WB_ITEM_SCROLL_HTHUMB,
  WB_ITEM_SCROLL_VTHUMB,

  WB_ITEM_COUNT,  // don't remove this, used as array size
};

enum WBSCROLLDRAGMODE {
  WB_SCROLLDRAG_NONE = 0,
  WB_SCROLLDRAG_BUTTON1,
  WB_SCROLLDRAG_UP,
  WB_SCROLLDRAG_THUMB,
  WB_SCROLLDRAG_DOWN,
  WB_SCROLLDRAG_BUTTON2,
};

class ScrollbarParams {
 public:
  ScrollbarParams()
      : drag_start_position_(0),
        drag_mode_(WB_SCROLLDRAG_NONE),
        visible_(false),
        dynamic_(false),
        enabled_(false),
        min_scroll_(0),
        max_scroll_(0),
        scroll_pos_(0),
        view_size_(0) {}

  bool enabled_;  // determines whether the scrollbar will be displayed at all
  bool dynamic_;  // if true the scrollbar disappears when not needed
  bool visible_;  // if true the client area has been adjusted so the scrollbar
                  // can fit

  // position data
  int32_t min_scroll_, max_scroll_;
  int32_t scroll_pos_;
  int32_t view_size_;

  // dragging data
  WBSCROLLDRAGMODE drag_mode_;
  int32_t drag_start_position_;
};

class DisplayState {
 public:
  DisplayState();
  virtual ~DisplayState();

  bool IsSet(WBITEMVISUALCOMPONENT v);
  CColor GetColor(WBITEMVISUALCOMPONENT v);
  WBSKINELEMENTID GetSkin(WBITEMVISUALCOMPONENT v);
  void SetValue(WBITEMVISUALCOMPONENT v, int32_t value);
  int32_t GetValue(WBITEMVISUALCOMPONENT v);

 private:
  std::array<uint32_t, WB_ITEM_COUNT> visuals_ = {0};
  std::array<bool, WB_ITEM_COUNT> visual_set_ = {false};
};

class DisplayProperties {
 public:
  DisplayProperties();
  virtual ~DisplayProperties();

  CColor GetColor(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);
  WBSKINELEMENTID GetSkin(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);
  void SetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v, int32_t value);
  int32_t GetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);

 private:
  std::array<DisplayState, WB_STATE_COUNT> states_;
};

class CssPropertyBatch {
 public:
  CssPropertyBatch();
  Font* GetFont(Application* App, WBITEMSTATE State);
  bool ApplyStyle(Widget* Owner, std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo);

  math::Rect border_sizes_;
  WBTEXTALIGNMENTX text_align_x_;
  WBTEXTALIGNMENTY text_align_y_;
  DisplayProperties display_descriptor_;
  PositionDescriptor position_descriptor_;
  std::unordered_map<WBITEMSTATE, std::string> fonts_;
};

class Widget : public CSS {
  friend Application;  // so we don't directly expose the message handling
                       // functions to the user
 public:
  ~Widget() override;

  virtual void AddChild(std::unique_ptr<Widget>&& Item);
  void RemoveChild(const Widget* Item);
  virtual bool Initialize(Widget* Parent, const math::Rect& Position);
  // return true if this item handled the message
  virtual bool MessageProc(const CWBMessage& Message);
  bool FindItemInParentTree(const Widget* Item);

  [[nodiscard]] const WBGUID GetGuid() const { return guid_; }
  [[nodiscard]] Application* GetApplication() const { return app_; }
  [[nodiscard]] Widget* GetParent() const { return parent_; }

  [[nodiscard]] virtual math::Rect GetClientRect()
      const;  // returns value in client space
  [[nodiscard]] virtual math::Rect GetWindowRect()
      const;  // returns value in client space
  [[nodiscard]] virtual math::Rect GetScreenRect()
      const;  // returns value in screen space

  [[nodiscard]] virtual math::Point ScreenToClient(const math::Point& p) const;
  [[nodiscard]] virtual math::Rect ScreenToClient(const math::Rect& p) const;
  [[nodiscard]] virtual math::Point ClientToScreen(const math::Point& p) const;
  [[nodiscard]] virtual math::Rect ClientToScreen(const math::Rect& p) const;

  virtual void SetPosition(const math::Rect& Pos);
  virtual void ApplyRelativePosition();
  // only to be used by the parent item when moving the item around
  virtual void ApplyPosition(const math::Rect& Pos);
  virtual void SetClientPadding(int32_t left, int32_t top, int32_t right,
                                int32_t bottom);
  // tells if the width has been specified in the style of the item
  [[nodiscard]] bool IsWidthSet() const;
  // tells if the height has been specified in the style of the item
  [[nodiscard]] bool IsHeightSet() const;
  // tells if the width has been specified in the style of the item
  [[nodiscard]] int32_t GetCalculatedWidth(math::Size ParentSize) const;
  // tells if the height has been specified in the style of the item
  [[nodiscard]] int32_t GetCalculatedHeight(math::Size ParentSize) const;

  [[nodiscard]] math::Rect GetPosition() const;

  [[nodiscard]] uint32_t NumChildren() const;
  Widget* GetChild(uint32_t idx);

  bool InFocus() const;
  [[nodiscard]] bool InLocalFocus() const;
  void SetFocus();
  void ClearFocus();
  bool MouseOver() const;
  virtual Widget* GetChildInFocus();

  void SavePosition();
  [[nodiscard]] math::Rect GetSavedPosition() const;
  void SetSavedPosition(const math::Rect& savedPos);

  void Hide(bool Hide);
  bool IsHidden();
  void Enable(bool Enabled);
  bool IsEnabled();

  void SetData(void* data);
  void* GetData();

  virtual void MarkForDeletion();

  virtual ContextMenu* OpenContextMenu(math::Point Position);

  [[nodiscard]] const std::string& GetType() const override {
    static const std::string type = "guiitem";
    return type;
  }

  [[nodiscard]] bool InstanceOf(std::string_view name) const override {
    return name == GetClassName();
  }

  virtual void EnableHScrollbar(bool Enabled, bool Dynamic);
  virtual void EnableVScrollbar(bool Enabled, bool Dynamic);
  virtual bool IsHScrollbarEnabled();
  virtual bool IsVScrollbarEnabled();
  virtual void SetHScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                       int32_t ViewSize);
  virtual void SetVScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                       int32_t ViewSize);
  virtual void GetHScrollbarParameters(int32_t* MinScroll, int32_t* MaxScroll,
                                       int32_t* ViewSize);
  virtual void GetVScrollbarParameters(int32_t* MinScroll, int32_t* MaxScroll,
                                       int32_t* ViewSize);
  virtual void SetHScrollbarPos(int32_t ScrollPos, bool Clamp = false);
  virtual void SetVScrollbarPos(int32_t ScrollPos, bool Clamp = false);
  virtual int32_t GetHScrollbarPos() { return HScrollbar.scroll_pos_; };
  virtual int32_t GetVScrollbarPos() { return VScrollbar.scroll_pos_; };
  virtual void SetTopmost();
  virtual void SetBottommost();

  virtual void SetBorderSizes(char Left, char Top, char Right, char Bottom);
  const math::Rect& GetBorderSizes() const {
    return css_properties_.border_sizes_;
  }
  virtual void SetFont(WBITEMSTATE State, std::string_view Font);

  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  Widget* FindChildByID(std::string_view value, std::string_view type = "");

  template <typename t>
  t* FindChildByID(std::string_view value) {
    Widget* it = FindChildByID(value, t::GetClassName());
    if (!it) return nullptr;
    return static_cast<t*>(it);
  }

  template <typename... Args>
  Widget* FindChildByIDs(std::string_view value, Args... args) {
    int len = sizeof...(Args);
    const TCHAR* vals[] = {args...};

    if (!len) return FindChildByID(value);

    for (int x = 0; x < len; x++) {
      auto child = FindChildByID(value, vals[x]);
      if (child) return child;
    }

    return nullptr;
  }

  Widget* FindParentByID(std::string_view value,
                         std::string_view type = "") const;
  virtual void CalculateWindowPosition(const math::Size& s);

  [[nodiscard]] CWBMessage BuildPositionMessage(const math::Rect& Pos) const;
  [[nodiscard]] CWBMessage BuildPositionMessage(const math::Rect& Pos,
                                                bool resized) const;
  void ApplyStyleDeclarations(std::string_view String);

  virtual WBITEMSTATE GetState();
  void SetDisplayProperty(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                          int32_t value);
  DisplayProperties& GetDisplayDescriptor() {
    return css_properties_.display_descriptor_;
  }
  const DisplayProperties& GetDisplayDescriptor() const {
    return css_properties_.display_descriptor_;
  }
  CssPropertyBatch& GetCSSProperties() { return css_properties_; }

  virtual Font* GetFont(WBITEMSTATE State);

  PositionDescriptor& GetPositionDescriptor() {
    return css_properties_.position_descriptor_;
  }
  const PositionDescriptor& GetPositionDescriptor() const {
    return css_properties_.position_descriptor_;
  }
  math::Size GetClientWindowSizeDifference();

  virtual void SetChildInFocus(Widget* i);

  virtual bool InterpretPositionString(CssPropertyBatch& pos,
                                       std::string_view prop,
                                       std::string_view value,
                                       const std::vector<std::string>& pseudo);
  virtual bool InterpretDisplayString(CssPropertyBatch& desc,
                                      std::string_view prop,
                                      std::string_view value,
                                      const std::vector<std::string>& pseudo);
  virtual bool InterpretFontString(CssPropertyBatch& desc,
                                   std::string_view prop,
                                   std::string_view value,
                                   const std::vector<std::string>& pseudo);
  // for fading out whole subtrees
  virtual void SetTreeOpacityMultiplier(float OpacityMul);
  virtual float GetTreeOpacityMultiplier();

  virtual void ReapplyStyles();
  virtual void SetForcedMouseTransparency(bool transparent);
  bool MarkedForDeletion();

 protected:
  Widget();

  // returns the highlight areas of the scrollbar in client space
  virtual bool GetHScrollbarRectangles(math::Rect* button1,
                                       math::Rect* Scrollup, math::Rect* Thumb,
                                       math::Rect* Scrolldown,
                                       math::Rect* button2);
  virtual bool GetVScrollbarRectangles(math::Rect* button1,
                                       math::Rect* Scrollup, math::Rect* Thumb,
                                       math::Rect* Scrolldown,
                                       math::Rect* button2);

  virtual void OnDraw(DrawAPI* API);
  virtual void OnPostDraw(DrawAPI* API);
  virtual int32_t GetScrollbarStep();

  Widget* GetRoot();
  const Widget* GetRoot() const;
  Widget* GetFocusItem();
  const Widget* GetFocusItem() const;
  virtual Widget* GetItemUnderMouse(const math::Point& Point,
                                    const math::Rect& CropRect,
                                    WBMESSAGE MessageType);
  const Widget* GetMouseItem() const;
  const Widget* GetMouseCaptureItem() const;
  const math::Point GetMousePos() const;
  void SelectMouseCursor(renderer::MouseCursor cursor);

  virtual void SetChildAsTopmost(int32_t Index);
  virtual void SetChildAsBottommost(int32_t Index);
  virtual bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                                  WBMESSAGE MessageType);

  Widget* SetCapture();
  [[nodiscard]] bool ReleaseCapture() const;
  virtual int32_t GetChildIndex(Widget* Item);

  virtual bool ScrollbarDragged();

  virtual void DrawBackgroundItem(
      DrawAPI* API, DisplayProperties& Descriptor, const math::Rect& Pos,
      WBITEMSTATE i, WBITEMVISUALCOMPONENT v = WB_ITEM_BACKGROUNDIMAGE);
  virtual void DrawBackground(DrawAPI* API, WBITEMSTATE State);
  virtual void DrawBackground(DrawAPI* API);
  virtual void DrawBorder(DrawAPI* API);
  virtual void ApplyOpacity(DrawAPI* API);

  virtual void DrawBackground(DrawAPI* API, const math::Rect& rect,
                              WBITEMSTATE State, CssPropertyBatch& cssProps);
  virtual void DrawBorder(DrawAPI* API, const math::Rect& rect,
                          CssPropertyBatch& cssProps);

  virtual std::vector<std::string> ExplodeValueWithoutSplittingParameters(
      std::string_view String);
  virtual bool ParseRGBA(std::string_view description, CColor* output);

  static void PositionApplicator(PositionDescriptor& pos, WBPOSITIONTYPE Type,
                                 std::string_view Value);
  static void VisualStyleApplicator(DisplayProperties& desc,
                                    WBITEMVISUALCOMPONENT TargetComponent,
                                    int32_t Value,
                                    const std::vector<std::string>& pseudo);
  static void FontStyleApplicator(CssPropertyBatch& desc,
                                  const std::vector<std::string>& pseudo,
                                  std::string_view name);

  // auto resize stuff
  virtual math::Size GetContentSize();
  virtual void ContentChanged();
  virtual void ChangeContentOffsetX(int32_t OffsetX);
  virtual void ChangeContentOffsetY(int32_t OffsetY);
  bool ScrollbarsEnabled();

  virtual math::Point GetContentOffset() { return content_offset_; }

  Application* app_ = nullptr;

 private:
  void UpdateScreenRect();

  virtual void OnMove(const math::Point& p);
  virtual void OnResize(const math::Size& s);
  virtual void OnMouseEnter();
  virtual void OnMouseLeave();

  virtual void CalculateClientPosition();

  void DrawTree(DrawAPI* API);

  [[nodiscard]] virtual bool Focusable() const;

  virtual void AdjustClientAreaToFitScrollbars();
  virtual void ScrollbarHelperFunct(ScrollbarParams& s, int32_t* r,
                                    bool ScrollbarNeeded);
  virtual void ScrollbardisplayHelperFunct(const ScrollbarParams& s,
                                           int32_t* a1, int32_t* a2,
                                           int32_t* thumbsize,
                                           int32_t* thumbpos);
  virtual bool ScrollbarRequired(const ScrollbarParams& s);
  virtual int32_t CalculateScrollbarMovement(const ScrollbarParams& s,
                                             int32_t scrollbarsize,
                                             int32_t delta);
  virtual void DrawScrollbarButton(DrawAPI* API, ScrollbarParams& s,
                                   math::Rect& r, WBITEMVISUALCOMPONENT Button);
  virtual void DrawHScrollbar(DrawAPI* API);
  virtual void DrawVScrollbar(DrawAPI* API);
  virtual void HandleHScrollbarClick(WBSCROLLDRAGMODE m);
  virtual void HandleVScrollbarClick(WBSCROLLDRAGMODE m);
  virtual bool AllowMouseHighlightWhileCaptureItem() const { return false; }

  bool ScanPXValue(std::string_view Value, int32_t* Result,
                   std::string_view PropName);
  bool ScanSkinValue(std::string_view Value, WBSKINELEMENTID* Result,
                     std::string_view PropName);

  Widget* ChildSearcherFunct(std::string_view value,
                             std::string_view type = "");

  WBITEMSTATE GetScrollbarState(WBITEMVISUALCOMPONENT Component, math::Rect r);
  virtual void ChangeContentOffset(math::Point ContentOff);

  static const std::string& GetClassName() {
    static const std::string type = "guiitem";
    return type;
  }

  const WBGUID guid_;
  CssPropertyBatch css_properties_;

  math::Rect position_;     // stored in parent space
  math::Rect client_rect_;  // stored in window space
  math::Rect screen_rect_;  // calculated automatically, stores the position in
                            // screen space
  math::Rect stored_position_;
  math::Point content_offset_;  // describes how much the content is moved
                                // relative to the item. used for easily sliding
                                // content around by scrollbars

  math::Size stored_content_size_;

  Widget* parent_ = nullptr;
  Widget* child_in_focus_ = nullptr;
  std::vector<std::unique_ptr<Widget>> children_;

  int32_t sort_layer_ = 0;
  float opacity_multiplier_ = 1;

  bool hidden_ = false;
  bool disabled_ = false;
  bool force_mouse_transparent_ = false;
  int32_t scrollbar_size_ = 16;
  int32_t scrollbar_button_size_ = 16;
  int32_t scrollbar_thumb_minimal_size_ = 4;
  ScrollbarParams HScrollbar, VScrollbar;

  void* data_ = nullptr;
};

// OOP kung-fu follows to provide InstanceOf() functionality for use with CSS
// the initial virtual InstanceOf() call ensures we start at the bottom of the
// class hierarchy on each level we check if the static name of the current
// class is the typename we're comparing against if not we traverse up the
// hierarchy by directly calling the InstanceOf() of the parent class

template <size_t N>
struct FixedString {
  char value[N]{};
  consteval FixedString(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i) value[i] = str[i];
  }
};

template <FixedString TypeName, typename Parent>
class GuiType : public Parent {
 public:
  template <typename... Args>
  GuiType(Args&&... args) : Parent(std::forward<Args>(args)...) {}

  [[nodiscard]] const std::string& GetType() const override {
    return GetClassName();
  }

  [[nodiscard]] bool InstanceOf(std::string_view name) const override {
    if (name == std::string(TypeName.value)) return true;
    // This looks like a bug but seems to work
    if (Widget::InstanceOf(name)) return true;
    return false;
  }

  static const std::string& GetClassName() {
    static const std::string type = TypeName.value;
    return type;
  }
};

}  // namespace gui
