#pragma once
#include <objbase.h>

#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/base/hasher.h"
#include "src/base/lock_free_queue.h"

import math;
import taco.marker_data;
import taco.poi_behavior;
import taco.tactical_category;
import whiteboard;
import xml;

struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

struct POI {
  void SetCategory(GW2TacticalCategory* t);

  bool IsVisible(
      const tm& ptm, const time_t& currtime,
      const std::unordered_map<int32_t, Achievement>& achievements) const;

  MarkerTypeData type_data_;
  gui::WBATLASHANDLE icon = 0;

  math::Vector4 cameraSpacePosition;

  math::Vector3 position;
  int32_t map_id = 0;
  size_t Wvwobjective_id = 0;
  std::string_view Type;

  time_t lastUpdateTime = 0;
  bool External = false;
  bool routeMember = false;

  std::string_view zip_file_;
  std::string_view icon_file_;

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
  math::Vector3 resetPos;
  float resetRad = 0;
  int map_id = 0;

  int32_t activeItem = -1;
};

uint32_t DictionaryHash(const GUID& i);
uint32_t DictionaryHash(const POIActivationDataKey& i);

typedef std::unordered_map<GUID, POI> POISet;
extern std::unordered_map<int, POISet> POIs;

extern std::unordered_map<POIActivationDataKey, POIActivationData>
    ActivationData;
extern std::vector<POIRoute> Routes;
extern gui::WBATLASHANDLE DefaultIconHandle;
extern std::string default_marker_category;

POISet& GetMapPOIs();

class GW2TacticalDisplay : public gui::CWBGuiType<"gw2tactical", gui::CWBItem> {
 public:
  GW2TacticalDisplay();
  static inline GW2TacticalDisplay* Create(gui::CWBItem* Parent,
                                           math::Rect Position) {
    auto p = std::make_unique<GW2TacticalDisplay>();
    p->Initialize(Parent, Position);
    GW2TacticalDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2TacticalDisplay() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;
  void RemoveUserMarkersFromMap();

 private:
  void FetchAchievements();
  void InsertPOI(POI& poi);
  void DrawPOI(gui::CWBDrawAPI* API, const tm& ptm, const time_t& currtime,
               POI& poi, bool drawDistance, std::string& infoText);
  void DrawPOIMinimap(gui::CWBDrawAPI* API, const math::Rect& miniRect,
                      math::Vector2 pos, const tm& ptm, const time_t& currtime,
                      const POI& poi, float alpha, float zoomLevel);
  void OnDraw(gui::CWBDrawAPI* API) override;
  math::Vector3 ProjectTacticalPos(math::Vector3 pos, float fov, float asp);

  bool TacticalIconsOnEdge = false;
  float asp = 0;
  math::Matrix4x4 cam;
  math::Matrix4x4 persp;
  math::Rect drawrect;

  std::vector<POI*> mapPOIs;
  std::vector<POI*> minimapPOIs;
  bool drawWvWNames = false;

  int32_t lastFetchTime = 0;

  LockFreeQueue<std::unordered_map<int32_t, Achievement>> achievements_queue;
  std::unordered_map<int32_t, Achievement> achievements;
  std::mutex achievements_mtx;
  std::future<void> fetchTask;
};

void AddPOI();
void DeletePOI();
void UpdatePOI();
void ImportPOIS();
void ExportPOIS();
void ImportPOIActivationData();

void OpenTypeContextMenu(gui::CWBContextMenu* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers = false, int32_t BaseID = 0,
                         bool closeOnClick = false);
void OpenTypeContextMenu(gui::CWBContextItem* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers = false, int32_t BaseID = 0,
                         bool closeOnClick = false);

float WorldToGameCoords(float world);
float GameToWorldCoords(float game);
void FindClosestRouteMarkers(bool force);
float GetMapFade();
