module;
#include <algorithm>
#include <ctime>
#include <format>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/gw2_tactical.h"

module taco.map_timer;

import math;
import taco.gw2;
import taco.gw2taco;
import taco.overlay_config;
import taco.time;
import whiteboard;
import xml;

using math::Point;
using math::Rect;
using math::Size;

bool GW2MapTimer::IsScrollbarVisible() {
  Rect cr = GetClientRect();
  return IsVScrollbarEnabled() && lastypos > cr.Height();
}

void GW2MapTimer::OnResize(const Size& s) {
  if (lastypos > 0) {
    UpdateScrollbarData(lastypos, GetClientRect());
  }
}

void GW2MapTimer::OnDraw(gui::CWBDrawAPI* API) {
  if (!GetConfigValue("MapTimerVisible")) {
    return;
  }

  std::string mouseToolTip;

  if (GW2::apiKeyManager.GetStatus() == GW2::APIKeyManager::Status::OK) {
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

    if (key && key->Valid() &&
        (GetTime() - lastFetchTime > 150000 || !lastFetchTime)) {
      if (!fetchTask.valid() || fetchTask.wait_for(std::chrono::seconds(0)) ==
                                    std::future_status::ready) {
        lastFetchTime = GetTime();
        fetchTask = std::async(std::launch::async, [this, key]() {
          const auto& bosses = key->QuerySet("/v2/account/worldbosses");
          boss_queue.push(bosses);
          const auto& chests = key->QuerySet("/v2/account/mapchests");
          mapchest_queue.push(chests);
        });
      }
    }
  }

  const auto& new_boss_data = boss_queue.pop();
  if (new_boss_data.has_value()) {
    auto boss_data = new_boss_data.value();
    std::swap(world_bosses, boss_data);
  }
  const auto& new_mapchest_data = mapchest_queue.pop();
  if (new_mapchest_data.has_value()) {
    auto mapchest_data = new_mapchest_data.value();
    std::swap(mapchests, mapchest_data);
  }

  bool compact = GetConfigValue("MapTimerCompact");
  bool showCategories = GetConfigValue("MapTimerCategories");

  int32_t timeWindow = 120;
  int32_t mapheight = 40;
  int32_t barheight = 20;

  int32_t categoryLineWidth = GetConfigValue("MapTimerCategoryLineWidth");
  int32_t paddingLeft = showCategories ? categoryLineWidth + 4 : 0;

  Rect cl = GetClientRect();

  time_t rawtime = 0;
  time(&rawtime);
  struct tm ptm {};
  gmtime_s(&ptm, &rawtime);

  gui::WBITEMSTATE i = GetState();
  gui::CWBFont* f = GetFont(i);

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
                     Rect(Point(cl.x1 + paddingLeft, cl.y1),
                           Point(cl.Width(), mapCount * mapheight + 1)),
                     GetState());

  auto TextTransform = static_cast<gui::WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, gui::WB_ITEM_TEXTTRANSFORM));

  int32_t minutes = ptm.tm_hour * 60 + ptm.tm_min;
  int32_t lefttime = minutes - timeWindow / 2;

  int32_t scrollbarPos = GetVScrollbarPos();
  int32_t ypos = -scrollbarPos;

  std::vector<Rect> highlightRects;
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
        Point p = f->GetCenter(
            map.name,
            Rect(cl.x1, ypos, cl.x2, ypos + mapheight - barheight + 1),
            TextTransform);
        if (!compact) {
          f->Write(API, map.name, Point(p.x, ypos + 2), CColor{0xffffffff},
                   TextTransform);
        }
      }

      // map category
      if (showCategories && !map.category.empty() &&
          categories.find(map.category) != categories.end()) {
        const Category& category = categories[map.category];

        if (&category != lastCategory) {
          if (lastCategory && lastCategory->color.A() > 0) {
            API->DrawRect(Rect(cl.x1, lastCategoryStartY,
                                cl.x1 + categoryLineWidth, toppos),
                          lastCategory->color);
            API->DrawRectBorder(Rect(cl.x1, lastCategoryStartY,
                                      cl.x1 + categoryLineWidth, toppos),
                                CColor{0x80000000});
          }
          lastCategoryStartY = toppos;
          lastCategory = &category;
        }

        Rect r = Rect(cl.x1, toppos, cl.x1 + paddingLeft, bottompos - 1)
                      .GetIntersection(cl);
        if (ClientToScreen(r).Contains(GetApplication()->GetMousePos())) {
          mouseToolTip = category.name;
        }
      }

      // highlight rect
      if (!map.chestId.empty()) {
        if (mapchests.contains(map.chestId)) {
          highlightRects.emplace_back(
              Rect(cl.x1 + paddingLeft, toppos, cl.x2, bottompos));
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
          Rect r = Rect(std::max(paddingLeft, p1), toppos,
                          std::min(cl.Width(), p2), bottompos);

          API->DrawRect(r, map.events[currevent].color);

          Rect cr = API->GetCropRect();
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

          Point p = f->GetCenter(text, r, TextTransform);
          f->Write(API, text, Point(p.x, r.y1 + 2), CColor{0xffffffff},
                   TextTransform);

          API->SetCropRect(cr);

          bool isHighlighted = false;

          const auto& bossId = map.events[currevent].worldBossId;
          if (!map.events[currevent].worldBossId.empty()) {
            if (world_bosses.contains(bossId)) {
              isHighlighted = true;
            }
            if (mapchests.contains(bossId)) {
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
          Rect(cl.x1, lastCategoryStartY, cl.x1 + categoryLineWidth, ypos),
          lastCategory->color);
      API->DrawRectBorder(
          Rect(cl.x1, lastCategoryStartY, cl.x1 + categoryLineWidth, ypos),
          CColor{0x80000000});
    }
  }

  for (const auto& r : highlightRects) {
    API->DrawRectBorder(r, CColor{0xffffcc00});
  }

  API->DrawRect(Rect(cl.Width() / 2, 0, cl.Width() / 2 + 1, ypos),
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

gui::CWBItem* GW2MapTimer::GetItemUnderMouse(Point& Pos, Rect& CropRect,
                                             gui::WBMESSAGE MessageType) {
  gui::CWBItem* item =
      gui::CWBItem::GetItemUnderMouse(Pos, CropRect, MessageType);
  if (item && IsScrollbarVisible()) {
    // Only the scrollbar needs to be "visible"

    Rect sr = GetScreenRect();
    Rect b1, su, th, sd, b2;
    GetVScrollbarRectangles(b1, su, th, sd, b2);

    b1.Move(sr.x1, sr.y1);
    return (Pos.x >= b1.x1 && Pos.x <= b1.x2) &&
                   (Pos.y >= sr.y1 && Pos.y <= sr.y2)
               ? this
               : nullptr;
  }
  return item;
}

void GW2MapTimer::SetLayout(const CXMLNode& node) {
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

void GW2MapTimer::UpdateScrollbarData(int ypos, const Rect& cl) {
  if (!ScrollbarsEnabled()) return;

  Rect BRect = Rect(0, 0, cl.Width(), ypos + 1);

  SetHScrollbarParameters(BRect.x1, BRect.x2, cl.Width());
  SetVScrollbarParameters(BRect.y1, BRect.y2, cl.Height());

  if (cl.Width() >= BRect.Width() && GetHScrollbarPos() != BRect.x1) {
    SetHScrollbarPos(BRect.x1, true);
  }
  if (cl.Height() >= BRect.Height() && GetVScrollbarPos() != BRect.y1) {
    SetVScrollbarPos(BRect.y1, true);
  }
}

GW2MapTimer::GW2MapTimer() : CWBGuiType() {
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

GW2MapTimer::~GW2MapTimer() {}

gui::CWBItem* GW2MapTimer::Factory(gui::CWBItem* Root, const CXMLNode& node,
                                   Rect& Pos) {
  return GW2MapTimer::Create(Root, Pos);
}
