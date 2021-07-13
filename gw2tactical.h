#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"
#include <objbase.h>

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


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

struct MarkerTypeData {
  struct {
    bool needsExportToUserData : 1;
    bool iconFileSaved : 1;
    bool sizeSaved : 1;
    bool alphaSaved : 1;
    bool fadeNearSaved : 1;
    bool fadeFarSaved : 1;
    bool heightSaved : 1;
    bool behaviorSaved : 1;
    bool resetLengthSaved : 1;
    bool autoTriggerSaved : 1;
    bool hasCountdownSaved : 1;
    bool triggerRangeSaved : 1;
    bool minSizeSaved : 1;
    bool maxSizeSaved : 1;
    bool colorSaved : 1;
    bool trailDataSaved : 1;
    bool animSpeedSaved : 1;
    bool textureSaved : 1;
    bool trailScaleSaved : 1;
    bool toggleCategorySaved : 1;
    bool achievementIdSaved : 1;
    bool achievementBitSaved : 1;
    bool autoTrigger : 1;
    bool hasCountdown : 1;
    bool miniMapVisible : 1;
    bool bigMapVisible : 1;
    bool inGameVisible : 1;
    bool miniMapVisibleSaved : 1;
    bool bigMapVisibleSaved : 1;
    bool inGameVisibleSaved : 1;
    bool scaleWithZoom : 1;
    bool scaleWithZoomSaved : 1;
    bool miniMapSizeSaved : 1;
    bool miniMapFadeOutLevelSaved : 1;
    bool keepOnMapEdge : 1;
    bool keepOnMapEdgeSaved : 1;
    bool infoSaved : 1;
    bool infoRangeSaved : 1;
  } bits;

  MarkerTypeData();

  float size = 1.0;
  float alpha = 1.0f;
  float fadeNear = -1;
  float fadeFar = -1;
  float height = 1.5f;
  float triggerRange = 2.0f;
  float animSpeed = 1;
  float trailScale = 1;
  int32_t miniMapSize = 20;
  float miniMapFadeOutLevel = 100.0f;
  float infoRange = 2.0f;

  POIBehavior behavior = POIBehavior::AlwaysVisible;
  CColor color = CColor(0xffffffff);

  int16_t resetLength = 0;
  int16_t minSize = 5;
  int16_t maxSize = 2048;

  int16_t iconFile = -1;
  int16_t trailData = -1;
  int16_t texture = -1;
  int16_t toggleCategory = -1;
  int16_t achievementId = -1;
  int16_t achievementBit = -1;
  int16_t info = -1;

  void Read(CXMLNode &n, TBOOL StoreSaveState);
  void Write(CXMLNode *n);
};

class GW2TacticalCategory;

struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

struct POI {
  MarkerTypeData typeData;
  WBATLASHANDLE icon = 0;

  CVector4 cameraSpacePosition;

  CVector3 position;
  int32_t mapID;
  size_t wvwObjectiveID;
  std::string Type;

  time_t lastUpdateTime = 0;
  TBOOL External = false;
  TBOOL routeMember = false;

  int16_t zipFile = 0;
  int16_t iconFile;

  GUID guid;

  GW2TacticalCategory *category = nullptr;
  void SetCategory(CWBApplication *App, GW2TacticalCategory *t);

  bool IsVisible(const tm &ptm, const time_t &currtime,
                 bool achievementsFetched,
                 std::unordered_map<int32_t, Achievement> &achievements,
                 LIGHTWEIGHT_CRITICALSECTION &dataWriteCritSec);
};

struct POIActivationDataKey {
  GUID guid;
  int uniqueData = 0;

  POIActivationDataKey() = default;

  POIActivationDataKey(GUID g, int inst) : guid(g), uniqueData(inst) {}

  bool operator==(const POIActivationDataKey &d) {
    return guid == d.guid && uniqueData == d.uniqueData;
  }
};

struct POIActivationData {
  GUID poiguid;
  int uniqueData = 0;
  time_t lastUpdateTime = 0;
};

struct POIRoute {
  std::string name;
  TBOOL backwards = true;
  std::vector<GUID> route;
  TBOOL external = false;
  TBOOL hasResetPos = false;
  CVector3 resetPos;
  float resetRad = 0;
  int MapID = 0;

  int32_t activeItem = -1;
};

uint32_t DictionaryHash(const GUID &i);
uint32_t DictionaryHash(const POIActivationDataKey &i);

extern CDictionaryEnumerable<GUID, POI> POIs;
extern CDictionaryEnumerable<POIActivationDataKey, POIActivationData>
    ActivationData;
extern std::vector<POIRoute> Routes;
extern std::unordered_map<std::string, POI> wvwPOIs;

class GW2TacticalDisplay : public CWBItem {
  TBOOL TacticalIconsOnEdge;
  float asp = 0;
  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CRect drawrect;

  void FetchAchievements();
  void InsertPOI(POI &poi);
  void DrawPOI(CWBDrawAPI *API, const tm &ptm, const time_t &currtime, POI &poi,
               bool drawDistance, std::string &infoText);
  void DrawPOIMinimap(CWBDrawAPI *API, const CRect &miniRect, CVector2 &pos,
                      const tm &ptm, const time_t &currtime, POI &poi,
                      float alpha, float zoomLevel);
  virtual void OnDraw(CWBDrawAPI *API);
  CVector3 ProjectTacticalPos(CVector3 pos, float fov, float asp);
  std::vector<POI *> mapPOIs;
  std::vector<POI *> minimapPOIs;
  bool drawWvWNames = false;

  bool beingFetched = false;
  bool achievementsFetched = false;
  int32_t lastFetchTime = 0;
  std::thread fetchThread;
  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

  std::unordered_map<int32_t, Achievement> achievements;

 public:
  GW2TacticalDisplay(CWBItem *Parent, CRect Position);
  virtual ~GW2TacticalDisplay();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "gw2tactical" ), CWBItem);

  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);
  void RemoveUserMarkersFromMap();
};

class GW2TacticalCategory {
  std::string cachedTypeName;

 public:
  std::string name;
  std::string displayName;

  int16_t zipFile = 0;

  MarkerTypeData data;
  TBOOL KeepSaveState = false;
  TBOOL IsOnlySeparator = false;
  GW2TacticalCategory *Parent = nullptr;
  std::vector<std::unique_ptr<GW2TacticalCategory>> children;

  std::string GetFullTypeName();

  bool IsDisplayed = true;
  bool IsVisible();

  virtual ~GW2TacticalCategory() {}
};

void AddPOI(CWBApplication *App);
void DeletePOI();
void UpdatePOI();
void ImportPOIS(CWBApplication *App);
void ExportPOIS();
void ImportPOIActivationData();

void OpenTypeContextMenu(CWBContextMenu *ctx,
                         std::vector<GW2TacticalCategory *> &CategoryList,
                         TBOOL AddVisibilityMarkers = false, int32_t BaseID = 0,
                         TBOOL closeOnClick = false);
void OpenTypeContextMenu(CWBContextItem *ctx,
                         std::vector<GW2TacticalCategory *> &CategoryList,
                         TBOOL AddVisibilityMarkers = false, int32_t BaseID = 0,
                         TBOOL closeOnClick = false);

float WorldToGameCoords(float world);
float GameToWorldCoords(float game);
void FindClosestRouteMarkers(TBOOL force);

int32_t GetTime();
std::string &GetStringFromMap(int32_t idx);

