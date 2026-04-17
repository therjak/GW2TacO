module;
#include <algorithm>
#include <cmath>

#include "src/gw2_tactical.h"

module taco.tactical_compass;

import math;
import taco.language;
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

float GetMapFade();

void GW2TacticalCompass::DrawTacticalCompass(gui::CWBDrawAPI* api) {
  Rect draw_rect = GetClientRect();

  Matrix4x4 cam;
  cam.SetLookAtLH(mumbleLink.camPosition,
                  mumbleLink.camPosition + mumbleLink.camDir,
                  Vector3(0, 1, 0));
  Matrix4x4 persp;
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, draw_rect.Width() / static_cast<float>(draw_rect.Height()),
      0.01f, 1000.0f);

  Vector4 char_pos = Vector4(mumbleLink.averagedCharPosition.x,
                              mumbleLink.averagedCharPosition.y,
                              mumbleLink.averagedCharPosition.z, 1.0f);
  ;
  float r_world = GameToWorldCoords(40);

  Vector4 cam_space_char = char_pos;
  Vector4 cam_space_eye = char_pos + Vector4(0, 3, 0, 0);

  Vector4 screen_space_char = (cam_space_char * cam) * persp;
  screen_space_char /= screen_space_char.w;
  Vector4 screen_space_eye = (cam_space_eye * cam) * persp;
  screen_space_eye /= screen_space_eye.w;

  auto pos =
      (Vector3(mumbleLink.averagedCharPosition) - mumbleLink.camPosition);
  pos.y = 0;
  bool zoomed_in = pos.Length() < 0.13;

  Vector4 cam_pos = Vector4(mumbleLink.camPosition.x, mumbleLink.camPosition.y,
                             mumbleLink.camPosition.z, 1.0f);
  Vector2 cam_dir =
      Vector2(cam_space_char.x - cam_pos.x, cam_space_char.z - cam_pos.z)
          .Normalized();

  gui::CWBFont* f = GetFont(GetState());

  std::string txt[4] = {DICT("compassnorth"), DICT("compasseast"),
                        DICT("compasssouth"), DICT("compasswest")};

  for (int x = 0; x < 4; x++) {
    float a1 = 1.0f;
    float f1 = x / static_cast<float>(4) * PI * 2;
    Vector4 p1 = Vector4(r_world * sinf(f1), 1.0f, r_world * cosf(f1), 0.0f);

    if (!zoomed_in) {
      a1 = 1 -
           std::pow(std::max(0.f, cam_dir * Vector2(p1.x, p1.z).Normalized()),
                    10.0f);
    }

    p1 = p1 + char_pos;
    p1 = p1 * cam;
    p1 /= p1.w;

    if (p1.z < 0.01) continue;

    p1.z = std::max(0.01f, p1.z);
    p1 = p1 * persp;
    p1 /= p1.w;

    if (a1 < 1) {
      a1 = 1 - (1 - a1) * (1 - powf((p1.y - screen_space_char.y) /
                                        (screen_space_eye.y - screen_space_char.y),
                                    10.0f));
    }

    p1 = p1 * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);

    Point pa = Point(static_cast<int>(p1.x * draw_rect.Width()),
                       static_cast<int>((1 - p1.y) * draw_rect.Height()));

    a1 = std::max(0.f, std::min(1.f, a1)) * 255.f;

    Rect cent = Rect(pa, pa);
    Point p = f->GetCenter(txt[x], cent);
    f->Write(api, txt[x], p,
             CColor(228, 210, 157, static_cast<uint8_t>(a1 * GetMapFade())));
  }
}

void GW2TacticalCompass::OnDraw(gui::CWBDrawAPI* api) {
  if (!mumbleLink.IsValid()) return;

  if (GetConfigValue("TacticalCompassVisible")) DrawTacticalCompass(api);
}

GW2TacticalCompass::GW2TacticalCompass() : CWBGuiType() {}
GW2TacticalCompass::~GW2TacticalCompass() = default;

gui::CWBItem* GW2TacticalCompass::Factory(gui::CWBItem* root,
                                          const CXMLNode& node, Rect& pos) {
  return GW2TacticalCompass::Create(root, pos);
}

bool GW2TacticalCompass::IsMouseTransparent(const Point& client_space_point,
                                            gui::WBMESSAGE message_type) {
  return true;
}
