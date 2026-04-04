#pragma once
#include <objbase.h>

#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/base/hasher.h"
#include "src/base/lock_free_queue.h"
#include "src/base/matrix.h"

import xml;
import whiteboard;
import taco.marker_data;
import taco.tactical_category;
import taco.poi_behavior;

struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

struct POI {
  void SetCategory(GW2TacticalCategory* t);

  bool IsVisible(
      const tm& ptm, const time_t& currtime,
      const std::unordered_map<int32_t, Achievement>& achievements) const;

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
};

struct POIActivationDataKey {
  POIActivationDataKey() = default;

  POIActivationDataKey(GUID g, int inst) : guid(g), uniqueData(inst) {}

  constexpr friend bool operator==(const POIActivationDataKey& lhs,
                                   const POIActivationDataKey& rhs) = default;

  GUID guid{};
  int uniqueData = 0;
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

POISet& GetMapPOIs();

class GW2TacticalDisplay : public CWBGuiType<"gw2tactical", CWBItem> {
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

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  void RemoveUserMarkersFromMap();

 private:
  void FetchAchievements();
  void InsertPOI(POI& poi);
  void DrawPOI(CWBDrawAPI* API, const tm& ptm, const time_t& currtime, POI& poi,
               bool drawDistance, std::string& infoText);
  void DrawPOIMinimap(CWBDrawAPI* API, const math::CRect& miniRect,
                      math::CVector2 pos, const tm& ptm, const time_t& currtime,
                      const POI& poi, float alpha, float zoomLevel);
  void OnDraw(CWBDrawAPI* API) override;
  math::CVector3 ProjectTacticalPos(math::CVector3 pos, float fov, float asp);

  bool TacticalIconsOnEdge = false;
  float asp = 0;
  math::CMatrix4x4 cam;
  math::CMatrix4x4 persp;
  math::CRect drawrect;

  std::vector<POI*> mapPOIs;
  std::vector<POI*> minimapPOIs;
  bool drawWvWNames = false;

  std::atomic<bool> being_fetched = false;
  std::atomic<bool> achievements_fetched = false;
  int32_t lastFetchTime = 0;

  LockFreeQueue<std::unordered_map<int32_t, Achievement>> achievements_queue;
  std::unordered_map<int32_t, Achievement> achievements;
  std::mutex achievements_mtx;
  // on destruction the thread should be destroyed first
  std::thread fetchThread;
};

void AddPOI();
void DeletePOI();
void UpdatePOI();
void ImportPOIS();
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