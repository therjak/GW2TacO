#include "MapTimer.h"

#include <string>

#include "Bedrock/BaseLib/string_format.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "GW2API.h"
#include "OverlayConfig.h"

using namespace jsonxx;

void GW2MapTimer::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("MapTimerVisible")) {
    SetConfigValue("MapTimerVisible", 1);
  }

  if (!HasConfigValue("MapTimerCompact")) {
    SetConfigValue("MapTimerCompact", 1);
  }

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

        auto lastDungeonStatus =
            "{\"worldbosses\":" + key->QueryAPI("/v2/account/worldbosses") +
            "}";
        json.parse(lastDungeonStatus);

        if (json.has<Array>("worldbosses")) {
          auto dungeonData = json.get<Array>("worldbosses").values();

          CLightweightCriticalSection cs(&critSec);

          worldBosses.clear();

          for (auto& x : dungeonData) {
            if (!x->is<String>()) continue;

            auto eventName = x->get<String>();
            worldBosses.push_back(eventName);
          }
        }

        lastDungeonStatus =
            "{\"mapchests\":" + key->QueryAPI("/v2/account/mapchests") + "}";
        json.parse(lastDungeonStatus);

        if (json.has<Array>("mapchests")) {
          auto dungeonData = json.get<Array>("mapchests").values();

          CLightweightCriticalSection cs(&critSec);

          mapchests.clear();

          for (auto& x : dungeonData) {
            if (!x->is<String>()) continue;

            auto eventName = x->get<String>();
            mapchests.push_back(eventName);
          }
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

  int32_t timeWindow = 120;
  int32_t mapheight = 40;
  int32_t barheight = 20;

  CRect cl = GetClientRect();

  time_t rawtime;
  time(&rawtime);
  struct tm ptm;
  gmtime_s(&ptm, &rawtime);

  WBITEMSTATE i = GetState();
  CWBFont* f = GetFont(i);

  barheight = f->GetLineHeight();
  mapheight = barheight + f->GetLineHeight();

  if (compact) mapheight = barheight;

  int32_t mapCount = 0;
  for (const auto& m : maps)
    if (m.display) mapCount++;

  DrawBackgroundItem(
      API, CSSProperties.DisplayDescriptor,
      CRect(cl.TopLeft(), CPoint(cl.Width(), mapCount * mapheight + 1)),
      GetState());

  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  int32_t minutes = ptm.tm_hour * 60 + ptm.tm_min;
  int32_t lefttime = minutes - timeWindow / 2;

  int32_t ypos = 0;

  std::vector<CRect> highlightRects;
  int x = 0;
  for (const auto& map : maps)  // int x = 0; x < maps.NumItems(); x++ )
  {
    if (!map.display) {
      continue;
    }

    int32_t currtime = -48 * 60 + map.Length + map.Start - lefttime;
    int32_t currevent = 0;

    if ((ypos > cl.y2) || (ypos < cl.y1 - mapheight)) {
      continue;
    }

    CPoint p = f->GetCenter(
        map.name, CRect(cl.x1, ypos, cl.x2, ypos + mapheight - barheight + 1),
        TextTransform);
    // p.y -= 3;
    if (!compact)
      f->Write(API, map.name, CPoint(p.x, ypos + 2), CColor{0xffffffff},
               TextTransform);

    int32_t toppos = ypos + mapheight - barheight;
    int32_t bottompos = ypos + mapheight + 1;

    {
      CLightweightCriticalSection cs(&critSec);
      if (map.chestId.size() > 0 &&
          std::find(mapchests.begin(), mapchests.end(), map.chestId) !=
              mapchests.end())
        highlightRects.emplace_back(CRect(cl.x1, toppos, cl.x2, bottompos));
    }

    while (currtime < 72 * 60) {
      int32_t p1 = static_cast<int32_t>(cl.Width() * currtime /
                                        static_cast<float>(timeWindow));
      int32_t p2 =
          static_cast<int32_t>(cl.Width() *
                               (currtime + maps[x].events[currevent].length) /
                               static_cast<float>(timeWindow)) +
          1;

      if (p2 >= 0 && p1 <= cl.Width()) {
        CRect r = CRect(max(0, p1), toppos, min(cl.Width(), p2), bottompos);

        API->DrawRect(r, map.events[currevent].color);

        CRect cr = API->GetCropRect();
        API->SetCropRect(ClientToScreen(r));

        auto text = map.events[currevent].name;

        // if ( minutes >= currtime && minutes < currtime +
        // maps[x].events[currevent].length)
        {
          int32_t timeleft =
              currtime * 60 - ptm.tm_sec - timeWindow * 30 +
              map.events[currevent].length *
                  60;  // (currtime + maps[x].events[currevent].length) * 60 -
                       // (minutes * 60 + systime.wSecond);
          if (timeleft >= 0 && timeleft <= map.events[currevent].length * 60) {
            text = FormatString("%s %d:%.2d", text.c_str(), timeleft / 60,
                                timeleft % 60);
          }
        }

        if (ClientToScreen(r).Contains(GetApplication()->GetMousePos())) {
          mouseToolTip = text;
        }

        CPoint p = f->GetCenter(text, r, TextTransform);
        // p.y -= 3;
        f->Write(API, text, CPoint(p.x, r.y1 + 2), CColor{0xffffffff},
                 TextTransform);

        API->SetCropRect(cr);

        bool isHighlighted = false;

        {
          CLightweightCriticalSection cs(&critSec);
          const auto& bossId = map.events[currevent].worldBossId;
          if (!map.events[currevent].worldBossId.empty() &&
              std::find(worldBosses.begin(), worldBosses.end(), bossId) !=
                  worldBosses.end()) {
            isHighlighted = true;
          }
          if (!map.events[currevent].worldBossId.empty() &&
              std::find(mapchests.begin(), mapchests.end(), bossId) !=
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

    ypos += mapheight;
  }

  for (const auto& r : highlightRects) {
    API->DrawRectBorder(r, CColor{0xffffcc00});
  }

  API->DrawRect(CRect(cl.Width() / 2, 0, cl.Width() / 2 + 1, ypos),
                CColor{0x80ffffff});
  SetMouseToolTip(mouseToolTip);
}

void GW2MapTimer::SetLayout(CXMLNode& node) {
  for (int x = 0; x < node.GetChildCount("Map"); x++) {
    CXMLNode& mapNode = node.GetChild("Map", x);

    Map map;
    if (mapNode.HasAttribute("Name"))
      map.name = mapNode.GetAttributeAsString("Name");

    if (mapNode.HasAttribute("ChestAPIID"))
      map.chestId = mapNode.GetAttributeAsString("ChestAPIID");

    if (mapNode.HasAttribute("id")) {
      map.id = mapNode.GetAttributeAsString("id");
      auto str = "maptimer_mapopen_" + map.id;
      if (HasConfigValue(str)) map.display = GetConfigValue(str);
    }

    if (mapNode.HasAttribute("Length"))
      mapNode.GetAttributeAsInteger("Length", &map.Length);

    if (mapNode.HasAttribute("Start"))
      mapNode.GetAttributeAsInteger("Start", &map.Start);

    int start = 0;

    for (int y = 0; y < mapNode.GetChildCount("Event"); y++) {
      CXMLNode& eventNode = mapNode.GetChild("Event", y);
      Event event;
      event.length = 0;
      event.start = start;

      if (eventNode.HasAttribute("Name"))
        event.name = eventNode.GetAttributeAsString("Name");

      if (eventNode.HasAttribute("WorldBossAPIID"))
        event.worldBossId = eventNode.GetAttributeAsString("WorldBossAPIID");

      if (eventNode.HasAttribute("WayPoint"))
        event.waypoint = eventNode.GetAttributeAsString("WayPoint");

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

    maps.push_back(map);
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
}

GW2MapTimer::~GW2MapTimer() = default;

CWBItem* GW2MapTimer::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2MapTimer::Create(Root, Pos).get();
}

bool GW2MapTimer::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                     WBMESSAGE MessageType) {
  return true;
}
