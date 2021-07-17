#pragma once
#include <memory>

#include "GuiItem.h"

class CWBRoot : public CWBItem {
  TBOOL MessageProc(CWBMessage &Message) override;
  void OnDraw(CWBDrawAPI *API) override;

 public:
  CWBRoot(CWBItem *Parent, const CRect &Pos);
  static inline std::unique_ptr<CWBRoot> Create(const CRect &Pos) {
    return std::unique_ptr<CWBRoot>(new CWBRoot(nullptr, Pos));
  }
  ~CWBRoot() override;

  void SetApplication(CWBApplication *Application);
  TBOOL Initialize(CWBItem *Parent, const CRect &Position) override;

  WB_DECLARE_GUIITEM(_T( "root" ), CWBItem);
};
