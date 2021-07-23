#pragma once
#include "Bedrock/Whiteboard/Application.h"

class TS3Control : public CWBItem {
  CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  TS3Control(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<TS3Control> Create(CWBItem* Parent,
                                                   CRect Position) {
    auto p = std::make_shared<TS3Control>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~TS3Control() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "ts3control" ), CWBItem);

  bool IsMouseTransparent(CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
