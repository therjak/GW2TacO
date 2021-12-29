#include "mumble_link.h"

#include "overlay_config.h"
#include "src/base/logger.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "trail_logger.h"

CMumbleLink mumbleLink;
bool frameTriggered = false;
extern std::unique_ptr<CWBApplication> App;

void ChangeUIScale(int size);

float GetUIScale() {
  float scale = 1.0;
  if (mumbleLink.uiSize == 0) scale = 0.9f;
  if (mumbleLink.uiSize == 2) scale = 1.111f;
  if (mumbleLink.uiSize == 3) scale = 1.224f;

  return scale;
}

float GetWindowTooSmallScale();

CRect GetMinimapRectangle() {
  int w = mumbleLink.miniMap.compassWidth;
  int h = mumbleLink.miniMap.compassHeight;

  CRect pos;
  CRect size = App->GetRoot()->GetClientRect();
  float scale = GetWindowTooSmallScale();

  pos.x1 = int(size.Width() - w * scale);
  pos.x2 = size.Width();

  if (mumbleLink.isMinimapTopRight) {
    pos.y1 = 1;
    pos.y2 = int(h * scale + 1);
  } else {
    int delta = 37;
    if (mumbleLink.uiSize == 0) delta = 33;
    if (mumbleLink.uiSize == 2) delta = 41;
    if (mumbleLink.uiSize == 3) delta = 45;

    pos.y1 = int(size.Height() - h * scale - delta * scale);
    pos.y2 = int(size.Height() - delta * scale);
  }

  return pos;
}

void CMumbleLink::Update() {
  bool justConnected = false;

  FORCEDDEBUGLOG("updating mumblelink");
  if (!lm) {
    HANDLE hMapObject =
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                           sizeof(LinkedMem), mumblePath.c_str());

    if (hMapObject == nullptr) {
      FORCEDDEBUGLOG("failed to create mumble link file");
      return;
    }

    lm = static_cast<LinkedMem*>(MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS,
                                               0, 0, sizeof(LinkedMem)));
    if (lm == nullptr) {
      FORCEDDEBUGLOG("mumble link file closed");
      CloseHandle(hMapObject);
      hMapObject = nullptr;
      return;
    }
    justConnected = true;
  }

  if (!lm) return;

  FORCEDDEBUGLOG("getting mumblelink data");

  if (tick == lm->uiTick) {
    memcpy(&lastData, lm, sizeof(LinkedMem));
    return;
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

  charPosition = Lerp(CVector3(prevData.fAvatarPosition),
                      CVector3(lastData.fAvatarPosition), inter);
  charEye =
      Lerp(CVector3(prevData.fAvatarTop), CVector3(lastData.fAvatarTop), 1);
  camPosition = Lerp(CVector3(prevData.fCameraPosition),
                     CVector3(lastData.fCameraPosition), 1);
  camUp = Lerp(CVector3(prevData.fCameraTop), CVector3(lastData.fCameraTop), 1);
  camDir = Lerp(CVector3(prevData.fCameraFront),
                CVector3(lastData.fCameraFront), inter);

  charPosChanged =
      CVector3(prevData.fAvatarPosition) != CVector3(lastData.fAvatarPosition);
  charEyeChanged =
      CVector3(prevData.fAvatarTop) != CVector3(lastData.fAvatarTop);
  camPosChanged =
      CVector3(prevData.fCameraPosition) != CVector3(lastData.fCameraPosition);
  camDirChanged =
      CVector3(prevData.fCameraFront) != CVector3(lastData.fCameraFront);
  camUpChanged = CVector3(prevData.fCameraTop) != CVector3(lastData.fCameraTop);

  if ((CVector3(lastData.fAvatarPosition) - CVector3(prevData.fAvatarPosition))
          .Length() > GameToWorldCoords(2000))
    FindClosestRouteMarkers(true);

  int32_t oldMapID = mapID;
  mapID = -1;

  std::wstring ident(lm->identity, 255);
  auto id = ident.find(L"\"map_id\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"map_id\":%d", &mapID);
    if (oldMapID != mapID) FindClosestRouteMarkers(true);
  } else {
    id = ident.find(L"\"map_id\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"map_id\": %d", &mapID);
      if (oldMapID != mapID) FindClosestRouteMarkers(true);
    }
  }

  GlobalDoTrailLogging(mapID, CVector3(lastData.fAvatarPosition));

  int32_t oldUISize = uiSize;

  id = ident.find(L"\"uisz\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"uisz\":%d", &uiSize);
    if (oldUISize != uiSize) ChangeUIScale(uiSize);
  } else {
    id = ident.find(L"\"uisz\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"uisz\": %d", &uiSize);
      if (oldUISize != uiSize) ChangeUIScale(uiSize);
    }
  }

  if (justConnected) {
    ChangeUIScale(uiSize);
  }

  id = ident.find(L"\"world_id\":");
  if (id != ident.npos)
    std::swscanf(ident.substr(id).c_str(), L"\"world_id\":%d", &worldID);
  else {
    id = ident.find(L"\"world_id\": ");
    if (id != ident.npos)
      std::swscanf(ident.substr(id).c_str(), L"\"world_id\": %d", &worldID);
  }

  MumbleContext* ctx = reinterpret_cast<MumbleContext*>(lastData.context);

  mapType = ctx->mapType;
  mapInstance = ctx->shardId;

  if (isMapOpen != (ctx->uiState & 0x01))
    lastMapChangeTime = globalTimer.GetTime();

  isMapOpen = (ctx->uiState & 0x01);
  isMinimapTopRight = (ctx->uiState & (0x01 << 1)) != 0;
  isMinimapRotating = (ctx->uiState & (0x01 << 2)) != 0;

  gameHasFocus = (ctx->uiState & (0x01 << 3)) != 0;
  isPvp = (ctx->uiState & (0x01 << 4)) != 0;
  textboxHasFocus = (ctx->uiState & (0x01 << 5)) != 0;
  isInCombat = (ctx->uiState & (0x01 << 6)) != 0;

  float scale = GetUIScale();

  if (!isMapOpen) {
    miniMap.compassWidth = int(ctx->compassWidth * scale);
    miniMap.compassHeight = int(ctx->compassHeight * scale);
    miniMap.compassRotation = ctx->compassRotation;
    miniMap.playerX = ctx->playerX;
    miniMap.playerY = ctx->playerY;
    miniMap.mapCenterX = ctx->mapCenterX;
    miniMap.mapCenterY = ctx->mapCenterY;
    miniMap.mapScale = ctx->mapScale;
  } else {
    bigMap.compassWidth = int(ctx->compassWidth * scale);
    bigMap.compassHeight = int(ctx->compassHeight * scale);
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
  if (id != ident.npos)
    std::swscanf(ident.substr(id).c_str(), L"\"fov\":%f", &fov);
  else {
    id = ident.find(L"\"fov\": ");
    if (id != ident.npos)
      std::swscanf(ident.substr(id).c_str(), L"\"fov\": %f", &fov);
  }

  CMatrix4x4 cam;
  cam.SetLookAtLH(camPosition, camPosition + camDir, CVector3(0, 1, 0));

  CMatrix4x4 cami = cam.Inverted();

  for (int x = 0; x < AVGCAMCOUNTER - 1; x++)
    camchardist[x] = camchardist[x + 1];
  camchardist[AVGCAMCOUNTER - 1] = charPosition * cam;

  CVector4 avgCamCharDist(0, 0, 0, 0);

  for (const auto& x : camchardist) avgCamCharDist += x;

  avgCamCharDist /= static_cast<float>(AVGCAMCOUNTER);
  averagedCharPosition = avgCamCharDist * cami;
  averagedCharPosition /= averagedCharPosition.w;

  if (!GetConfigValue("SmoothCharacterPos"))
    averagedCharPosition =
        CVector4(charPosition.x, charPosition.y, charPosition.z, 1.0f);
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

CMumbleLink::CMumbleLink() {
  LastFrameTime = GetTime();
  FrameTimes = std::make_unique<CRingBuffer<int32_t>>(60);
}

CMumbleLink::~CMumbleLink() = default;

bool CMumbleLink::IsValid() { return lm != 0 && lastGW2ProcessID != 0; }

CMatrix4x4 CompassData::BuildTransformationMatrix(const CRect& miniRect,
                                                  bool ignoreRotation) {
  CMatrix4x4 miniMapTrafo(1 / 0.0254f, 0, 0, 0, 0, 0, 0, 0, 0, 1 / 0.0254f, 0,
                          0, 0, 0, 0, 1);

  CVector2 mapOffset = CVector2(WorldToGameCoords(mumbleLink.charPosition.x),
                                WorldToGameCoords(mumbleLink.charPosition.z));

  float rotation = ignoreRotation ? 0 : compassRotation;

  miniMapTrafo *=
      CMatrix4x4::Translation(CVector3(-mapOffset.x, -mapOffset.y, 0.0));
  miniMapTrafo *= CMatrix4x4::Scaling(CVector3(1, -1, 1));
  miniMapTrafo *= CMatrix4x4::Rotation(CVector3(0, 0, 1), rotation);
  miniMapTrafo *= CMatrix4x4::Scaling(CVector3(1, 1, 1) / 24.0f);

  CVector2 offset =
      -((CVector2(mapCenterX, mapCenterY) - CVector2(playerX, playerY)) *
        GetWindowTooSmallScale())
           .Rotated(CVector2(0, 0), rotation);
  miniMapTrafo *= CMatrix4x4::Translation(CVector3(offset.x, offset.y, 0.0));
  miniMapTrafo *=
      CMatrix4x4::Scaling(CVector3(1, 1, 1) / mapScale * GetUIScale());
  miniMapTrafo *= CMatrix4x4::Translation(
      CVector3(float(miniRect.Center().x), float(miniRect.Center().y), 0.0));

  return miniMapTrafo;
}
