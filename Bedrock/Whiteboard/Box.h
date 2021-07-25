#pragma once

#include <string_view>

#include "Application.h"

enum WBBOXAXIS {
  WB_HORIZONTAL = 0,
  WB_VERTICAL,
};

enum WBBOXARRANGEMENT {
  WB_ARRANGE_NONE = 0,
  WB_ARRANGE_HORIZONTAL,
  WB_ARRANGE_VERTICAL
};

enum WBBOXSIZING {
  WB_SIZING_KEEP = 0,
  WB_SIZING_FILL,
};

class CWBBox : public CWBItem {
 protected:
  void AddChild(const std::shared_ptr<CWBItem>& Item) override;
  bool MessageProc(const CWBMessage& Message) override;
  virtual void RearrangeChildren();

  void OnDraw(CWBDrawAPI* API) override;
  void RearrangeHorizontal();
  void RearrangeVertical();
  void UpdateScrollbarData();

  int32_t Spacing;
  WBBOXARRANGEMENT Arrangement;
  WBALIGNMENT AlignmentX, AlignmentY;
  WBBOXSIZING SizingX, SizingY;
  bool ClickThrough = false;

 public:
  CWBBox(CWBItem* Parent, const CRect& Pos);
  static inline std::shared_ptr<CWBBox> Create(CWBItem* Parent,
                                               const CRect& Pos) {
    auto p = std::make_shared<CWBBox>(Parent, Pos);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }

  ~CWBBox() override;

  bool Initialize(CWBItem* Parent, const CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "box" ), CWBItem);

  virtual void SetArrangement(WBBOXARRANGEMENT a);
  WBBOXARRANGEMENT GetArrangement();
  virtual void SetSpacing(int32_t s);
  virtual void SetAlignment(WBBOXAXIS axis, WBALIGNMENT align);
  virtual void SetSizing(WBBOXAXIS axis, WBBOXSIZING siz);
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;
  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
