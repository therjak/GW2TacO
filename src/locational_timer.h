#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/sphere.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class LocationalTimer {
 public:
  struct TimerEvent {
    std::string Text;
    int32_t Time = 0;
    int32_t CountdownLength = 0;
    int32_t OnScreenLength = 0;
  };

  LocationalTimer();
  virtual ~LocationalTimer();

  void Update();
  void ImportData(const CXMLNode& node);

  int32_t MapID = 0;
  math::CSphere EnterSphere = math::CSphere(math::CVector3(0, 0, 0), 0);
  math::CSphere ExitSphere = math::CSphere(math::CVector3(0, 0, 0), 0);
  math::CVector3 ResetPoint = math::CVector3(0, 0, 0);
  int32_t TimerLength = 0;
  int32_t StartDelay = 0;

  std::vector<TimerEvent> Events;

  bool IsRunning = false;
  int32_t StartTime = 0;
};

class TimerDisplay : public CWBItem {
 public:
  TimerDisplay(CWBItem* Parent, math::CRect Position);
  static inline TimerDisplay* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<TimerDisplay>(Parent, Position);
    TimerDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~TimerDisplay() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("TimerDisplay", CWBItem);

  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};

extern std::vector<LocationalTimer> LocationalTimers;
void ImportLocationalTimers();
