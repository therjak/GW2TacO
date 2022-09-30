#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "src/base/matrix.h"
#include "src/base/rectangle.h"
#include "src/base/ring_buffer.h"

struct MumbleContext {
  unsigned char serverAddress[28];  // contains sockaddr_in or sockaddr_in6
  uint32_t mapId;
  uint32_t mapType;
  uint32_t shardId;
  uint32_t instance;
  uint32_t buildId;
  // Additional data beyond the 48 bytes Mumble uses for identification
  uint32_t uiState;  // Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight,
                     // Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game has
                     // focus, Bit 5 = Is in Competitive game mode, Bit 6 =
                     // Textbox has focus, Bit 7 = Is in Combat
  uint16_t compassWidth;   // pixels
  uint16_t compassHeight;  // pixels
  float compassRotation;   // radians
  float playerX;           // continentCoords
  float playerY;           // continentCoords
  float mapCenterX;        // continentCoords
  float mapCenterY;        // continentCoords
  float mapScale;
  uint32_t processId;
  uint8_t mountIndex;
};

struct CompassData {
  int compassWidth = 0;       // pixels
  int compassHeight = 0;      // pixels
  float compassRotation = 0;  // guessing... radians? :-P
  float playerX = 0;          // continentCoords
  float playerY = 0;          // continentCoords
  float mapCenterX = 0;       // continentCoords
  float mapCenterY = 0;       // continentCoords
  float mapScale = 0;         // not even sure TBH :-P};
  math::CMatrix4x4 BuildTransformationMatrix(const math::CRect& miniRect,
                                             bool ignoreRotation);
};

struct LinkedMem {
  uint32_t uiVersion = 0;
  uint32_t uiTick = 0;
  float fAvatarPosition[3] = {0};
  float fAvatarFront[3] = {0};
  float fAvatarTop[3] = {0};
  wchar_t name[256] = {0};
  float fCameraPosition[3] = {0};
  float fCameraFront[3] = {0};
  float fCameraTop[3] = {0};
  wchar_t identity[256] = {0};
  uint32_t context_len = 0;
  unsigned char context[256] = {0};
  wchar_t description[2048] = {0};
};

#define AVGCAMCOUNTER 6

class CMumbleLink {
 public:
  CMumbleLink();
  virtual ~CMumbleLink();

  bool Update();
  bool IsValid();
  float GetFrameRate();

  math::CVector3 charPosition;
  math::CVector3 charEye;
  math::CVector3 camPosition;
  math::CVector3 camDir;
  math::CVector3 camUp;
  float fov = 0;
  int32_t mapID = 0;
  int32_t worldID = 0;
  int32_t mapType = 0;
  int32_t mapInstance = 0;
  int32_t charIDHash = 0;

  int32_t lastMapChangeTime = 0;
  bool isMapOpen = false;  // bit 1: IsMapOpen, bit2: IsCompassTopRight, bit3:
                           // DoesCompassHaveRotationEnabled
  bool isMinimapTopRight = false;
  bool isMinimapRotating = false;
  bool gameHasFocus = false;
  bool isPvp = false;
  bool textboxHasFocus = false;
  bool isInCombat = false;

  CompassData miniMap;
  CompassData bigMap;

  int32_t uiSize = 1;

  std::string charName;

  math::CVector4 averagedCharPosition;

  std::unique_ptr<CRingBuffer<int32_t, 60>> FrameTimes;

  int32_t LastFrameTime = 0;

  uint64_t lastTickTime = 0;
  uint64_t lastTickLength = 0;

  bool charPosChanged = false;
  bool charEyeChanged = false;
  bool camPosChanged = false;
  bool camDirChanged = false;
  bool camUpChanged = false;

  std::string mumblePath = "MumbleLink";
  uint32_t lastGW2ProcessID = 0;

 private:
  LinkedMem lastData;
  LinkedMem prevData;

  int tick = 0;
  double interpolation = 0;

  LinkedMem* lm = nullptr;
  math::CVector4 camchardist[AVGCAMCOUNTER];
};

extern CMumbleLink mumbleLink;
math::CRect GetMinimapRectangle();
