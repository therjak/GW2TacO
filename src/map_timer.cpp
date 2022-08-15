#include "src/map_timer.h"

#include <algorithm>
#include <format>
#include <mutex>
#include <string>
#include <thread>

#include "src/gw2_api.h"
#include "src/overlay_config.h"
#include "src/util/jsonxx.h"

using namespace jsonxx;

using math::CPoint;
using math::CRect;
using math::CSize;

bool GW2MapTimer::IsScrollbarVisible() {
  CRect cr = GetClientRect();
  return IsVScrollbarEnabled() && lastypos > cr.Height();
}

void GW2MapTimer::OnResize(const CSize& s) {
  if (lastypos > 0) {
    UpdateScrollbarData(lastypos, GetClientRect());
  }
}

void GW2MapTimer::OnDraw(CWBDrawAPI* API) {
  if (!GetConfigValue("MapTimerVisible")) {
    return;
  }

  std::string mouseToolTip;

  if (GW2::apiKeyManager.GetStatus() == GW2::APIKeyManager::Status::OK) {
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

    if (key && key->valid &&
        (GetTime() - lastFetchTime > 150000 || !lastFetchTime) &&
        !beingFetched && !fetchThread.joinable()) {
      beingFetched = true;
      fetchThread = std::thread([key, this]() {
        Object json;

        const auto& lastWorldBosses =
            "{\"worldbosses\":" + key->QueryAPI("/v2/account/worldbosses") +
            "}";
        json.parse(lastWorldBosses);

        if (json.has<Array>("worldbosses")) {
          auto worldBossData = json.get<Array>("worldbosses").values();

          std::vector<std::string> localWorldBosses;
          for (auto& x : worldBossData) {
            if (!x->is<String>()) continue;

            auto eventName = x->get<String>();
            localWorldBosses.push_back(eventName);
          }

          std::lock_guard<std::mutex> lockGuard(mtx);
          std::swap(localWorldBosses, worldBosses);
        }

        const auto& lastMapChests =
            "{\"mapchests\":" + key->QueryAPI("/v2/account/mapchests") + "}";
        json.parse(lastMapChests);

        if (json.has<Array>("mapchests")) {
          const auto& mapChestData = json.get<Array>("mapchests").values();

          std::vector<std::string> localMapchests;
          for (auto& x : mapChestData) {
            if (!x->is<String>()) continue;

            auto eventName = x->get<String>();
            localMapchests.push_back(eventName);
          }

          std::lock_guard<std::mutex> lockGuard(mtx);
          std::swap(mapchests, localMapchests);
        }

        beingFetched = false;
      });
    }
  }

  if (!beingFetched && fetchThread.joinable()) {
    lastFetchTime = GetTime();
    fetchThread.join();
  }

  bool compact = GetConfigValue("MapTimerCompact");
  bool showCategories = GetConfigValue("MapTimerCategories");

  int32_t timeWindow = 120;
  int32_t mapheight = 40;
  int32_t barheight = 20;

  int32_t categoryLineWidth = GetConfigValue("MapTimerCategoryLineWidth");
  int32_t paddingLeft = showCategories ? categoryLineWidth + 4 : 0;

  CRect cl = GetClientRect();

  time_t rawtime = 0;
  time(&rawtime);
  struct tm ptm {};
  gmtime_s(&ptm, &rawtime);

  WBITEMSTATE i = GetState();
  CWBFont* f = GetFont(i);

  barheight = f->GetLineHeight();
  mapheight = barheight + f->GetLineHeight();

  if (compact) {
    mapheight = barheight;
  }

  int32_t mapCount = 0;
  for (const auto& m : maps) {
    if (m.display) {
      mapCount++;
    }
  }

  DrawBackgroundItem(API, CSSProperties.DisplayDescriptor,
                     CRect(CPoint(cl.x1 + paddingLeft, cl.y1),
                           CPoint(cl.Width(), mapCount * mapheight + 1)),
                     GetState());

  auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  int32_t minutes = ptm.tm_hour * 60 + ptm.tm_min;
  int32_t lefttime = minutes - timeWindow / 2;

  int32_t scrollbarPos = GetVScrollbarPos();
  int32_t ypos = -scrollbarPos;

  std::vector<CRect> highlightRects;
  int32_t lastCategoryStartY = 0;
  const Category* lastCategory = nullptr;
  for (const auto& map : maps) {
    if (!map.display) {
      continue;
    }

    int32_t currtime = -48 * 60 + map.Length + map.Start - lefttime;
    int32_t currevent = 0;

    bool shouldDraw = true;
    if ((ypos > cl.y2) || (ypos < cl.y1 - mapheight)) {
      shouldDraw = false;
    }

    if (shouldDraw) {
      int32_t toppos = ypos + mapheight - barheight;
      int32_t bottompos = ypos + mapheight + 1;

      // map name
      {
        CPoint p = f->GetCenter(
            map.name,
            CRect(cl.x1, ypos, cl.x2, ypos + mapheight - barheight + 1),
            TextTransform);
        if (!compact) {
          f->Write(API, map.name, CPoint(p.x, ypos + 2), CColor{0xffffffff},
                   TextTransform);
        }
      }

      // map category
      if (showCategories && !map.category.empty() &&
          categories.find(map.category) != categories.end()) {
        const Category& category = categories[map.category];

        if (&category != lastCategory) {
          if (lastCategory && lastCategory->color.A() > 0) {
            API->DrawRect(CRect(cl.x1, lastCategoryStartY,
                                cl.x1 + categoryLineWidth, toppos),
                          lastCategory->color);
            API->DrawRectBorder(CRect(cl.x1, lastCategoryStartY,
                                      cl.x1 + categoryLineWidth, toppos),
                                CColor{0x80000000});
          }
          lastCategoryStartY = toppos;
          lastCategory = &category;
        }

        CRect r = CRect(cl.x1, toppos, cl.x1 + paddingLeft, bottompos - 1)
                      .GetIntersection(cl);
        if (ClientToScreen(r).Contains(GetApplication()->GetMousePos())) {
          mouseToolTip = category.name;
        }
      }

      // highlight rect
      if (!map.chestId.empty()) {
        std::lock_guard<std::mutex> lockGuard(mtx);
        if (std::find(mapchests.begin(), mapchests.end(), map.chestId) !=
            mapchests.end()) {
          highlightRects.emplace_back(
              CRect(cl.x1 + paddingLeft, toppos, cl.x2, bottompos));
        }
      }

      // map events
      while (currtime < 72 * 60) {
        int32_t p1 =
            paddingLeft + static_cast<int32_t>((cl.Width() - paddingLeft) *
                                               currtime / timeWindow);
        int32_t p2 =
            paddingLeft +
            static_cast<int32_t>((cl.Width() - paddingLeft) *
                                 (currtime + map.events[currevent].length) /
                                 timeWindow) +
            1;

        if (p2 >= paddingLeft && p1 <= cl.Width()) {
          CRect r = CRect(std::max(paddingLeft, p1), toppos,
                          std::min(cl.Width(), p2), bottompos);

          API->DrawRect(r, map.events[currevent].color);

          CRect cr = API->GetCropRect();
          API->SetCropRect(ClientToScreen(r));

          auto text = map.events[currevent].name;

          {
            int32_t timeleft = currtime * 60 - ptm.tm_sec - timeWindow * 30 +
                               map.events[currevent].length * 60;
            if (timeleft >= 0 &&
                timeleft <= map.events[currevent].length * 60) {
              text = !text.empty() ? std::format("{:s} {:d}:{:02d}", text,
                                                 timeleft / 60, timeleft % 60)
                                   : std::format("{:d}:{:02d}", timeleft / 60,
                                                 timeleft % 60);
            }
          }

          if (ClientToScreen(r.GetIntersection(cl))
                  .Contains(GetApplication()->GetMousePos())) {
            mouseToolTip = !map.events[currevent].name.empty()
                               ? std::format("{:s} - {:s}", map.name, text)
                               : text;
          }

          CPoint p = f->GetCenter(text, r, TextTransform);
          f->Write(API, text, CPoint(p.x, r.y1 + 2), CColor{0xffffffff},
                   TextTransform);

          API->SetCropRect(cr);

          bool isHighlighted = false;

          const auto& bossId = map.events[currevent].worldBossId;
          if (!map.events[currevent].worldBossId.empty()) {
            std::lock_guard<std::mutex> lockGuard(mtx);
            if (std::find(worldBosses.begin(), worldBosses.end(), bossId) !=
                worldBosses.end()) {
              isHighlighted = true;
            }
            if (std::find(mapchests.begin(), mapchests.end(), bossId) !=
                mapchests.end()) {
              isHighlighted = true;
            }
          }

          if (!isHighlighted) {
            API->DrawRectBorder(r, CColor{0x80000000});
          } else {
            highlightRects.push_back(r);
          }
        }

        currtime += map.events[currevent].length;
        currevent = (currevent + 1) % map.events.size();
      }
    }

    ypos += mapheight;
  }

  if (showCategories) {
    if (lastCategory && lastCategory->color.A() > 0) {
      API->DrawRect(
          CRect(cl.x1, lastCategoryStartY, cl.x1 + categoryLineWidth, ypos),
          lastCategory->color);
      API->DrawRectBorder(
          CRect(cl.x1, lastCategoryStartY, cl.x1 + categoryLineWidth, ypos),
          CColor{0x80000000});
    }
  }

  for (const auto& r : highlightRects) {
    API->DrawRectBorder(r, CColor{0xffffcc00});
  }

  API->DrawRect(CRect(cl.Width() / 2, 0, cl.Width() / 2 + 1, ypos),
                CColor{0x80ffffff});
  SetMouseToolTip(mouseToolTip);

  // update scrollbar
  ypos += scrollbarPos;
  if (lastypos == -1 || ypos != lastypos) UpdateScrollbarData(ypos, cl);
  lastypos = ypos;

  // set mouse transparency
  // if there is overflow, this gui item needs to be able to receive mouse
  // events to handle the scrollbar
  SetForcedMouseTransparency(!IsScrollbarVisible());
}

int32_t GW2MapTimer::GetScrollbarStep() { return 5; }

CWBItem* GW2MapTimer::GetItemUnderMouse(CPoint& Pos, CRect& CropRect,
                                        WBMESSAGE MessageType) {
  CWBItem* item = CWBItem::GetItemUnderMouse(Pos, CropRect, MessageType);
  if (item && IsScrollbarVisible()) {
    // Only the scrollbar needs to be "visible"

    CRect sr = GetScreenRect();
    CRect b1, su, th, sd, b2;
    GetVScrollbarRectangles(b1, su, th, sd, b2);

    b1.Move(sr.x1, sr.y1);
    return (Pos.x >= b1.x1 && Pos.x <= b1.x2) &&
                   (Pos.y >= sr.y1 && Pos.y <= sr.y2)
               ? this
               : nullptr;
  }
  return item;
}

void GW2MapTimer::SetLayout(CXMLNode& node) {
  std::unordered_map<std::string, std::vector<Map>> _categoryMapsDict;
  std::vector<std::string> _categories;

  // categories
  const CXMLNode& categoriesNode = node.GetChild("Categories");
  if (categoriesNode.IsValid()) {
    for (int x = 0; x < categoriesNode.GetChildCount("Category"); ++x) {
      const CXMLNode& categoryNode = categoriesNode.GetChild("Category", x);

      Category category;
      if (categoryNode.HasAttribute("id")) {
        category.id = categoryNode.GetAttributeAsString("id");
      }

      if (categoryNode.HasAttribute("Name")) {
        category.name = categoryNode.GetAttributeAsString("Name");
      }

      if (categoryNode.HasAttribute("Color")) {
        auto s = categoryNode.GetAttributeAsString("Color");
        uint32_t c = 0;
        std::sscanf(s.c_str(), "%x", &c);
        category.color = CColor(c);
      }

      categories[category.id] = category;
    }
  }

  // maps
  const CXMLNode& mapsNode = node.GetChild("Maps");
  if (mapsNode.IsValid()) {
    for (int x = 0; x < mapsNode.GetChildCount("Map"); x++) {
      const CXMLNode& mapNode = mapsNode.GetChild("Map", x);

      Map map;
      if (mapNode.HasAttribute("Name")) {
        map.name = mapNode.GetAttributeAsString("Name");
      }

      if (mapNode.HasAttribute("ChestAPIID")) {
        map.chestId = mapNode.GetAttributeAsString("ChestAPIID");
      }

      if (mapNode.HasAttribute("Category")) {
        map.category = mapNode.GetAttributeAsString("Category");
      }

      if (mapNode.HasAttribute("id")) {
        map.id = mapNode.GetAttributeAsString("id");
        auto str = "maptimer_mapopen_" + map.id;
        if (HasConfigValue(str)) {
          map.display = GetConfigValue(str);
        }
      }

      if (mapNode.HasAttribute("Length")) {
        mapNode.GetAttributeAsInteger("Length", &map.Length);
      }

      if (mapNode.HasAttribute("Start")) {
        mapNode.GetAttributeAsInteger("Start", &map.Start);
      }

      int start = 0;

      // events
      for (int y = 0; y < mapNode.GetChildCount("Event"); y++) {
        const CXMLNode& eventNode = mapNode.GetChild("Event", y);
        Event event;
        event.length = 0;
        event.start = start;

        if (eventNode.HasAttribute("Name")) {
          event.name = eventNode.GetAttributeAsString("Name");
        }

        if (eventNode.HasAttribute("WorldBossAPIID")) {
          event.worldBossId = eventNode.GetAttributeAsString("WorldBossAPIID");
        }

        if (eventNode.HasAttribute("WayPoint")) {
          event.waypoint = eventNode.GetAttributeAsString("WayPoint");
        }

        if (eventNode.HasAttribute("Length")) {
          eventNode.GetAttributeAsInteger("Length", &event.length);
          start += event.length;
        }

        if (eventNode.HasAttribute("Color")) {
          auto s = eventNode.GetAttributeAsString("Color");
          uint32_t c = 0;
          std::sscanf(s.c_str(), "%x", &c);
          event.color = CColor(c);
        }

        map.events.push_back(event);
      }

      const auto& cat = !map.category.empty() ? map.category : "";
      if (_categoryMapsDict.find(cat) == _categoryMapsDict.end()) {
        _categoryMapsDict.insert(std::make_pair(cat, std::vector<Map>()));
        _categories.emplace_back(cat);
      }
      _categoryMapsDict[cat].push_back(map);
    }

    // categorized maps (group maps by category)
    for (const auto& cat : _categories) {
      std::vector<Map> ms = _categoryMapsDict[cat];
      for (const auto& m : ms) {
        maps.emplace_back(m);
      }
    }
  }
}

void GW2MapTimer::UpdateScrollbarData(int ypos, const CRect& cl) {
  if (!ScrollbarsEnabled()) return;

  CRect BRect = CRect(0, 0, cl.Width(), ypos + 1);

  SetHScrollbarParameters(BRect.x1, BRect.x2, cl.Width());
  SetVScrollbarParameters(BRect.y1, BRect.y2, cl.Height());

  if (cl.Width() >= BRect.Width() && GetHScrollbarPos() != BRect.x1) {
    SetHScrollbarPos(BRect.x1, true);
  }
  if (cl.Height() >= BRect.Height() && GetVScrollbarPos() != BRect.y1) {
    SetVScrollbarPos(BRect.y1, true);
  }
}

GW2MapTimer::GW2MapTimer(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {
  CXMLDocument d;
  if (!d.LoadFromFile("maptimer.xml")) {
    return;
  }

  if (!d.GetDocumentNode().GetChildCount("GW2MapTimer")) {
    return;
  }
  CXMLNode root = d.GetDocumentNode().GetChild("GW2MapTimer");
  SetLayout(root);

  SetID("MapTimer");
  ApplyStyle("overflow-y", "auto", std::vector<std::string>());
}

GW2MapTimer::~GW2MapTimer() {
  if (fetchThread.joinable()) {
    fetchThread.join();
  }
}

CWBItem* GW2MapTimer::Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos) {
  return GW2MapTimer::Create(Root, Pos);
}
