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
using math::Point;
using math::Rect;
using math::Vector2;
using math::Vector3;
using math::Vector4;
using math::PI;

void GW2RangeDisplay::DrawRangeCircle(gui::CWBDrawAPI* API, float range,
                                      float alpha) {
  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.isPvp) return;
  if (mumbleLink.isMapOpen) return;

  Rect drawrect = GetClientRect();

  Matrix4x4 cam;
  cam.SetLookAtLH(mumbleLink.camPosition,
                  mumbleLink.camPosition + mumbleLink.camDir,
                  Vector3(0, 1, 0));
  Matrix4x4 persp;
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, drawrect.Width() / static_cast<float>(drawrect.Height()),
      0.01f, 1000.0f);

  int32_t resolution = 60;

  Vector4 charpos = Vector4(mumbleLink.averagedCharPosition.x,
                              mumbleLink.averagedCharPosition.y,
                              mumbleLink.averagedCharPosition.z, 1.0f);
  ;
  float rworld = GameToWorldCoords(range);

  Vector4 camSpaceChar = charpos;
  Vector4 camSpaceEye = charpos + Vector4(0, 3, 0, 0);

  Vector4 screenSpaceChar = (camSpaceChar * cam) * persp;
  screenSpaceChar /= screenSpaceChar.w;
  Vector4 screenSpaceEye = (camSpaceEye * cam) * persp;
  screenSpaceEye /= screenSpaceEye.w;

  auto pos =
      (Vector3(mumbleLink.averagedCharPosition) - mumbleLink.camPosition);
  pos.y = 0;
  bool zoomedin = pos.Length() < 0.13;

  Vector4 campos = Vector4(mumbleLink.camPosition.x, mumbleLink.camPosition.y,
                             mumbleLink.camPosition.z, 1.0f);
  Vector2 camDir =
      Vector2(camSpaceChar.x - campos.x, camSpaceChar.z - campos.z)
          .Normalized();

  Vector3 toChar(charpos - campos);

  for (int x = 0; x < resolution; x++) {
    float a1 = 1.0f;
    float a2 = 1.0f;
    float f1 = x / static_cast<float>(resolution) * PI * 2;
    float f2 = (x + 1) / static_cast<float>(resolution) * PI * 2;
    Vector4 p1 =
        Vector4(rworld * std::sin(f1), 0, rworld * std::cos(f1), 0.0f);
    Vector4 p2 =
        Vector4(rworld * std::sin(f2), 0, rworld * std::cos(f2), 0.0f);

    Vector3 toPoint(p1 - campos);

    if (!zoomedin) {
      a1 = 1 -
           std::pow(std::max(0.f, camDir * Vector2(p1.x, p1.z).Normalized()),
                    10.0f);
      a2 = 1 -
           std::pow(std::max(0.f, camDir * Vector2(p2.x, p2.z).Normalized()),
                    10.0f);
    }

    p1 = p1 + charpos;
    p2 = p2 + charpos;

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
      a1 = 1 - (1 - a1) * (1 - powf((p1.y - screenSpaceChar.y) /
                                        (screenSpaceEye.y - screenSpaceChar.y),
                                    10.0f));
    }

    if (a2 < 1) {
      a2 = 1 - (1 - a2) * (1 - powf((p2.y - screenSpaceChar.y) /
                                        (screenSpaceEye.y - screenSpaceChar.y),
                                    10.0f));
    }

    p1 = p1 * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);
    p2 = p2 * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);

    Point pa = Point(static_cast<int>(p1.x * drawrect.Width()),
                       static_cast<int>((1 - p1.y) * drawrect.Height()));
    Point pb = Point(static_cast<int>(p2.x * drawrect.Width()),
                       static_cast<int>((1 - p2.y) * drawrect.Height()));

    a1 = std::max(0.f, std::min(1.f, a1)) * alpha * 255.f;
    a2 = std::max(0.f, std::min(1.f, a2)) * alpha * 255.f;

    API->DrawLine(pa, pb, CColor(228, 210, 157, static_cast<uint8_t>(a1)),
                  CColor(228, 210, 157, static_cast<uint8_t>(a2)));
  }
}

void GW2RangeDisplay::OnDraw(gui::CWBDrawAPI* API) {
  if (!mumbleLink.IsValid()) return;

  if (GetConfigValue("RangeCirclesVisible")) {
    float circ = GetConfigValue("RangeCircleTransparency") / 100.0f;
    if (GetConfigValue("RangeCircle90")) DrawRangeCircle(API, 90, circ);
    if (GetConfigValue("RangeCircle120")) DrawRangeCircle(API, 120, circ);
    if (GetConfigValue("RangeCircle180")) DrawRangeCircle(API, 180, circ);
    if (GetConfigValue("RangeCircle240")) DrawRangeCircle(API, 240, circ);
    if (GetConfigValue("RangeCircle300")) DrawRangeCircle(API, 300, circ);
    if (GetConfigValue("RangeCircle400")) DrawRangeCircle(API, 400, circ);
    if (GetConfigValue("RangeCircle600")) DrawRangeCircle(API, 600, circ);
    if (GetConfigValue("RangeCircle900")) DrawRangeCircle(API, 900, circ);
    if (GetConfigValue("RangeCircle1200")) DrawRangeCircle(API, 1200, circ);
    if (GetConfigValue("RangeCircle1500")) DrawRangeCircle(API, 1500, circ);
    if (GetConfigValue("RangeCircle1600")) DrawRangeCircle(API, 1600, circ);
  }
}

GW2RangeDisplay::GW2RangeDisplay() : CWBGuiType() {}
GW2RangeDisplay::~GW2RangeDisplay() = default;

gui::CWBItem* GW2RangeDisplay::Factory(gui::CWBItem* Root, const CXMLNode& node,
                                       Rect& Pos) {
  return GW2RangeDisplay::Create(Root, Pos);
}

bool GW2RangeDisplay::IsMouseTransparent(const Point& ClientSpacePoint,
                                         gui::WBMESSAGE MessageType) {
  return true;
}
