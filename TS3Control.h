#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class TS3Control : public CWBItem {
  CPoint lastpos;
  virtual void OnDraw(CWBDrawAPI *API);

 public:
  TS3Control(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<TS3Control> Create(CWBItem *Parent,
                                                   CRect Position) {
    auto p = std::make_shared<TS3Control>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  virtual ~TS3Control();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "ts3control" ), CWBItem);

  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);
};
