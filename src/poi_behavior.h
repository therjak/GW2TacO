#pragma once
#include <cstdint>

enum class POIBehavior : int32_t {
  AlwaysVisible,
  ReappearOnMapChange,
  ReappearOnDailyReset,
  OnlyVisibleBeforeActivation,
  ReappearAfterTimer,
  ReappearOnMapReset,
  OncePerInstance,
  DailyPerChar,
  OncePerInstancePerChar,
  WvWObjective,
};