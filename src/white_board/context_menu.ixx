module;

#include <cassert>
#include <climits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module whiteboard:context_menu;

import :gui_item;

export namespace gui {

class ContextItem {
  friend class ContextMenu;

 public:
  ContextItem();
  virtual ~ContextItem();
  virtual ContextItem* AddItem(std::string_view Text, int32_t ID,
                               bool Highlighted = false,
                               bool closesContext = true);
  virtual void AddSeparator();
  virtual void SetText(std::string_view text);
  virtual void SetHighlight(bool highlighted);

 private:
  void CopyChildrenFrom(ContextItem* itm);
  std::string text_;
  int32_t return_id_;
  bool separator_;
  bool highlighted_{};
  bool closes_context_{};

  std::vector<std::unique_ptr<ContextItem>> children_;
  ContextItem* copy_of_ = nullptr;
};

class ContextMenu : public GuiType<"contextmenu", Widget> {
 public:
  ContextMenu(WBGUID Target);
  static inline ContextMenu* Create(Widget* Parent, const math::Rect& Pos,
                                    WBGUID Target) {
    auto p = std::make_unique<ContextMenu>(Target);
    p->Initialize(Parent, Pos);
    ContextMenu* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~ContextMenu() override;

  bool Initialize(Widget* Parent, const math::Rect& Position) override;

  virtual ContextItem* AddItem(std::string_view Text, int32_t ID,
                               bool Highlighted = false,
                               bool closesContext = true);
  virtual void AddSeparator();

  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;
  virtual ContextItem* GetItem(int32_t ID);

 private:
  bool MessageProc(const CWBMessage& Message) override;
  virtual void ResizeToContentSize();
  void OnDraw(DrawAPI* API) override;
  void SpawnSubMenu(int32_t itemidx);
  math::Rect GetItemRect(int32_t idx);
  void MarkParentForDeletion();
  bool MouseInContextHierarchy();
  ContextMenu* GetContextRoot();
  bool AllowMouseHighlightWhileCaptureItem() const override { return true; }
  void MarkForDeletion() override;

  // used to ignore mouse clicks originating from the opening item
  bool pushed_ = false;
  WBGUID target_ = 0;

  // SubMenu is also a child
  ContextMenu* sub_menu_ = nullptr;
  int32_t sub_menu_idx_ = -1;
  ContextMenu* parent_menu_ = nullptr;
  std::vector<std::unique_ptr<ContextItem>> items_;

  CssPropertyBatch separator_elements_;
};

}  // namespace gui
