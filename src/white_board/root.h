#pragma once
#include <memory>

#include "src/white_board/gui_item.h"

class CWBRoot : public CWBItem {
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  CWBRoot(CWBItem* Parent, const CRect& Pos);
  static inline std::unique_ptr<CWBRoot> Create(const CRect& Pos) {
    return std::make_unique<CWBRoot>(nullptr, Pos);
  }
  ~CWBRoot() override;

  void SetApplication(CWBApplication* Application);
  bool Initialize(CWBItem* Parent, const CRect& Position) override;

  WB_DECLARE_GUIITEM(_T( "root" ), CWBItem);
};
