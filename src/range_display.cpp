module;
#include <algorithm>
#include <cmath>

#include "src/gw2_tactical.h"

module taco.range_display;

import math;
import taco.mumble_link;
import taco.overlay_config;
import whiteboard;
import xml;

using math::Matrix4x4;
using math::PI;
using math::Point;
using math::Rect;
using math::Vector2;
using math::Vector3;
using math::Vector4;

void GW2RangeDisplay::DrawRangeCircle(gui::CWBDrawAPI* api, float range,
                                      float alpha) {
  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.is_pvp) return;
  if (mumbleLink.is_map_open) return;

  Rect draw_rect = GetClientRect();

  Matrix4x4 cam;
  cam.SetLookAtLH(mumbleLink.cam_position,
                  mumbleLink.cam_position + mumbleLink.cam_dir,
                  Vector3(0, 1, 0));
  Matrix4x4 persp;
  persp.SetPerspectiveFovLH(
      mumbleLink.fov,
      draw_rect.Width() / static_cast<float>(draw_rect.Height()), 0.01f,
      1000.0f);

  int32_t resolution = 60;

  Vector4 char_pos = Vector4(mumbleLink.averaged_char_position.x,
                             mumbleLink.averaged_char_position.y,
                             mumbleLink.averaged_char_position.z, 1.0f);

  float r_world = GameToWorldCoords(range);

  Vector4 cam_space_char = char_pos;
  Vector4 cam_space_eye = char_pos + Vector4(0, 3, 0, 0);

  Vector4 screen_space_char = (cam_space_char * cam) * persp;
  screen_space_char /= screen_space_char.w;
  Vector4 screen_space_eye = (cam_space_eye * cam) * persp;
  screen_space_eye /= screen_space_eye.w;

  auto pos =
      (Vector3(mumbleLink.averaged_char_position) - mumbleLink.cam_position);
  pos.y = 0;
  bool zoomed_in = pos.Length() < 0.13;

  Vector4 cam_pos =
      Vector4(mumbleLink.cam_position.x, mumbleLink.cam_position.y,
              mumbleLink.cam_position.z, 1.0f);
  Vector2 cam_dir =
      Vector2(cam_space_char.x - cam_pos.x, cam_space_char.z - cam_pos.z)
          .Normalized();

  Vector3 to_char(char_pos - cam_pos);

  for (int x = 0; x < resolution; x++) {
    float a1 = 1.0f;
    float a2 = 1.0f;
    float f1 = x / static_cast<float>(resolution) * PI * 2;
    float f2 = (x + 1) / static_cast<float>(resolution) * PI * 2;
    Vector4 p1 =
        Vector4(r_world * std::sin(f1), 0, r_world * std::cos(f1), 0.0f);
    Vector4 p2 =
        Vector4(r_world * std::sin(f2), 0, r_world * std::cos(f2), 0.0f);

    Vector3 to_point(p1 - cam_pos);

    if (!zoomed_in) {
      a1 = 1 -
           std::pow(std::max(0.f, cam_dir * Vector2(p1.x, p1.z).Normalized()),
                    10.0f);
      a2 = 1 -
           std::pow(std::max(0.f, cam_dir * Vector2(p2.x, p2.z).Normalized()),
                    10.0f);
    }

    p1 = p1 + char_pos;
    p2 = p2 + char_pos;

    p1 = p1 * cam;
    p2 = p2 * cam;

    p1 /= p1.w;
    p2 /= p2.w;

    if (p1.z < 0.01 && p2.z < 0.01) continue;

    p1.z = std::max(0.01f, p1.z);
    p2.z = std::max(0.01f, p2.z);

    p1 = p1 * persp;
    p2 = p2 * persp;
    p1 /= p1.w;
    p2 /= p2.w;

    if (a1 < 1) {
      a1 = 1 -
           (1 - a1) * (1 - powf((p1.y - screen_space_char.y) /
                                    (screen_space_eye.y - screen_space_char.y),
                                10.0f));
    }

    if (a2 < 1) {
      a2 = 1 -
           (1 - a2) * (1 - powf((p2.y - screen_space_char.y) /
                                    (screen_space_eye.y - screen_space_char.y),
                                10.0f));
    }

    p1 = p1 * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);
    p2 = p2 * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);

    Point pa = Point(static_cast<int>(p1.x * draw_rect.Width()),
                     static_cast<int>((1 - p1.y) * draw_rect.Height()));
    Point pb = Point(static_cast<int>(p2.x * draw_rect.Width()),
                     static_cast<int>((1 - p2.y) * draw_rect.Height()));

    a1 = std::max(0.f, std::min(1.f, a1)) * alpha * 255.f;
    a2 = std::max(0.f, std::min(1.f, a2)) * alpha * 255.f;

    api->DrawLine(pa, pb, CColor(228, 210, 157, static_cast<uint8_t>(a1)),
                  CColor(228, 210, 157, static_cast<uint8_t>(a2)));
  }
}

void GW2RangeDisplay::OnDraw(gui::CWBDrawAPI* api) {
  if (!mumbleLink.IsValid()) return;

  if (GetConfigValue("RangeCirclesVisible")) {
    float circ = GetConfigValue("RangeCircleTransparency") / 100.0f;
    if (GetConfigValue("RangeCircle90")) DrawRangeCircle(api, 90, circ);
    if (GetConfigValue("RangeCircle120")) DrawRangeCircle(api, 120, circ);
    if (GetConfigValue("RangeCircle180")) DrawRangeCircle(api, 180, circ);
    if (GetConfigValue("RangeCircle240")) DrawRangeCircle(api, 240, circ);
    if (GetConfigValue("RangeCircle300")) DrawRangeCircle(api, 300, circ);
    if (GetConfigValue("RangeCircle400")) DrawRangeCircle(api, 400, circ);
    if (GetConfigValue("RangeCircle600")) DrawRangeCircle(api, 600, circ);
    if (GetConfigValue("RangeCircle900")) DrawRangeCircle(api, 900, circ);
    if (GetConfigValue("RangeCircle1200")) DrawRangeCircle(api, 1200, circ);
    if (GetConfigValue("RangeCircle1500")) DrawRangeCircle(api, 1500, circ);
    if (GetConfigValue("RangeCircle1600")) DrawRangeCircle(api, 1600, circ);
  }
}

GW2RangeDisplay::GW2RangeDisplay() : CWBGuiType() {}
GW2RangeDisplay::~GW2RangeDisplay() = default;

gui::CWBItem* GW2RangeDisplay::Factory(gui::CWBItem* root, const CXMLNode& node,
                                       Rect& pos) {
  return GW2RangeDisplay::Create(root, pos);
}

bool GW2RangeDisplay::IsMouseTransparent(const Point& client_space_point,
                                         gui::WBMESSAGE message_type) {
  return true;
}
