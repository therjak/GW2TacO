#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "GuiItem.h"

#define WB_CONTEXT_SEPARATOR ((void *)(INT_MIN))

class CWBContextItem {
  friend class CWBContextMenu;

  std::string Text;
  int32_t ReturnID;
  TBOOL Separator;
  TBOOL Highlighted;
  TBOOL closesContext;

  std::vector<std::unique_ptr<CWBContextItem>> Children;
  CWBContextItem *CopyOf = nullptr;

  void CopyChildrenFrom(CWBContextItem *itm);

 public:
  CWBContextItem();
  virtual ~CWBContextItem();
  virtual CWBContextItem *AddItem(std::string_view Text, int32_t ID,
                                  TBOOL Highlighted = false,
                                  TBOOL closesContext = true);
  virtual void AddSeparator();
  virtual void SetText(std::string_view text);
  virtual void SetHighlight(TBOOL highlighted);
};

class CWBContextMenu : public CWBItem {
  bool Pushed =
      false;  // used to ignore mouse clicks originating from the opening item
  WBGUID Target = 0;

  // SubMenu is also a child
  CWBContextMenu *SubMenu = nullptr;
  int32_t SubMenuIdx = -1;
  CWBContextMenu *ParentMenu = nullptr;
  std::vector<std::unique_ptr<CWBContextItem>> Items;

  TBOOL MessageProc(CWBMessage &Message) override;
  virtual void ResizeToContentSize();
  void OnDraw(CWBDrawAPI *API) override;
  void SpawnSubMenu(int32_t itemidx);
  CRect GetItemRect(int32_t idx);
  void MarkParentForDeletion();

  TBOOL MouseInContextHierarchy();
  CWBContextMenu *GetContextRoot();
  TBOOL AllowMouseHighlightWhileCaptureItem() override { return true; }

  CWBCSSPropertyBatch SeparatorElements;
  void MarkForDeletion() override;

 public:
  CWBContextMenu(CWBItem *Parent, const CRect &Pos, WBGUID Target);
  static inline std::shared_ptr<CWBContextMenu> Create(CWBItem *Parent,
                                                       const CRect &Pos,
                                                       WBGUID Target) {
    auto p = std::make_shared<CWBContextMenu>(Parent, Pos, Target);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBContextMenu() override;

  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position,
                           WBGUID Target);

  WB_DECLARE_GUIITEM(_T( "contextmenu" ), CWBItem);

  virtual CWBContextItem *AddItem(std::string_view Text, int32_t ID,
                                  TBOOL Highlighted = false,
                                  TBOOL closesContext = true);
  virtual void AddSeparator();

  TBOOL ApplyStyle(std::string_view prop, std::string_view value,
                   const std::vector<std::string> &pseudo) override;
  virtual CWBContextItem *GetItem(int32_t ID);
};
