#pragma once
#include "src/base/base_lib.h"

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
  CMatrix4x4 BuildTransformationMatrix(const CRect& miniRect,
                                       bool ignoreRotation);
};

struct LinkedMem {
#ifdef WIN32
  UINT32 uiVersion = 0;
  DWORD uiTick = 0;
#else
  uint32_t uiVersion = 0;
  uint32_t uiTick = 0;
#endif
  float fAvatarPosition[3] = {0};
  float fAvatarFront[3] = {0};
  float fAvatarTop[3] = {0};
  wchar_t name[256] = {0};
  float fCameraPosition[3] = {0};
  float fCameraFront[3] = {0};
  float fCameraTop[3] = {0};
  wchar_t identity[256] = {0};
#ifdef WIN32
  UINT32 context_len = 0;
#else
  uint32_t context_len = 0;
#endif
  unsigned char context[256] = {0};
  wchar_t description[2048] = {0};
};

#define AVGCAMCOUNTER 6

class CMumbleLink {
  LinkedMem* lm = nullptr;
  CVector4 camchardist[AVGCAMCOUNTER];

 public:
  LinkedMem lastData;
  LinkedMem prevData;

  int tick = 0;
  double interpolation = 0;

  CVector3 charPosition;
  CVector3 charEye;
  CVector3 camPosition;
  CVector3 camDir;
  CVector3 camUp;
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

  CVector4 averagedCharPosition;

  void Update();
  bool IsValid();

  std::unique_ptr<CRingBuffer<int32_t>> FrameTimes;
  float GetFrameRate();

  int32_t LastFrameTime = 0;
  CMumbleLink();
  virtual ~CMumbleLink();

  uint64_t lastTickTime = 0;
  uint64_t lastTickLength = 0;

  bool charPosChanged = false;
  bool charEyeChanged = false;
  bool camPosChanged = false;
  bool camDirChanged = false;
  bool camUpChanged = false;

  std::string mumblePath = "MumbleLink";
  uint32_t lastGW2ProcessID = 0;
};

extern CMumbleLink mumbleLink;
CRect GetMinimapRectangle();
