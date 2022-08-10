#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "src/white_board/gui_item.h"

#define WB_CONTEXT_SEPARATOR ((void*)(INT_MIN))

class CWBContextItem {
  friend class CWBContextMenu;

  std::string Text;
  int32_t ReturnID;
  bool Separator;
  bool Highlighted;
  bool closesContext;

  std::vector<std::unique_ptr<CWBContextItem>> Children;
  CWBContextItem* CopyOf = nullptr;

  void CopyChildrenFrom(CWBContextItem* itm);

 public:
  CWBContextItem();
  virtual ~CWBContextItem();
  virtual CWBContextItem* AddItem(std::string_view Text, int32_t ID,
                                  bool Highlighted = false,
                                  bool closesContext = true);
  virtual void AddSeparator();
  virtual void SetText(std::string_view text);
  virtual void SetHighlight(bool highlighted);
};

class CWBContextMenu : public CWBItem {
  // used to ignore mouse clicks originating from the opening item
  bool Pushed = false;
  WBGUID Target = 0;

  // SubMenu is also a child
  CWBContextMenu* SubMenu = nullptr;
  int32_t SubMenuIdx = -1;
  CWBContextMenu* ParentMenu = nullptr;
  std::vector<std::unique_ptr<CWBContextItem>> Items;

  bool MessageProc(const CWBMessage& Message) override;
  virtual void ResizeToContentSize();
  void OnDraw(CWBDrawAPI* API) override;
  void SpawnSubMenu(int32_t itemidx);
  math::CRect GetItemRect(int32_t idx);
  void MarkParentForDeletion();

  bool MouseInContextHierarchy();
  CWBContextMenu* GetContextRoot();
  bool AllowMouseHighlightWhileCaptureItem() override { return true; }

  CWBCSSPropertyBatch SeparatorElements;
  void MarkForDeletion() override;

 public:
  CWBContextMenu(CWBItem* Parent, const math::CRect& Pos, WBGUID Target);
  static inline CWBContextMenu* Create(CWBItem* Parent, const math::CRect& Pos,
                                       WBGUID Target) {
    auto p = std::make_unique<CWBContextMenu>(Parent, Pos, Target);
    CWBContextMenu* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBContextMenu() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  WB_DECLARE_GUIITEM("contextmenu", CWBItem);

  virtual CWBContextItem* AddItem(std::string_view Text, int32_t ID,
                                  bool Highlighted = false,
                                  bool closesContext = true);
  virtual void AddSeparator();

  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;
  virtual CWBContextItem* GetItem(int32_t ID);
};
