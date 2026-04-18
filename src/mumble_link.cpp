module;
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <cwchar>
#include <memory>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/gw2_tactical.h"

module taco.mumble_link;

import math;
import taco.overlay_config;
import taco.trail_logger;
import time;
import whiteboard;

using math::Matrix4x4;
using math::Rect;
using math::Vector2;
using math::Vector3;
using math::Vector4;

CMumbleLink mumbleLink("MumbleLink");
bool frameTriggered = false;
extern std::unique_ptr<gui::CWBApplication> App;

void ChangeUIScale(int size);

float GetUIScale() {
  float scale = 1.0;
  if (mumbleLink.uiSize == 0) scale = 0.9f;
  if (mumbleLink.uiSize == 2) scale = 1.111f;
  if (mumbleLink.uiSize == 3) scale = 1.224f;

  return scale;
}

float GetWindowTooSmallScale();

Rect GetMinimapRectangle() {
  int w = mumbleLink.miniMap.compassWidth;
  int h = mumbleLink.miniMap.compassHeight;

  Rect pos;
  Rect size = App->GetRoot()->GetClientRect();
  float scale = GetWindowTooSmallScale();

  pos.x1 = static_cast<int>(size.Width() - w * scale);
  pos.x2 = size.Width();

  if (mumbleLink.isMinimapTopRight) {
    pos.y1 = 1;
    pos.y2 = static_cast<int>(h * scale + 1);
  } else {
    int delta = 37;
    if (mumbleLink.uiSize == 0) delta = 33;
    if (mumbleLink.uiSize == 2) delta = 41;
    if (mumbleLink.uiSize == 3) delta = 45;

    pos.y1 = static_cast<int>(size.Height() - h * scale - delta * scale);
    pos.y2 = static_cast<int>(size.Height() - delta * scale);
  }

  return pos;
}

bool CMumbleLink::Update() {
  bool justConnected = false;

  if (!lm) {
    HANDLE hMapObject =
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                           sizeof(LinkedMem), mumblePath.c_str());

    if (hMapObject == nullptr) {
      return false;
    }

    lm = static_cast<LinkedMem*>(MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS,
                                               0, 0, sizeof(LinkedMem)));
    if (lm == nullptr) {
      CloseHandle(hMapObject);
      hMapObject = nullptr;
      return false;
    }
    justConnected = true;
  }

  if (!lm) {
    return false;
  }

  if (tick == lm->uiTick) {
    return false;
  } else {
    memcpy(&prevData, &lastData, sizeof(LinkedMem));
    memcpy(&lastData, lm, sizeof(LinkedMem));

    tick = lm->uiTick;

    globalTimer.Update();
    int32_t frametime = GetTime();
    FrameTimes->Add(frametime - LastFrameTime);
    LastFrameTime = frametime;
    frameTriggered = true;
  }

  float inter = 1.0;  // +( measurement - lastTickTime ) / lastTickLength;

  interpolation = inter;

  charPosition = math::Lerp(Vector3(prevData.fAvatarPosition),
                            Vector3(lastData.fAvatarPosition), inter);
  charEye =
      math::Lerp(Vector3(prevData.fAvatarTop), Vector3(lastData.fAvatarTop), 1);
  camPosition = math::Lerp(Vector3(prevData.fCameraPosition),
                           Vector3(lastData.fCameraPosition), 1);
  camUp =
      math::Lerp(Vector3(prevData.fCameraTop), Vector3(lastData.fCameraTop), 1);
  camDir = math::Lerp(Vector3(prevData.fCameraFront),
                      Vector3(lastData.fCameraFront), inter);

  charPosChanged =
      Vector3(prevData.fAvatarPosition) != Vector3(lastData.fAvatarPosition);
  charEyeChanged = Vector3(prevData.fAvatarTop) != Vector3(lastData.fAvatarTop);
  camPosChanged =
      Vector3(prevData.fCameraPosition) != Vector3(lastData.fCameraPosition);
  camDirChanged =
      Vector3(prevData.fCameraFront) != Vector3(lastData.fCameraFront);
  camUpChanged = Vector3(prevData.fCameraTop) != Vector3(lastData.fCameraTop);

  if ((Vector3(lastData.fAvatarPosition) - Vector3(prevData.fAvatarPosition))
          .Length() > GameToWorldCoords(2000)) {
    FindClosestRouteMarkers(true);
  }

  int32_t oldmap_id = map_id;
  map_id = -1;

  std::wstring ident(lm->identity, 255);
  auto id = ident.find(L"\"map_id\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"map_id\":%d", &map_id);
    if (oldmap_id != map_id) FindClosestRouteMarkers(true);
  } else {
    id = ident.find(L"\"map_id\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"map_id\": %d", &map_id);
      if (oldmap_id != map_id) FindClosestRouteMarkers(true);
    }
  }

  GlobalDoTrailLogging(map_id, Vector3(lastData.fAvatarPosition));

  int32_t oldUISize = uiSize;

  id = ident.find(L"\"uisz\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"uisz\":%d", &uiSize);
    if (oldUISize != uiSize) {
      ChangeUIScale(uiSize);
    }
  } else {
    id = ident.find(L"\"uisz\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"uisz\": %d", &uiSize);
      if (oldUISize != uiSize) {
        ChangeUIScale(uiSize);
      }
    }
  }

  if (justConnected) {
    ChangeUIScale(uiSize);
  }

  id = ident.find(L"\"world_id\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"world_id\":%d", &worldID);
  } else {
    id = ident.find(L"\"world_id\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"world_id\": %d", &worldID);
    }
  }

  auto* ctx = reinterpret_cast<MumbleContext*>(lastData.context);

  map_type = ctx->map_type;
  mapInstance = ctx->shardId;

  if (isMapOpen != (ctx->uiState & 0x01)) {
    lastMapChangeTime = globalTimer.GetTime();
  }

  isMapOpen = (ctx->uiState & 0x01);
  isMinimapTopRight = (ctx->uiState & (0x01 << 1)) != 0;
  isMinimapRotating = (ctx->uiState & (0x01 << 2)) != 0;

  gameHasFocus = (ctx->uiState & (0x01 << 3)) != 0;
  isPvp = (ctx->uiState & (0x01 << 4)) != 0;
  textboxHasFocus = (ctx->uiState & (0x01 << 5)) != 0;
  isInCombat = (ctx->uiState & (0x01 << 6)) != 0;

  float scale = GetUIScale();

  if (!isMapOpen) {
    miniMap.compassWidth = static_cast<int>(ctx->compassWidth * scale);
    miniMap.compassHeight = static_cast<int>(ctx->compassHeight * scale);
    miniMap.compassRotation = ctx->compassRotation;
    miniMap.playerX = ctx->playerX;
    miniMap.playerY = ctx->playerY;
    miniMap.mapCenterX = ctx->mapCenterX;
    miniMap.mapCenterY = ctx->mapCenterY;
    miniMap.mapScale = ctx->mapScale;
  } else {
    bigMap.compassWidth = static_cast<int>(ctx->compassWidth * scale);
    bigMap.compassHeight = static_cast<int>(ctx->compassHeight * scale);
    bigMap.compassRotation = ctx->compassRotation;
    bigMap.playerX = ctx->playerX;
    bigMap.playerY = ctx->playerY;
    bigMap.mapCenterX = ctx->mapCenterX;
    bigMap.mapCenterY = ctx->mapCenterY;
    bigMap.mapScale = ctx->mapScale;
  }

  lastGW2ProcessID = ctx->processId;

  id = ident.find(L"\"name\":");
  if (id != ident.npos) {
    int end = ident.substr(id + 8).find(L'\"');
    if (end != ident.npos) {
      charName = wstring2string(ident.substr(id + 8, end));
      charIDHash = CalculateHash(charName);
    } else {
      charName = "";
      charIDHash = 0;
    }
  } else {
    id = ident.find(L"\"name\": ");
    if (id >= 0) {
      int end = ident.substr(id + 9).find(L'\"');
      if (end != ident.npos) {
        charName = wstring2string(ident.substr(id + 9, end));
        charIDHash = CalculateHash(charName);
      } else {
        charName = "";
        charIDHash = 0;
      }
    } else {
      charName = "";
      charIDHash = 0;
    }
  }

  fov = 0;

  id = ident.find(L"\"fov\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"fov\":%f", &fov);
  } else {
    id = ident.find(L"\"fov\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"fov\": %f", &fov);
    }
  }

  Matrix4x4 cam;
  cam.SetLookAtLH(camPosition, camPosition + camDir, Vector3(0, 1, 0));

  Matrix4x4 cami = cam.Inverted();

  for (int x = 0; x < AVGCAMCOUNTER - 1; x++) {
    camchardist[x] = camchardist[x + 1];
  }
  camchardist[AVGCAMCOUNTER - 1] = charPosition * cam;

  Vector4 avgCamCharDist(0, 0, 0, 0);

  for (const auto& x : camchardist) avgCamCharDist += x;

  avgCamCharDist /= static_cast<float>(AVGCAMCOUNTER);
  averagedCharPosition = avgCamCharDist * cami;
  averagedCharPosition /= averagedCharPosition.w;

  if (!GetConfigValue("SmoothCharacterPos")) {
    averagedCharPosition =
        Vector4(charPosition.x, charPosition.y, charPosition.z, 1.0f);
  }

  return true;
}

float CMumbleLink::GetFrameRate() {
  int32_t FrameTimeAcc = 0;
  int32_t FrameCount = 0;
  for (int32_t x = 0; x < 60; x++) {
    if (FrameTimes->NumItems() < x) break;
    FrameTimeAcc += (*FrameTimes)[FrameTimes->NumItems() - 1 - x];
    FrameCount++;
  }

  if (!FrameCount) return 0;
  if (!FrameTimeAcc) return 9999;
  return 1000.0f / (FrameTimeAcc / static_cast<float>(FrameCount));
}

CMumbleLink::CMumbleLink(std::string_view mumblePath)
    : FrameTimes(std::make_unique<CRingBuffer<int32_t, 60>>()),
      LastFrameTime(GetTime()),
      mumblePath(mumblePath) {}

CMumbleLink::~CMumbleLink() = default;

bool CMumbleLink::IsValid() { return lm != nullptr && lastGW2ProcessID != 0; }

Matrix4x4 CompassData::BuildTransformationMatrix(const Rect& miniRect,
                                                 bool ignoreRotation) {
  Matrix4x4 miniMapTrafo(1 / 0.0254f, 0, 0, 0, 0, 0, 0, 0, 0, 1 / 0.0254f, 0, 0,
                         0, 0, 0, 1);

  Vector2 mapOffset = Vector2(WorldToGameCoords(mumbleLink.charPosition.x),
                              WorldToGameCoords(mumbleLink.charPosition.z));

  float rotation = ignoreRotation ? 0 : compassRotation;

  miniMapTrafo *=
      Matrix4x4::Translation(Vector3(-mapOffset.x, -mapOffset.y, 0.0));
  miniMapTrafo *= Matrix4x4::Scaling(Vector3(1, -1, 1));
  miniMapTrafo *= Matrix4x4::Rotation(Vector3(0, 0, 1), rotation);
  miniMapTrafo *= Matrix4x4::Scaling(Vector3(1, 1, 1) / 24.0f);

  Vector2 offset =
      -((Vector2(mapCenterX, mapCenterY) - Vector2(playerX, playerY)) *
        GetWindowTooSmallScale())
           .Rotated(Vector2(0, 0), rotation);
  miniMapTrafo *= Matrix4x4::Translation(Vector3(offset.x, offset.y, 0.0));
  miniMapTrafo *=
      Matrix4x4::Scaling(Vector3(1, 1, 1) / mapScale * GetUIScale());
  miniMapTrafo *= Matrix4x4::Translation(
      Vector3(static_cast<float>(miniRect.Center().x),
              static_cast<float>(miniRect.Center().y), 0.0));

  return miniMapTrafo;
}
