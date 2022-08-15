#pragma once
#include <objbase.h>

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/base/hasher.h"
#include "src/base/matrix.h"
#include "src/white_board/context_menu.h"
#include "src/white_board/gui_item.h"

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
  } bits{};

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

  std::string_view iconFile;
  std::string_view trailData;
  std::string_view texture;
  std::string_view toggleCategory;
  int16_t achievementId = -1;
  int16_t achievementBit = -1;
  std::string_view info;

  void Read(const CXMLNode& n, bool StoreSaveState);
  void Write(CXMLNode* n);
};

class GW2TacticalCategory;

struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

struct POI {
  MarkerTypeData typeData;
  WBATLASHANDLE icon = 0;

  math::CVector4 cameraSpacePosition;

  math::CVector3 position;
  int32_t mapID = 0;
  size_t wvwObjectiveID = 0;
  std::string_view Type;

  time_t lastUpdateTime = 0;
  bool External = false;
  bool routeMember = false;

  std::string_view zipFile;
  std::string_view iconFile;

  GUID guid{};

  GW2TacticalCategory* category = nullptr;
  void SetCategory(CWBApplication* App, GW2TacticalCategory* t);

  bool IsVisible(const tm& ptm, const time_t& currtime,
                 bool achievementsFetched,
                 std::unordered_map<int32_t, Achievement>& achievements,
                 std::mutex& mtx);
};

struct POIActivationDataKey {
  GUID guid{};
  int uniqueData = 0;

  POIActivationDataKey() = default;

  POIActivationDataKey(GUID g, int inst) : guid(g), uniqueData(inst) {}

  bool operator==(const POIActivationDataKey& d) const {
    return guid == d.guid && uniqueData == d.uniqueData;
  }
};

namespace std {
template <>
struct hash<POIActivationDataKey> {
  std::size_t operator()(const POIActivationDataKey& guid) const;
};
}  // namespace std

struct POIActivationData {
  GUID poiguid{};
  int uniqueData = 0;
  time_t lastUpdateTime = 0;
};

struct POIRoute {
  std::string name;
  bool backwards = true;
  std::vector<GUID> route;
  bool external = false;
  bool hasResetPos = false;
  math::CVector3 resetPos;
  float resetRad = 0;
  int MapID = 0;

  int32_t activeItem = -1;
};

uint32_t DictionaryHash(const GUID& i);
uint32_t DictionaryHash(const POIActivationDataKey& i);

typedef std::unordered_map<GUID, POI> POISet;
extern std::unordered_map<int, POISet> POIs;

extern std::unordered_map<POIActivationDataKey, POIActivationData>
    ActivationData;
extern std::vector<POIRoute> Routes;
extern std::unordered_map<std::string, POI> wvwPOIs;
extern GW2TacticalCategory CategoryRoot;
POISet& GetMapPOIs();

class GW2TacticalDisplay : public CWBItem {
  bool TacticalIconsOnEdge = false;
  float asp = 0;
  math::CMatrix4x4 cam;
  math::CMatrix4x4 persp;
  math::CRect drawrect;

  void FetchAchievements();
  void InsertPOI(POI& poi);
  void DrawPOI(CWBDrawAPI* API, const tm& ptm, const time_t& currtime, POI& poi,
               bool drawDistance, std::string& infoText);
  void DrawPOIMinimap(CWBDrawAPI* API, const math::CRect& miniRect,
                      math::CVector2 pos, const tm& ptm, const time_t& currtime,
                      POI& poi, float alpha, float zoomLevel);
  void OnDraw(CWBDrawAPI* API) override;
  math::CVector3 ProjectTacticalPos(math::CVector3 pos, float fov, float asp);
  std::vector<POI*> mapPOIs;
  std::vector<POI*> minimapPOIs;
  bool drawWvWNames = false;

  bool beingFetched = false;
  bool achievementsFetched = false;
  int32_t lastFetchTime = 0;
  std::thread fetchThread;
  std::mutex achievements_mtx;

  std::unordered_map<int32_t, Achievement> achievements;

 public:
  GW2TacticalDisplay(CWBItem* Parent, math::CRect Position);
  static inline GW2TacticalDisplay* Create(CWBItem* Parent,
                                           math::CRect Position) {
    auto p = std::make_unique<GW2TacticalDisplay>(Parent, Position);
    GW2TacticalDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2TacticalDisplay() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("gw2tactical", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  void RemoveUserMarkersFromMap();
};

class GW2TacticalCategory {
  std::string cachedTypeName;

 public:
  std::string name;
  std::string displayName;

  std::string_view zipFile;

  MarkerTypeData data;
  bool KeepSaveState = false;
  bool IsOnlySeparator = false;
  GW2TacticalCategory* Parent = nullptr;
  std::vector<std::unique_ptr<GW2TacticalCategory>> children;

  std::string GetFullTypeName();

  bool IsDisplayed = true;
  bool cachedVisibility = true;
  [[nodiscard]] bool IsVisible() const;
  void CacheVisibility();

  static bool visibilityCached;
  void CalculateVisibilityCache();
  virtual ~GW2TacticalCategory() = default;
};

void AddPOI(CWBApplication* App);
void DeletePOI();
void UpdatePOI();
void ImportPOIS(CWBApplication* App);
void ExportPOIS();
void ImportPOIActivationData();

void OpenTypeContextMenu(CWBContextMenu* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers = false, int32_t BaseID = 0,
                         bool closeOnClick = false);
void OpenTypeContextMenu(CWBContextItem* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers = false, int32_t BaseID = 0,
                         bool closeOnClick = false);

float WorldToGameCoords(float world);
float GameToWorldCoords(float game);
void FindClosestRouteMarkers(bool force);

int32_t GetTime();
