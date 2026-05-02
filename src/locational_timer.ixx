module;
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

export module taco.locational_timer;

import math;
import whiteboard;
import xml;

export class LocationalTimer {
 public:
  struct TimerEvent {
    std::string text;
    int32_t time = 0;
    int32_t countdown_length = 0;
    int32_t on_screen_length = 0;
  };

  LocationalTimer();
  virtual ~LocationalTimer();

  void Update();
  void ImportData(const CXMLNode& node);

  int32_t map_id_ = 0;
  math::CSphere enter_sphere_ = math::CSphere(math::Vector3(0, 0, 0), 0);
  math::CSphere exit_sphere_ = math::CSphere(math::Vector3(0, 0, 0), 0);
  math::Vector3 reset_point_ = math::Vector3(0, 0, 0);
  int32_t timer_length_ = 0;
  int32_t start_delay_ = 0;

  std::vector<TimerEvent> events_;

  bool is_running_ = false;
  int32_t start_time_ = 0;
};

export class TimerDisplay
    : public gui::CWBGuiType<"TimerDisplay", gui::CWBItem> {
 public:
  TimerDisplay();
  static inline TimerDisplay* Create(gui::CWBItem* parent,
                                     math::Rect position) {
    auto p = std::make_unique<TimerDisplay>();
    p->Initialize(parent, position);
    TimerDisplay* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~TimerDisplay() override;

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  void OnDraw(gui::CWBDrawAPI* api) override;
  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;
};

export std::vector<LocationalTimer> LocationalTimers;
export void ImportLocationalTimers();
