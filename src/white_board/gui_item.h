#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/white_board/css_item.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/message_enum.h"
#include "src/white_board/skin.h"

typedef uint32_t WBGUID;
class CWBDrawAPI;
class CWBApplication;
class CWBMessage;
enum WBMESSAGE;

#define WBMARGIN_KEEP (INT_MAX)
#define POSSIZETORECT(a, b) (CRect(a.x, a.y, a.x + b.x, a.y + b.y))

class CWBContextMenu;

enum WBALIGNMENT {
  WB_ALIGN_TOP = 0,
  WB_ALIGN_LEFT = 0,
  WB_ALIGN_CENTER = 1,
  WB_ALIGN_MIDDLE = 1,
  WB_ALIGN_RIGHT = 2,
  WB_ALIGN_BOTTOM = 2,
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

class CWBScrollbarParams {
 public:
  bool Enabled;  // determines whether the scrollbar will be displayed at all
  bool Dynamic;  // if true the scrollbar disappears when not needed
  bool Visible;  // if true the client area has been adjusted so the scrollbar
                 // can fit

  // position data
  int32_t MinScroll, MaxScroll;
  int32_t ScrollPos;
  int32_t ViewSize;

  // dragging data
  WBSCROLLDRAGMODE Dragmode;
  int32_t DragStartPosition;

  CWBScrollbarParams() {
    DragStartPosition = 0;
    Dragmode = WB_SCROLLDRAG_NONE;
    Visible = Dynamic = Enabled = false;
    MinScroll = MaxScroll = ScrollPos = 0;
    ViewSize = 0;
  }
};

class CWBDisplayState {
  std::array<uint32_t, WB_ITEM_COUNT> Visuals = {0};
  std::array<bool, WB_ITEM_COUNT> VisualSet = {false};

 public:
  CWBDisplayState();
  virtual ~CWBDisplayState();

  bool IsSet(WBITEMVISUALCOMPONENT v);
  CColor GetColor(WBITEMVISUALCOMPONENT v);
  WBSKINELEMENTID GetSkin(WBITEMVISUALCOMPONENT v);
  void SetValue(WBITEMVISUALCOMPONENT v, int32_t value);
  int32_t GetValue(WBITEMVISUALCOMPONENT v);
};

class CWBDisplayProperties {
  std::array<CWBDisplayState, WB_STATE_COUNT> States;

 public:
  CWBDisplayProperties();
  virtual ~CWBDisplayProperties();

  CColor GetColor(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);
  WBSKINELEMENTID GetSkin(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);
  void SetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v, int32_t value);
  int32_t GetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v);
};

class CWBCSSPropertyBatch {
 public:
  CRect BorderSizes;
  WBTEXTALIGNMENTX TextAlignX;
  WBTEXTALIGNMENTY TextAlignY;
  CWBDisplayProperties DisplayDescriptor;
  CWBPositionDescriptor PositionDescriptor;
  std::unordered_map<WBITEMSTATE, std::string> Fonts;

  CWBCSSPropertyBatch();
  CWBFont* GetFont(CWBApplication* App, WBITEMSTATE State);
  bool ApplyStyle(CWBItem* Owner, std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo);
};

class CWBItem : public IWBCSS {
  friend CWBApplication;  // so we don't directly expose the message handling
                          // functions to the user

  const WBGUID Guid;

  CRect Position;    // stored in parent space
  CRect ClientRect;  // stored in window space
  CRect ScreenRect;  // calculated automatically, stores the position in screen
                     // space
  CRect StoredPosition;
  CPoint ContentOffset;  // describes how much the content is moved relative to
                         // the item. used for easily sliding content around by
                         // scrollbars

  CSize StoredContentSize;

  CWBItem* Parent = nullptr;
  std::vector<std::shared_ptr<CWBItem>> Children;

  int32_t SortLayer = 0;
  float OpacityMultiplier = 1;

  bool Hidden = false;
  bool Disabled = false;
  bool ForceMouseTransparent = false;
  int32_t Scrollbar_Size = 16;
  int32_t Scrollbar_ButtonSize = 16;
  int32_t Scrollbar_ThumbMinimalSize = 4;
  CWBScrollbarParams HScrollbar, VScrollbar;

  //////////////////////////////////////////////////////////////////////////
  // private functions

  void UpdateScreenRect();

  virtual void OnMove(const CPoint& p);
  virtual void OnResize(const CSize& s);
  virtual void OnMouseEnter();
  virtual void OnMouseLeave();

  virtual void CalculateClientPosition();

  void DrawTree(CWBDrawAPI* API);

  virtual bool Focusable() const;

  void* Data = nullptr;

  virtual void AdjustClientAreaToFitScrollbars();
  virtual void ScrollbarHelperFunct(CWBScrollbarParams& s, int32_t& r,
                                    bool ScrollbarNeeded);
  virtual void ScrollbardisplayHelperFunct(CWBScrollbarParams& s, int32_t& a1,
                                           int32_t& a2, int32_t& thumbsize,
                                           int32_t& thumbpos);
  virtual bool ScrollbarRequired(CWBScrollbarParams& s);
  virtual int32_t CalculateScrollbarMovement(CWBScrollbarParams& s,
                                             int32_t scrollbarsize,
                                             int32_t delta);
  virtual void DrawScrollbarButton(CWBDrawAPI* API, CWBScrollbarParams& s,
                                   CRect& r, WBITEMVISUALCOMPONENT Button);
  virtual void DrawHScrollbar(CWBDrawAPI* API);
  virtual void DrawVScrollbar(CWBDrawAPI* API);
  virtual void HandleHScrollbarClick(WBSCROLLDRAGMODE m);
  virtual void HandleVScrollbarClick(WBSCROLLDRAGMODE m);
  virtual bool AllowMouseHighlightWhileCaptureItem() { return false; }

  bool ScanPXValue(std::string_view Value, int32_t& Result,
                   std::string_view PropName);
  bool ScanSkinValue(std::string_view Value, WBSKINELEMENTID& Result,
                     std::string_view PropName);

  CWBItem* ChildSearcherFunct(std::string_view value,
                              std::string_view type = _T( "" ));

  WBITEMSTATE GetScrollbarState(WBITEMVISUALCOMPONENT Component, CRect r);
  virtual void ChangeContentOffset(CPoint ContentOff);

  static const std::string& GetClassName() {
    static const std::string type = _T( "guiitem" );
    return type;
  }

 protected:
  std::weak_ptr<CWBItem> SelfRef;
  CWBApplication* App = nullptr;
  CWBItem* ChildInFocus = nullptr;

  // returns the highlight areas of the scrollbar in client space
  virtual bool GetHScrollbarRectangles(CRect& button1, CRect& Scrollup,
                                       CRect& Thumb, CRect& Scrolldown,
                                       CRect& button2);
  virtual bool GetVScrollbarRectangles(CRect& button1, CRect& Scrollup,
                                       CRect& Thumb, CRect& Scrolldown,
                                       CRect& button2);

  virtual void OnDraw(CWBDrawAPI* API);
  virtual void OnPostDraw(CWBDrawAPI* API);
  virtual int32_t GetScrollbarStep();

  //////////////////////////////////////////////////////////////////////////
  // CSS modifiable properties

  CWBCSSPropertyBatch CSSProperties;

  virtual CWBItem* GetItemUnderMouse(CPoint& Point, CRect& CropRect,
                                     WBMESSAGE MessageType);
  virtual void SetChildAsTopmost(int32_t Index);
  virtual void SetChildAsBottommost(int32_t Index);
  virtual bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                                  WBMESSAGE MessageType);

  CWBItem* SetCapture();
  bool ReleaseCapture() const;
  virtual int32_t GetChildIndex(CWBItem* Item);

  virtual bool ScrollbarDragged();

  virtual void DrawBackgroundItem(
      CWBDrawAPI* API, CWBDisplayProperties& Descriptor, const CRect& Pos,
      WBITEMSTATE i, WBITEMVISUALCOMPONENT v = WB_ITEM_BACKGROUNDIMAGE);
  virtual void DrawBackground(CWBDrawAPI* API, WBITEMSTATE State);
  virtual void DrawBackground(CWBDrawAPI* API);
  virtual void DrawBorder(CWBDrawAPI* API);
  virtual void ApplyOpacity(CWBDrawAPI* API);

  virtual void DrawBackground(CWBDrawAPI* API, const CRect& rect,
                              WBITEMSTATE State, CWBCSSPropertyBatch& cssProps);
  virtual void DrawBorder(CWBDrawAPI* API, const CRect& rect,
                          CWBCSSPropertyBatch& cssProps);

  virtual std::vector<std::string> ExplodeValueWithoutSplittingParameters(
      std::string_view String);
  virtual bool ParseRGBA(std::string_view description, CColor& output);

  static void PositionApplicator(CWBPositionDescriptor& pos,
                                 WBPOSITIONTYPE Type, std::string_view Value);
  static void VisualStyleApplicator(CWBDisplayProperties& desc,
                                    WBITEMVISUALCOMPONENT TargetComponent,
                                    int32_t Value,
                                    const std::vector<std::string>& pseudo);
  static void FontStyleApplicator(CWBCSSPropertyBatch& desc,
                                  const std::vector<std::string>& pseudo,
                                  std::string_view name);

  // auto resize stuff
  virtual CSize GetContentSize();
  virtual void ContentChanged();
  virtual void ChangeContentOffsetX(int32_t OffsetX);
  virtual void ChangeContentOffsetY(int32_t OffsetY);
  bool ScrollbarsEnabled();

  virtual CPoint GetContentOffset() { return ContentOffset; }
  CWBItem();
  CWBItem(CWBItem* Parent, const CRect& Position);

 public:
  ~CWBItem() override;

  virtual void AddChild(const std::shared_ptr<CWBItem>& Item);
  virtual void RemoveChild(const std::shared_ptr<CWBItem>& Item);
  virtual bool Initialize(CWBItem* Parent, const CRect& Position);
  // return true if this item handled the message
  virtual bool MessageProc(const CWBMessage& Message);
  bool FindItemInParentTree(const CWBItem* Item);

  const WBGUID GetGuid() const { return Guid; }
  CWBApplication* GetApplication() const { return App; }
  CWBItem* GetParent() const { return Parent; }

  virtual CRect GetClientRect() const;  // returns value in client space
  virtual CRect GetWindowRect() const;  // returns value in client space
  virtual CRect GetScreenRect() const;  // returns value in screen space

  virtual CPoint ScreenToClient(const CPoint& p) const;
  virtual CRect ScreenToClient(const CRect& p) const;
  virtual CPoint ClientToScreen(const CPoint& p) const;
  virtual CRect ClientToScreen(const CRect& p) const;

  virtual void SetPosition(const CRect& Pos);
  virtual void ApplyRelativePosition();
  // only to be used by the parent item when moving the item around
  virtual void ApplyPosition(const CRect& Pos);
  virtual void SetClientPadding(int32_t left, int32_t top, int32_t right,
                                int32_t bottom);
  // tells if the width has been specified in the style of the item
  bool IsWidthSet();
  // tells if the height has been specified in the style of the item
  bool IsHeightSet();
  // tells if the width has been specified in the style of the item
  int32_t GetCalculatedWidth(CSize ParentSize);
  // tells if the height has been specified in the style of the item
  int32_t GetCalculatedHeight(CSize ParentSize);

  CRect GetPosition();

  uint32_t NumChildren();
  CWBItem* GetChild(uint32_t idx);

  bool InFocus();
  bool InLocalFocus();
  void SetFocus();
  void ClearFocus();
  bool MouseOver();
  virtual CWBItem* GetChildInFocus();

  void SavePosition();
  CRect GetSavedPosition() const;
  void SetSavedPosition(const CRect& savedPos);

  void Hide(bool Hide);
  bool IsHidden();
  void Enable(bool Enabled);
  bool IsEnabled();

  void SetData(void* data);
  void* GetData();

  virtual void MarkForDeletion();

  virtual CWBContextMenu* OpenContextMenu(CPoint Position);

  const std::string& GetType() const override {
    static const std::string type = _T( "guiitem" );
    return type;
  }

  bool InstanceOf(std::string_view name) const override {
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
  virtual void GetHScrollbarParameters(int32_t& MinScroll, int32_t& MaxScroll,
                                       int32_t& ViewSize);
  virtual void GetVScrollbarParameters(int32_t& MinScroll, int32_t& MaxScroll,
                                       int32_t& ViewSize);
  virtual void SetHScrollbarPos(int32_t ScrollPos, bool Clamp = false);
  virtual void SetVScrollbarPos(int32_t ScrollPos, bool Clamp = false);
  virtual int32_t GetHScrollbarPos() { return HScrollbar.ScrollPos; };
  virtual int32_t GetVScrollbarPos() { return VScrollbar.ScrollPos; };
  virtual void SetTopmost();
  virtual void SetBottommost();

  virtual void SetBorderSizes(char Left, char Top, char Right, char Bottom);
  virtual void SetFont(WBITEMSTATE State, std::string_view Font);

  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  CWBItem* FindChildByID(std::string_view value,
                         std::string_view type = _T( "" ));

  template <typename t>
  t* FindChildByID(std::string_view value) {
    CWBItem* it = FindChildByID(value, t::GetClassName());
    if (!it) return nullptr;
    return static_cast<t*>(it);
  }

  template <typename... Args>
  CWBItem* FindChildByIDs(std::string_view value, Args... args) {
    int len = sizeof...(Args);
    const TCHAR* vals[] = {args...};

    if (!len) return FindChildByID(value);

    for (int x = 0; x < len; x++) {
      auto child = FindChildByID(value, vals[x]);
      if (child) return child;
    }

    return nullptr;
  }

  CWBItem* FindParentByID(std::string_view value,
                          std::string_view type = _T( "" ));
  virtual void CalculateWindowPosition(const CSize& s);

  CWBMessage BuildPositionMessage(const CRect& Pos);
  void ApplyStyleDeclarations(std::string_view String);

  virtual WBITEMSTATE GetState();
  void SetDisplayProperty(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                          int32_t value);
  CWBDisplayProperties& GetDisplayDescriptor() {
    return CSSProperties.DisplayDescriptor;
  }
  CWBCSSPropertyBatch& GetCSSProperties() { return CSSProperties; }

  virtual CWBFont* GetFont(WBITEMSTATE State);

  CWBPositionDescriptor& GetPositionDescriptor();
  CSize GetClientWindowSizeDifference();

  virtual void SetChildInFocus(CWBItem* i);

  virtual bool InterpretPositionString(CWBCSSPropertyBatch& pos,
                                       std::string_view prop,
                                       std::string_view value,
                                       const std::vector<std::string>& pseudo);
  virtual bool InterpretDisplayString(CWBCSSPropertyBatch& desc,
                                      std::string_view prop,
                                      std::string_view value,
                                      const std::vector<std::string>& pseudo);
  virtual bool InterpretFontString(CWBCSSPropertyBatch& desc,
                                   std::string_view prop,
                                   std::string_view value,
                                   const std::vector<std::string>& pseudo);
  // for fading out whole subtrees
  virtual void SetTreeOpacityMultiplier(float OpacityMul);
  virtual float GetTreeOpacityMultiplier();

  virtual void ReapplyStyles();
  virtual void SetForcedMouseTransparency(bool transparent);
  bool MarkedForDeletion();
};

// OOP kung-fu follows to provide InstanceOf() functionality for use with CSS
// the initial virtual InstanceOf() call ensures we start at the bottom of the
// class hierarchy on each level we check if the static name of the current
// class is the typename we're comparing against if not we traverse up the
// hierarchy by directly calling the InstanceOf() of the parent class

#define WB_DECLARE_GUIITEM_1PARENTS(TYPE, PARENTCLASS)    \
  const std::string& GetType() const override {           \
    static const std::string type = TYPE;                 \
    return type;                                          \
  }                                                       \
                                                          \
  friend CWBItem;                                         \
                                                          \
 private:                                                 \
  static const std::string& GetClassName() {              \
    static const std::string type = TYPE;                 \
    return type;                                          \
  }                                                       \
                                                          \
 public:                                                  \
  bool InstanceOf(std::string_view name) const override { \
    if (name == GetClassName()) return true;              \
    return PARENTCLASS::InstanceOf(name);                 \
  }

#define WB_DECLARE_GUIITEM_2PARENTS(TYPE, PARENTCLASS1, PARENTCLASS2)        \
  const std::string& GetType() const override {                              \
    static const std::string type = TYPE;                                    \
    return type;                                                             \
  }                                                                          \
                                                                             \
  friend CWBItem;                                                            \
                                                                             \
 private:                                                                    \
  static const std::string& GetClassName() {                                 \
    static const std::string type = TYPE;                                    \
    return type;                                                             \
  }                                                                          \
                                                                             \
 public:                                                                     \
  bool InstanceOf(std::string_view name) const override {                    \
    if (name == GetClassName()) return true;                                 \
    return PARENTCLASS1::InstanceOf(name) || PARENTCLASS2::InstanceOf(name); \
  }

#define WB_DECLARE_GUIITEM_3PARENTS(TYPE, PARENTCLASS1, PARENTCLASS2,          \
                                    PARENTCLASS3)                              \
  const std::string& GetType() const override {                                \
    static const std::string type = TYPE;                                      \
    return type;                                                               \
  }                                                                            \
                                                                               \
  friend CWBItem;                                                              \
                                                                               \
 private:                                                                      \
  static const std::string& GetClassName() {                                   \
    static const std::string type = TYPE;                                      \
    return type;                                                               \
  }                                                                            \
                                                                               \
 public:                                                                       \
  bool InstanceOf(std::string_view name) const override {                      \
    if (name == GetClassName()) return true;                                   \
    return PARENTCLASS1::InstanceOf(name) || PARENTCLASS2::InstanceOf(name) || \
           PARENTCLASS3::InstanceOf(name);                                     \
  }

#define EXPAND(x) x
#define WB_DECLARE_MACRO_SELECTOR(_1, _2, _3, NAME, ...) NAME
#define WB_DECLARE_GUIITEM(TYPE, ...)                                        \
  EXPAND(EXPAND(WB_DECLARE_MACRO_SELECTOR(                                   \
      __VA_ARGS__, WB_DECLARE_GUIITEM_3PARENTS, WB_DECLARE_GUIITEM_2PARENTS, \
      WB_DECLARE_GUIITEM_1PARENTS))(TYPE, __VA_ARGS__))
