﻿#include "src/range_display.h"

#include <algorithm>

#include "src/mumble_link.h"
#include "src/overlay_config.h"

using math::CMatrix4x4;
using math::CPoint;
using math::CRect;
using math::CVector2;
using math::CVector3;
using math::CVector4;

constexpr float PI = 3.1415926535897932384626433832795f;

void GW2RangeDisplay::DrawRangeCircle(CWBDrawAPI* API, float range,
                                      float alpha) {
  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.isPvp) return;
  if (mumbleLink.isMapOpen) return;

  CRect drawrect = GetClientRect();

  CMatrix4x4 cam;
  cam.SetLookAtLH(mumbleLink.camPosition,
                  mumbleLink.camPosition + mumbleLink.camDir,
                  CVector3(0, 1, 0));
  CMatrix4x4 persp;
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, drawrect.Width() / static_cast<float>(drawrect.Height()),
      0.01f, 1000.0f);

  int32_t resolution = 60;

  CVector4 charpos = CVector4(mumbleLink.averagedCharPosition.x,
                              mumbleLink.averagedCharPosition.y,
                              mumbleLink.averagedCharPosition.z, 1.0f);
  ;
  float rworld = GameToWorldCoords(range);

  CVector4 camSpaceChar = charpos;
  CVector4 camSpaceEye = charpos + CVector4(0, 3, 0, 0);

  CVector4 screenSpaceChar = (camSpaceChar * cam) * persp;
  screenSpaceChar /= screenSpaceChar.w;
  CVector4 screenSpaceEye = (camSpaceEye * cam) * persp;
  screenSpaceEye /= screenSpaceEye.w;

  auto pos =
      (CVector3(mumbleLink.averagedCharPosition) - mumbleLink.camPosition);
  pos.y = 0;
  bool zoomedin = pos.Length() < 0.13;

  CVector4 campos = CVector4(mumbleLink.camPosition.x, mumbleLink.camPosition.y,
                             mumbleLink.camPosition.z, 1.0f);
  CVector2 camDir =
      CVector2(camSpaceChar.x - campos.x, camSpaceChar.z - campos.z)
          .Normalized();

  CVector3 toChar(charpos - campos);

  for (int x = 0; x < resolution; x++) {
    float a1 = 1.0f;
    float a2 = 1.0f;
    float f1 = x / static_cast<float>(resolution) * PI * 2;
    float f2 = (x + 1) / static_cast<float>(resolution) * PI * 2;
    CVector4 p1 =
        CVector4(rworld * std::sin(f1), 0, rworld * std::cos(f1), 0.0f);
    CVector4 p2 =
        CVector4(rworld * std::sin(f2), 0, rworld * std::cos(f2), 0.0f);

    CVector3 toPoint(p1 - campos);

    if (!zoomedin) {
      a1 = 1 -
           std::pow(std::max(0.f, camDir * CVector2(p1.x, p1.z).Normalized()),
                    10.0f);
      a2 = 1 -
           std::pow(std::max(0.f, camDir * CVector2(p2.x, p2.z).Normalized()),
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

    if (a1 < 1)
      a1 = 1 - (1 - a1) * (1 - powf((p1.y - screenSpaceChar.y) /
                                        (screenSpaceEye.y - screenSpaceChar.y),
                                    10.0f));

    if (a2 < 1)
      a2 = 1 - (1 - a2) * (1 - powf((p2.y - screenSpaceChar.y) /
                                        (screenSpaceEye.y - screenSpaceChar.y),
                                    10.0f));

    p1 = p1 * 0.5 + CVector4(0.5, 0.5, 0.5, 0.0);
    p2 = p2 * 0.5 + CVector4(0.5, 0.5, 0.5, 0.0);

    CPoint pa = CPoint(static_cast<int>(p1.x * drawrect.Width()),
                       static_cast<int>((1 - p1.y) * drawrect.Height()));
    CPoint pb = CPoint(static_cast<int>(p2.x * drawrect.Width()),
                       static_cast<int>((1 - p2.y) * drawrect.Height()));

    a1 = std::max(0.f, std::min(1.f, a1)) * alpha * 255.f;
    a2 = std::max(0.f, std::min(1.f, a2)) * alpha * 255.f;

    API->DrawLine(pa, pb, CColor(228, 210, 157, static_cast<uint8_t>(a1)),
                  CColor(228, 210, 157, static_cast<uint8_t>(a2)));
  }
}

void GW2RangeDisplay::OnDraw(CWBDrawAPI* API) {
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

GW2RangeDisplay::GW2RangeDisplay(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}
GW2RangeDisplay::~GW2RangeDisplay() = default;

CWBItem* GW2RangeDisplay::Factory(CWBItem* Root, const CXMLNode& node,
                                  CRect& Pos) {
  return GW2RangeDisplay::Create(Root, Pos);
}

bool GW2RangeDisplay::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                         WBMESSAGE MessageType) {
  return true;
}
