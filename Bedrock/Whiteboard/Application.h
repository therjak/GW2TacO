#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../BaseLib/Color.h"
#include "../CoRE2/Core2.h"
#include "../UtilLib/XMLDocument.h"
#include "ContextMenu.h"
#include "DrawAPI.h"
#include "GuiItem.h"
#include "Message.h"
#include "Root.h"
#include "StyleManager.h"

enum WBMOUSECLICKREPEATMODE {
  WB_MCR_OFF = 0,
  WB_MCR_LEFT = 1,
  WB_MCR_RIGHT = 2,
  WB_MCR_MIDDLE = 3
};

typedef CWBItem*(__cdecl* WBFACTORYCALLBACK)(CWBItem* Root, CXMLNode& node,
                                             CRect& Pos);

class CWBApplication : public CCoreWindowHandlerWin {
  friend class CWBItem;

  CRingBuffer<int32_t>* FrameTimes;
  int32_t LastFrameTime;

  std::unordered_map<WBGUID, CWBItem*> Items;
  CArrayThreadSafe<CWBMessage> MessageBuffer;
  std::recursive_mutex TrashMutex;
  std::vector<std::shared_ptr<CWBItem>> Trash;

  CWBItem* MouseCaptureItem;
  CWBItem* MouseItem;

  CWBSkin* Skin;
  std::unordered_map<std::string, std::unique_ptr<CWBFont>> Fonts;
  CWBFont* DefaultFont;

  bool Alt, Ctrl, Shift, Left, Middle, Right;

  bool Vsync;

  std::string ScreenShotName;

  bool SendMessageToItem(CWBMessage& Message, CWBItem* Target);
  void ProcessMessage(CWBMessage& Message);
  CWBItem* GetItemUnderMouse(CPoint& Point, WBMESSAGE w);

  virtual void UpdateMouseItem();
  virtual void CleanTrash();
  virtual void UpdateControlKeyStates();  // update ctrl alt shift states
  virtual int32_t GetKeyboardState();

  WBMOUSECLICKREPEATMODE ClickRepeaterMode;
  int64_t NextRepeatedClickTime;

  //////////////////////////////////////////////////////////////////////////
  // gui layout and style

  std::unordered_map<std::string, std::unique_ptr<CXMLDocument>>
      LayoutRepository;
  CStyleManager StyleManager;

  //////////////////////////////////////////////////////////////////////////
  // ui generator

  std::unordered_map<std::string, WBFACTORYCALLBACK> FactoryCallbacks;

  bool ProcessGUIXML(CWBItem* Root, CXMLNode& node);
  bool GenerateGUIFromXMLNode(CWBItem* Root, CXMLNode& node, CRect& Pos);
  bool GenerateGUITemplateFromXML(CWBItem* Root, CXMLDocument* doc,
                                  std::string_view TemplateID);
  CWBItem* GenerateUIItem(CWBItem* Root, CXMLNode& node, CRect& Pos);

  CColor ClearColor = CColor(0, 0, 0, 255);

 protected:
  CAtlas* Atlas;
  std::unique_ptr<CWBRoot> Root;

  CWBDrawAPI* DrawAPI;

  LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  bool GenerateGUIFromXML(CWBItem* Root, CXMLDocument* doc);
  virtual bool Initialize();

 public:
  bool Initialize(const CCoreWindowParameters& WindowParams) override;

  CWBApplication();
  ~CWBApplication() override;

  CWBItem* GetRoot();
  CWBItem* GetFocusItem();
  bool HandleMessages() override;
  virtual bool IsDone() { return Done; }
  virtual void SetDone(bool d);
  virtual void Display();
  virtual void Display(CWBDrawAPI* DrawAPI);

  void RegisterItem(CWBItem* Item);
  void UnRegisterItem(CWBItem* Item);
  CWBItem* FindItemByGuid(WBGUID Guid, const TCHAR* type = nullptr);

  template <typename... Args>
  CWBItem* FindItemByGuids(WBGUID Guid, Args... args) {
    int len = sizeof...(Args);
    const TCHAR* vals[] = {args...};

    if (!len) return FindItemByGuid(Guid);

    for (int x = 0; x < len; x++) {
      auto item = FindItemByGuid(Guid, vals[x]);
      if (item) return item;
    }

    return nullptr;
  }

  virtual void SendMessage(CWBMessage& Message);
  CWBItem* SetCapture(CWBItem* Capturer);
  bool ReleaseCapture();

  CWBItem* GetMouseItem();
  CWBItem* GetMouseCaptureItem();

  bool CreateFont(std::string_view FontName, CWBFontDescription* Font);
  CWBFont* GetFont(std::string_view FontName);
  CWBFont* GetDefaultFont();
  bool SetDefaultFont(std::string_view FontName);
  void AddToTrash(const std::shared_ptr<CWBItem>& item);

  bool GetCtrlState() { return Ctrl; }
  bool GetAltState() { return Alt; }
  bool GetShiftState() { return Shift; }
  bool GetLeftButtonState() { return Left; }
  bool GetRightButtonState() { return Right; }
  bool GetMiddleButtonState() { return Middle; }

  bool LoadXMLLayout(std::string_view XML);
  bool LoadXMLLayoutFromFile(std::string_view FileName);
  bool LoadCSS(std::string_view CSS, bool ResetStyleManager = true);
  bool LoadCSSFromFile(std::string_view FileName,
                       bool ResetStyleManager = true);
  // bool GenerateGUI( CWBItem *Root, const TCHAR *layout );
  bool GenerateGUI(CWBItem* Root, std::string_view Layout);
  bool GenerateGUITemplate(CWBItem* Root, std::string_view Layout,
                           std::string_view TemplateID);
  // bool GenerateGUITemplate( CWBItem *Root, TCHAR *Layout, TCHAR *TemplateID
  // );
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

  CAtlas* GetAtlas();
  CWBSkin* GetSkin();

  void ApplyStyle(CWBItem* Target);
  void ReApplyStyle();

  // void RegisterUIFactoryCallback( TCHAR *ElementName, WBFACTORYCALLBACK
  // FactoryCallback );
  void RegisterUIFactoryCallback(std::string_view ElementName,
                                 WBFACTORYCALLBACK FactoryCallback);

  virtual void TakeScreenshot();
  void SetScreenshotName(std::string_view s) { ScreenShotName = s; }

  void SetVSync(bool vs) { Vsync = vs; }
  void SetClearColor(CColor color) { ClearColor = color; }

  float GetFrameRate();
  int32_t GetInitialKeyboardDelay();
  int32_t GetKeyboardRepeatTime();

  void HandleResize() override;

  LRESULT InjectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
