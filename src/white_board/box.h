#pragma once

#include <cstdint>
#include <string_view>

#include "src/white_board/application.h"

enum class WBBOXAXIS : uint8_t {
  WB_HORIZONTAL = 0,
  WB_VERTICAL,
};

enum class WBBOXARRANGEMENT : uint8_t {
  WB_ARRANGE_NONE = 0,
  WB_ARRANGE_HORIZONTAL,
  WB_ARRANGE_VERTICAL
};

enum class WBBOXSIZING : uint8_t {
  WB_SIZING_KEEP = 0,
  WB_SIZING_FILL,
};

class CWBBox : public CWBItem {
 protected:
  void AddChild(std::unique_ptr<CWBItem>&& Item) override;
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
  CWBBox(CWBItem* Parent, const math::CRect& Pos);
  static inline CWBBox* Create(CWBItem* Parent, const math::CRect& Pos) {
    auto p = std::make_unique<CWBBox>(Parent, Pos);
    CWBBox* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  ~CWBBox() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("box", CWBItem);

  virtual void SetArrangement(WBBOXARRANGEMENT a);
  WBBOXARRANGEMENT GetArrangement();
  virtual void SetSpacing(int32_t s);
  virtual void SetAlignment(WBBOXAXIS axis, WBALIGNMENT align);
  virtual void SetSizing(WBBOXAXIS axis, WBBOXSIZING siz);
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
