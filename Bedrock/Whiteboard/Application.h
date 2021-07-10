#pragma once

#include "../CoRE2/Core2.h"
#include "../UtilLib/XMLDocument.h"
#include "Message.h"
#include "DrawAPI.h"
#include "GuiItem.h"
#include "Root.h"
#include "ContextMenu.h"
#include "StyleManager.h"

#include <unordered_map>
#include <vector>

enum WBMOUSECLICKREPEATMODE
{
  WB_MCR_OFF = 0,
  WB_MCR_LEFT = 1,
  WB_MCR_RIGHT = 2,
  WB_MCR_MIDDLE = 3
};

typedef CWBItem* ( __cdecl *WBFACTORYCALLBACK )( CWBItem *Root, CXMLNode &node, CRect &Pos );

class CWBApplication : public CCoreWindowHandlerWin
{
  friend class CWBItem;
  CArray<CWBItem*> MessagePath;

  CRingBuffer<int32_t> *FrameTimes;
  int32_t LastFrameTime;

  std::unordered_map<WBGUID, CWBItem*> Items;
  CArrayThreadSafe<CWBMessage> MessageBuffer;
  CArrayThreadSafe<CWBItem*> Trash;

  CWBItem *MouseCaptureItem;
  CWBItem *MouseItem;

  CWBSkin *Skin;
  std::unordered_map<std::string, std::unique_ptr<CWBFont> > Fonts;
  CWBFont *DefaultFont;

  TBOOL Alt, Ctrl, Shift, Left, Middle, Right;

  TBOOL Vsync;

  std::string ScreenShotName;

  TBOOL SendMessageToItem( CWBMessage &Message, CWBItem *Target );
  void ProcessMessage( CWBMessage &Message );
  CWBItem *GetItemUnderMouse( CPoint &Point, WBMESSAGE w );

  virtual void UpdateMouseItem();
  virtual void CleanTrash();
  virtual void UpdateControlKeyStates(); //update ctrl alt shift states
  virtual int32_t GetKeyboardState();

  WBMOUSECLICKREPEATMODE ClickRepeaterMode;
  int64_t NextRepeatedClickTime;

  //////////////////////////////////////////////////////////////////////////
  // gui layout and style

  std::unordered_map<std::string, std::unique_ptr<CXMLDocument> > LayoutRepository;
  CStyleManager StyleManager;

  //////////////////////////////////////////////////////////////////////////
  // ui generator

  std::unordered_map<std::string, WBFACTORYCALLBACK> FactoryCallbacks;

  TBOOL ProcessGUIXML( CWBItem *Root, CXMLNode & node );
  TBOOL GenerateGUIFromXMLNode( CWBItem * Root, CXMLNode & node, CRect &Pos );
  TBOOL GenerateGUITemplateFromXML(CWBItem *Root, CXMLDocument *doc,
                                   std::string_view TemplateID);
  CWBItem *GenerateUIItem( CWBItem *Root, CXMLNode &node, CRect &Pos );

  CColor ClearColor = CColor( 0, 0, 0, 255 );

protected:

  CAtlas *Atlas;
  CWBRoot *Root;

  CWBDrawAPI *DrawAPI;

  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
  TBOOL GenerateGUIFromXML( CWBItem *Root, CXMLDocument *doc );
  virtual TBOOL Initialize();

public:

  virtual TBOOL Initialize( const CCoreWindowParameters &WindowParams );

  CWBApplication();
  virtual ~CWBApplication();

  CWBItem *GetRoot();
  CWBItem *GetFocusItem();
  virtual TBOOL HandleMessages();
  virtual TBOOL IsDone() { return Done; }
  virtual void SetDone( TBOOL d );
  virtual void Display();
  virtual void Display( CWBDrawAPI *DrawAPI );

  void RegisterItem( CWBItem *Item );
  void UnRegisterItem( CWBItem *Item );
  CWBItem *FindItemByGuid( WBGUID Guid, const TCHAR *type = NULL );

  template< typename... Args>
  CWBItem *FindItemByGuids( WBGUID Guid, Args ... args )
  {
    int len = sizeof...( Args );
    const TCHAR* vals[] = { args... };

    if ( !len )
      return FindItemByGuid( Guid );

    for ( int x = 0; x < len; x++ )
    {
      auto item = FindItemByGuid( Guid, vals[ x ] );
      if ( item )
        return item;
    }

    return nullptr;
  }

  virtual void SendMessage( CWBMessage &Message );
  CWBItem *SetCapture( CWBItem *Capturer );
  TBOOL ReleaseCapture();

  CWBItem *GetMouseItem();
  CWBItem *GetMouseCaptureItem();

  TBOOL CreateFont(std::string_view FontName, CWBFontDescription *Font);
  CWBFont *GetFont(std::string_view FontName);
  CWBFont *GetDefaultFont();
  TBOOL SetDefaultFont(std::string_view FontName);
  void AddToTrash( CWBItem *item );

  TBOOL GetCtrlState() { return Ctrl; }
  TBOOL GetAltState() { return Alt; }
  TBOOL GetShiftState() { return Shift; }
  TBOOL GetLeftButtonState() { return Left; }
  TBOOL GetRightButtonState() { return Right; }
  TBOOL GetMiddleButtonState() { return Middle; }

  TBOOL LoadXMLLayout(std::string_view XML);
  TBOOL LoadXMLLayoutFromFile(std::string_view FileName);
  TBOOL LoadCSS(std::string_view CSS, TBOOL ResetStyleManager = true);
  TBOOL LoadCSSFromFile(std::string_view FileName, TBOOL ResetStyleManager = true);
  // TBOOL GenerateGUI( CWBItem *Root, const TCHAR *layout );
  TBOOL GenerateGUI(CWBItem *Root, std::string_view Layout);
  TBOOL GenerateGUITemplate(CWBItem *Root, std::string_view Layout,
                            std::string_view TemplateID);
  // TBOOL GenerateGUITemplate( CWBItem *Root, TCHAR *Layout, TCHAR *TemplateID );
  TBOOL LoadSkin(std::string_view XML,
                 std::vector<int> &enabledGlyphs = std::vector<int>());
  TBOOL LoadSkinFromFile(std::string_view FileName,
                         std::vector<int> &enabledGlyphs = std::vector<int>());

  CAtlas *GetAtlas();
  CWBSkin *GetSkin();

  void ApplyStyle( CWBItem *Target );
  void ReApplyStyle();

  // void RegisterUIFactoryCallback( TCHAR *ElementName, WBFACTORYCALLBACK FactoryCallback );
  void RegisterUIFactoryCallback(std::string_view ElementName,
                                 WBFACTORYCALLBACK FactoryCallback);

  virtual void TakeScreenshot();
  void SetScreenshotName(std::string_view s) { ScreenShotName = s; }

  void SetVSync( TBOOL vs ) { Vsync = vs; }
  void SetClearColor( CColor color ) { ClearColor = color; }

  float GetFrameRate();
  int32_t GetInitialKeyboardDelay();
  int32_t GetKeyboardRepeatTime();

  virtual void HandleResize();

  LRESULT InjectMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
};
