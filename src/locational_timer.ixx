module;
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

export module taco.locational_timer;

import math;
import whiteboard;
import xml;

export class LocationalTimer {
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

export class TimerDisplay
    : public gui::CWBGuiType<"TimerDisplay", gui::CWBItem> {
 public:
  TimerDisplay();
  static inline TimerDisplay* Create(gui::CWBItem* Parent,
                                     math::CRect Position) {
    auto p = std::make_unique<TimerDisplay>();
    p->Initialize(Parent, Position);
    TimerDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~TimerDisplay() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::CRect& Pos);

  void OnDraw(gui::CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;
};

export extern std::vector<LocationalTimer> LocationalTimers;
export void ImportLocationalTimers();
