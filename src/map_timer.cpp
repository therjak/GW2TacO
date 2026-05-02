module;
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <format>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/base/color.h"
#include "src/gw2_tactical.h"

module taco.map_timer;

import math;
import taco.gw2;
import taco.gw2taco;
import taco.overlay_config;
import time;
import whiteboard;
import xml;

using math::Point;
using math::Rect;
using math::Size;

bool GW2MapTimer::IsScrollbarVisible() {
  Rect cr = GetClientRect();
  return IsVScrollbarEnabled() && last_ypos_ > cr.Height();
}

void GW2MapTimer::OnResize(const Size& s) {
  if (last_ypos_ > 0) {
    UpdateScrollbarData(last_ypos_, GetClientRect());
  }
}

void GW2MapTimer::OnDraw(gui::CWBDrawAPI* api) {
  if (!GetConfigValue("MapTimerVisible")) {
    return;
  }

  std::string mouse_tool_tip;

  if (GW2::apiKeyManager.GetStatus() == GW2::APIKeyManager::Status::OK) {
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

    if (key && key->Valid() &&
        (GetTime() - last_fetch_time_ > 150000 || !last_fetch_time_)) {
      if (!fetch_task_.valid() || fetch_task_.wait_for(std::chrono::seconds(
                                      0)) == std::future_status::ready) {
        last_fetch_time_ = GetTime();
        fetch_task_ = std::async(std::launch::async, [this, key]() {
          const auto& bosses = key->QuerySet("/v2/account/worldbosses");
          boss_queue_.push(bosses);
          const auto& chests = key->QuerySet("/v2/account/mapchests");
          mapchest_queue_.push(chests);
        });
      }
    }
  }

  const auto& new_boss_data = boss_queue_.pop();
  if (new_boss_data.has_value()) {
    auto boss_data = new_boss_data.value();
    std::swap(world_bosses_, boss_data);
  }
  const auto& new_mapchest_data = mapchest_queue_.pop();
  if (new_mapchest_data.has_value()) {
    auto mapchest_data = new_mapchest_data.value();
    std::swap(mapchests_, mapchest_data);
  }

  bool compact = GetConfigValue("MapTimerCompact");
  bool show_categories = GetConfigValue("MapTimerCategories");

  int32_t time_window = 120;
  int32_t map_height = 40;
  int32_t bar_height = 20;

  int32_t category_line_width = GetConfigValue("MapTimerCategoryLineWidth");
  int32_t padding_left = show_categories ? category_line_width + 4 : 0;

  Rect cl = GetClientRect();

  time_t rawtime = 0;
  time(&rawtime);
  struct tm ptm {};
  gmtime_s(&ptm, &rawtime);

  gui::WBITEMSTATE i = GetState();
  gui::CWBFont* f = GetFont(i);

  bar_height = f->GetLineHeight();
  map_height = bar_height + f->GetLineHeight();

  if (compact) {
    map_height = bar_height;
  }

  int32_t map_count = 0;
  for (const auto& m : maps_) {
    if (m.display) {
      map_count++;
    }
  }

  DrawBackgroundItem(api, CSSProperties.DisplayDescriptor,
                     Rect(Point(cl.x1 + padding_left, cl.y1),
                          Point(cl.Width(), map_count * map_height + 1)),
                     GetState());

  auto text_transform = static_cast<gui::WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, gui::WB_ITEM_TEXTTRANSFORM));

  int32_t minutes = ptm.tm_hour * 60 + ptm.tm_min;
  int32_t left_time = minutes - time_window / 2;

  int32_t scrollbar_pos = GetVScrollbarPos();
  int32_t ypos = -scrollbar_pos;

  std::vector<Rect> highlight_rects;
  int32_t last_category_start_y = 0;
  const Category* last_category = nullptr;
  for (const auto& map : maps_) {
    if (!map.display) {
      continue;
    }

    int32_t currtime = -48 * 60 + map.length + map.start - left_time;
    int32_t currevent = 0;

    bool should_draw = true;
    if ((ypos > cl.y2) || (ypos < cl.y1 - map_height)) {
      should_draw = false;
    }

    if (should_draw) {
      int32_t top_pos = ypos + map_height - bar_height;
      int32_t bottom_pos = ypos + map_height + 1;

      // map name
      {
        Point p = f->GetCenter(
            map.name,
            Rect(cl.x1, ypos, cl.x2, ypos + map_height - bar_height + 1),
            text_transform);
        if (!compact) {
          f->Write(api, map.name, Point(p.x, ypos + 2), CColor{0xffffffff},
                   text_transform);
        }
      }

      // map category
      if (show_categories && !map.category.empty() &&
          categories_.find(map.category) != categories_.end()) {
        const Category& category = categories_[map.category];

        if (&category != last_category) {
          if (last_category && last_category->color.A() > 0) {
            api->DrawRect(Rect(cl.x1, last_category_start_y,
                               cl.x1 + category_line_width, top_pos),
                          last_category->color);
            api->DrawRectBorder(Rect(cl.x1, last_category_start_y,
                                     cl.x1 + category_line_width, top_pos),
                                CColor{0x80000000});
          }
          last_category_start_y = top_pos;
          last_category = &category;
        }

        Rect r = Rect(cl.x1, top_pos, cl.x1 + padding_left, bottom_pos - 1)
                     .GetIntersection(cl);
        if (ClientToScreen(r).Contains(GetApplication()->GetMousePos())) {
          mouse_tool_tip = category.name;
        }
      }

      // highlight rect
      if (!map.chest_id.empty()) {
        if (mapchests_.contains(map.chest_id)) {
          highlight_rects.emplace_back(
              Rect(cl.x1 + padding_left, top_pos, cl.x2, bottom_pos));
        }
      }

      // map events
      while (currtime < 72 * 60) {
        int32_t p1 =
            padding_left + static_cast<int32_t>((cl.Width() - padding_left) *
                                                currtime / time_window);
        int32_t p2 =
            padding_left +
            static_cast<int32_t>((cl.Width() - padding_left) *
                                 (currtime + map.events[currevent].length) /
                                 time_window) +
            1;

        if (p2 >= padding_left && p1 <= cl.Width()) {
          Rect r = Rect(std::max(padding_left, p1), top_pos,
                        std::min(cl.Width(), p2), bottom_pos);

          api->DrawRect(r, map.events[currevent].color);

          Rect cr = api->GetCropRect();
          api->SetCropRect(ClientToScreen(r));

          auto text = map.events[currevent].name;

          {
            int32_t time_left = currtime * 60 - ptm.tm_sec - time_window * 30 +
                                map.events[currevent].length * 60;
            if (time_left >= 0 &&
                time_left <= map.events[currevent].length * 60) {
              text = !text.empty() ? std::format("{:s} {:d}:{:02d}", text,
                                                 time_left / 60, time_left % 60)
                                   : std::format("{:d}:{:02d}", time_left / 60,
                                                 time_left % 60);
            }
          }

          if (ClientToScreen(r.GetIntersection(cl))
                  .Contains(GetApplication()->GetMousePos())) {
            mouse_tool_tip = !map.events[currevent].name.empty()
                                 ? std::format("{:s} - {:s}", map.name, text)
                                 : text;
          }

          Point p = f->GetCenter(text, r, text_transform);
          f->Write(api, text, Point(p.x, r.y1 + 2), CColor{0xffffffff},
                   text_transform);

          api->SetCropRect(cr);

          bool is_highlighted = false;

          const auto& boss_id = map.events[currevent].world_boss_id;
          if (!map.events[currevent].world_boss_id.empty()) {
            if (world_bosses_.contains(boss_id)) {
              is_highlighted = true;
            }
            if (mapchests_.contains(boss_id)) {
              is_highlighted = true;
            }
          }

          if (!is_highlighted) {
            api->DrawRectBorder(r, CColor{0x80000000});
          } else {
            highlight_rects.push_back(r);
          }
        }

        currtime += map.events[currevent].length;
        currevent = (currevent + 1) % map.events.size();
      }
    }

    ypos += map_height;
  }

  if (show_categories) {
    if (last_category && last_category->color.A() > 0) {
      api->DrawRect(
          Rect(cl.x1, last_category_start_y, cl.x1 + category_line_width, ypos),
          last_category->color);
      api->DrawRectBorder(
          Rect(cl.x1, last_category_start_y, cl.x1 + category_line_width, ypos),
          CColor{0x80000000});
    }
  }

  for (const auto& r : highlight_rects) {
    api->DrawRectBorder(r, CColor{0xffffcc00});
  }

  api->DrawRect(Rect(cl.Width() / 2, 0, cl.Width() / 2 + 1, ypos),
                CColor{0x80ffffff});
  SetMouseToolTip(mouse_tool_tip);

  // update scrollbar
  ypos += scrollbar_pos;
  if (last_ypos_ == -1 || ypos != last_ypos_) UpdateScrollbarData(ypos, cl);
  last_ypos_ = ypos;

  // set mouse transparency
  // if there is overflow, this gui item needs to be able to receive mouse
  // events to handle the scrollbar
  SetForcedMouseTransparency(!IsScrollbarVisible());
}

int32_t GW2MapTimer::GetScrollbarStep() { return 5; }

gui::CWBItem* GW2MapTimer::GetItemUnderMouse(Point& point, Rect& crop_rect,
                                             gui::WBMESSAGE message_type) {
  gui::CWBItem* item =
      gui::CWBItem::GetItemUnderMouse(point, crop_rect, message_type);
  if (item && IsScrollbarVisible()) {
    // Only the scrollbar needs to be "visible"

    Rect sr = GetScreenRect();
    Rect b1, su, th, sd, b2;
    GetVScrollbarRectangles(b1, su, th, sd, b2);

    b1.Move(sr.x1, sr.y1);
    return (point.x >= b1.x1 && point.x <= b1.x2) &&
                   (point.y >= sr.y1 && point.y <= sr.y2)
               ? this
               : nullptr;
  }
  return item;
}

void GW2MapTimer::SetLayout(const CXMLNode& node) {
  std::unordered_map<std::string, std::vector<Map>> category_maps_dict;
  std::vector<std::string> categories_list;

  // categories
  const CXMLNode& categories_node = node.GetChild("Categories");
  if (categories_node.IsValid()) {
    for (int x = 0; x < categories_node.GetChildCount("Category"); ++x) {
      const CXMLNode& category_node = categories_node.GetChild("Category", x);

      Category category;
      if (category_node.HasAttribute("id")) {
        category.id = category_node.GetAttributeAsString("id");
      }

      if (category_node.HasAttribute("Name")) {
        category.name = category_node.GetAttributeAsString("Name");
      }

      if (category_node.HasAttribute("Color")) {
        auto s = category_node.GetAttributeAsString("Color");
        uint32_t c = 0;
        std::sscanf(s.c_str(), "%x", &c);
        category.color = CColor(c);
      }

      categories_[category.id] = category;
    }
  }

  // maps
  const CXMLNode& maps_node = node.GetChild("Maps");
  if (maps_node.IsValid()) {
    for (int x = 0; x < maps_node.GetChildCount("Map"); x++) {
      const CXMLNode& map_node = maps_node.GetChild("Map", x);

      Map map;
      if (map_node.HasAttribute("Name")) {
        map.name = map_node.GetAttributeAsString("Name");
      }

      if (map_node.HasAttribute("ChestAPIID")) {
        map.chest_id = map_node.GetAttributeAsString("ChestAPIID");
      }

      if (map_node.HasAttribute("Category")) {
        map.category = map_node.GetAttributeAsString("Category");
      }

      if (map_node.HasAttribute("id")) {
        map.id = map_node.GetAttributeAsString("id");
        auto str = "maptimer_mapopen_" + map.id;
        if (HasConfigValue(str)) {
          map.display = GetConfigValue(str);
        }
      }

      if (map_node.HasAttribute("Length")) {
        map_node.GetAttributeAsInteger("Length", &map.length);
      }

      if (map_node.HasAttribute("Start")) {
        map_node.GetAttributeAsInteger("Start", &map.start);
      }

      int start = 0;

      // events
      for (int y = 0; y < map_node.GetChildCount("Event"); y++) {
        const CXMLNode& event_node = map_node.GetChild("Event", y);
        Event event;
        event.length = 0;
        event.start = start;

        if (event_node.HasAttribute("Name")) {
          event.name = event_node.GetAttributeAsString("Name");
        }

        if (event_node.HasAttribute("WorldBossAPIID")) {
          event.world_boss_id =
              event_node.GetAttributeAsString("WorldBossAPIID");
        }

        if (event_node.HasAttribute("WayPoint")) {
          event.waypoint = event_node.GetAttributeAsString("WayPoint");
        }

        if (event_node.HasAttribute("Length")) {
          event_node.GetAttributeAsInteger("Length", &event.length);
          start += event.length;
        }

        if (event_node.HasAttribute("Color")) {
          auto s = event_node.GetAttributeAsString("Color");
          uint32_t c = 0;
          std::sscanf(s.c_str(), "%x", &c);
          event.color = CColor(c);
        }

        map.events.push_back(event);
      }

      const auto& cat = !map.category.empty() ? map.category : "";
      if (category_maps_dict.find(cat) == category_maps_dict.end()) {
        category_maps_dict.insert(std::make_pair(cat, std::vector<Map>()));
        categories_list.emplace_back(cat);
      }
      category_maps_dict[cat].push_back(map);
    }

    // categorized maps (group maps by category)
    for (const auto& cat : categories_list) {
      std::vector<Map> ms = category_maps_dict[cat];
      for (const auto& m : ms) {
        maps_.emplace_back(m);
      }
    }
  }
}

void GW2MapTimer::UpdateScrollbarData(int ypos, const Rect& cl) {
  if (!ScrollbarsEnabled()) return;

  Rect b_rect = Rect(0, 0, cl.Width(), ypos + 1);

  SetHScrollbarParameters(b_rect.x1, b_rect.x2, cl.Width());
  SetVScrollbarParameters(b_rect.y1, b_rect.y2, cl.Height());

  if (cl.Width() >= b_rect.Width() && GetHScrollbarPos() != b_rect.x1) {
    SetHScrollbarPos(b_rect.x1, true);
  }
  if (cl.Height() >= b_rect.Height() && GetVScrollbarPos() != b_rect.y1) {
    SetVScrollbarPos(b_rect.y1, true);
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

gui::CWBItem* GW2MapTimer::Factory(gui::CWBItem* root, const CXMLNode& node,
                                   Rect& pos) {
  return GW2MapTimer::Create(root, pos);
}
