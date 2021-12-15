﻿#include "TacticalCompass.h"

#include <algorithm>

#include "Language.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"

constexpr float PI = 3.1415926535897932384626433832795f;

float GetMapFade();

void GW2TacticalCompass::DrawTacticalCompass(CWBDrawAPI* API) {
  CRect drawrect = GetClientRect();

  CMatrix4x4 cam;
  cam.SetLookAtLH(mumbleLink.camPosition,
                  mumbleLink.camPosition + mumbleLink.camDir,
                  CVector3(0, 1, 0));
  CMatrix4x4 persp;
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, drawrect.Width() / static_cast<float>(drawrect.Height()),
      0.01f, 1000.0f);

  CVector4 charpos = CVector4(mumbleLink.averagedCharPosition.x,
                              mumbleLink.averagedCharPosition.y,
                              mumbleLink.averagedCharPosition.z, 1.0f);
  ;
  float rworld = GameToWorldCoords(40);

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

  CWBFont* f = GetFont(GetState());

  std::string txt[4] = {DICT("compassnorth"), DICT("compasseast"),
                        DICT("compasssouth"), DICT("compasswest")};

  for (int x = 0; x < 4; x++) {
    float a1 = 1.0f;
    float f1 = x / static_cast<float>(4) * PI * 2;
    CVector4 p1 = CVector4(rworld * sinf(f1), 1.0f, rworld * cosf(f1), 0.0f);

    if (!zoomedin)
      a1 = 1 -
           std::pow(std::max(0.f, camDir * CVector2(p1.x, p1.z).Normalized()),
                    10.0f);

    p1 = p1 + charpos;
    p1 = p1 * cam;
    p1 /= p1.w;

    if (p1.z < 0.01) continue;

    p1.z = std::max(0.01f, p1.z);
    p1 = p1 * persp;
    p1 /= p1.w;

    if (a1 < 1)
      a1 = 1 - (1 - a1) * (1 - powf((p1.y - screenSpaceChar.y) /
                                        (screenSpaceEye.y - screenSpaceChar.y),
                                    10.0f));

    p1 = p1 * 0.5 + CVector4(0.5, 0.5, 0.5, 0.0);

    CPoint pa = CPoint(static_cast<int>(p1.x * drawrect.Width()),
                       static_cast<int>((1 - p1.y) * drawrect.Height()));

    a1 = std::max(0.f, std::min(1.f, a1)) * 255.f;

    CRect cent = CRect(pa, pa);
    CPoint p = f->GetCenter(txt[x], cent);
    f->Write(API, txt[x], p,
             CColor(228, 210, 157, static_cast<uint8_t>(a1 * GetMapFade())));
  }
}

void GW2TacticalCompass::OnDraw(CWBDrawAPI* API) {
  if (!mumbleLink.IsValid()) return;

  if (!HasConfigValue("TacticalCompassVisible"))
    SetConfigValue("TacticalCompassVisible", 0);

  if (GetConfigValue("TacticalCompassVisible")) DrawTacticalCompass(API);
}

GW2TacticalCompass::GW2TacticalCompass(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}
GW2TacticalCompass::~GW2TacticalCompass() = default;

CWBItem* GW2TacticalCompass::Factory(CWBItem* Root, const CXMLNode& node,
                                     CRect& Pos) {
  return GW2TacticalCompass::Create(Root, Pos).get();
}

bool GW2TacticalCompass::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                            WBMESSAGE MessageType) {
  return true;
}
