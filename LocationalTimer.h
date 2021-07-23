#pragma once
#include <vector>

#include "Bedrock/BaseLib/BaseLib.h"
#include "Bedrock/BaseLib/Sphere.h"
#include "Bedrock/Whiteboard/Application.h"

class LocationalTimer {
 public:
  struct TimerEvent {
    std::string Text;
    int32_t Time;
    int32_t CountdownLength;
    int32_t OnScreenLength;
  };

  int32_t MapID = 0;
  CSphere EnterSphere = CSphere(CVector3(0, 0, 0), 0);
  CSphere ExitSphere = CSphere(CVector3(0, 0, 0), 0);
  CVector3 ResetPoint = CVector3(0, 0, 0);
  int32_t TimerLength = 0;
  int32_t StartDelay = 0;

  std::vector<TimerEvent> Events;

  bool IsRunning = false;
  int32_t StartTime = 0;

  LocationalTimer();
  virtual ~LocationalTimer();

  void Update();
  void ImportData(CXMLNode& node);
};

class TimerDisplay : public CWBItem {
 public:
  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  TimerDisplay(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<TimerDisplay> Create(CWBItem* Parent,
                                                     CRect Position) {
    auto p = std::make_shared<TimerDisplay>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~TimerDisplay() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "TimerDisplay" ), CWBItem);
};

extern std::vector<LocationalTimer> LocationalTimers;
void ImportLocationalTimers();
