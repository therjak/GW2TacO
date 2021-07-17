#pragma once
#include <memory>

#include "GuiItem.h"

class CWBRoot : public CWBItem {
  TBOOL MessageProc(CWBMessage &Message);
  virtual void OnDraw(CWBDrawAPI *API);

 public:
  CWBRoot(CWBItem *Parent, const CRect &Pos);
  static inline std::unique_ptr<CWBRoot> Create(const CRect &Pos) {
    return std::unique_ptr<CWBRoot>(new CWBRoot(nullptr, Pos));
  }
  virtual ~CWBRoot();

  void SetApplication(CWBApplication *Application);
  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position);

  WB_DECLARE_GUIITEM(_T( "root" ), CWBItem);
};
