#include "WvW.h"

#include <ctime>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Bedrock/BaseLib/string_format.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "GW2API.h"
#include "Language.h"
#include "OverlayConfig.h"
#include "gw2tactical.h"

using namespace jsonxx;

bool wvwCanBeRendered = false;
std::vector<WvWObjective> wvwObjectives;
std::string FetchHTTPS(std::string_view url, std::string_view path);
std::unordered_map<std::string, POI> wvwPOIs;
GW2TacticalCategory* GetCategory(std::string_view s);
std::unordered_map<int, bool> wvwMapIDs;

std::thread wvwPollThread;

#define DAYFLAG 0x001000
#define DHMSFLAG 0x001111
#define HOURFLAG 0x000100
#define HMSFLAG 0x000111
#define MINFLAG 0x000010
#define MSFLAG 0x000011
#define SECFLAG 0x000001

void parseISO8601(const char* text, time_t& isotime, char& flag) {
  const char* c;
  int num;

  struct tm tmstruct;

  int year = 0;
  int month = 0;
  int seconds = 0;
  int minutes = 0;
  int hours = 0;
  int days = 0;

  int dateflags = 0; /* flag which date component we've seen */

  c = text;
  isotime = 0;

  if (*c++ == 'P') {
    /* duration */
    flag = 'D';
    while (*c != '\0') {
      num = 0;
      while (*c >= '0' && *c <= '9') {
        /* assumes ASCII sequence! */
        num = 10 * num + *c++ - '0';
      }

      switch (*c++) {
        case 'D':
          if (dateflags & DHMSFLAG) {
            /* day, hour, min or sec already set */
            return;
          } else {
            dateflags |= DAYFLAG;
            days = num;
          }
          break;
        case 'H':
          if (dateflags & HMSFLAG) {
            /* hour, min or sec already set */
            return;
          } else {
            dateflags |= DAYFLAG;
            hours = num;
          }
          break;
        case 'M':
          if (dateflags & MSFLAG) {
            /* min or sec already set */
            return;
          } else {
            dateflags |= MINFLAG;
            minutes = num;
          }
          break;
        case 'S':
          if (dateflags & SECFLAG) {
            /* sec already set */
            return;
          } else {
            dateflags |= SECFLAG;
            seconds = num;
          }
          break;
        default:
          return;
      }
    }
    isotime = seconds + 60 * minutes + 3600 * hours + 86400 * days;
  } else {
    /* point in time, must be one of
    CCYYMMDD
    CCYY-MM-DD
    CCYYMMDDTHHMM
    CCYY-MM-DDTHH:MM
    CCYYMMDDTHHMMSS
    CCYY-MM-DDTHH:MM:SS
    */
    c = text;
    flag = 'T';

    /* NOTE: we have to check for the extended format first,
    because otherwise the separting '-' will be interpreted
    by sscanf as signs of a 1 digit integer .... :-(  */

    if (sscanf_s(text, "%4u-%2u-%2u", &year, &month, &days) == 3) {
      c += 10;
    } else if (sscanf_s(text, "%4u%2u%2u", &year, &month, &days) == 3) {
      c += 8;
    } else {
      return;
    }

    tmstruct.tm_year = year - 1900;
    tmstruct.tm_mon = month - 1;
    tmstruct.tm_mday = days;

    if (*c == '\0') {
      tmstruct.tm_hour = 0;
      tmstruct.tm_sec = 0;
      tmstruct.tm_min = 0;
      isotime = _mkgmtime(&tmstruct);
    } else if (*c == 'T') {
      /* time of day part */
      c++;
      if (sscanf_s(c, "%2d%2d", &hours, &minutes) == 2) {
        c += 4;
      } else if (sscanf_s(c, "%2d:%2d", &hours, &minutes) == 2) {
        c += 5;
      } else {
        return;
      }

      if (*c == ':') {
        c++;
      }

      if (*c != '\0') {
        if (sscanf_s(c, "%2d", &seconds) == 1) {
          c += 2;
        } else {
          return;
        }
        if (*c != '\0' && *c != 'Z') { /* something left? */
          return;
        }
      }
      tmstruct.tm_hour = hours;
      tmstruct.tm_min = minutes;
      tmstruct.tm_sec = seconds;
      isotime = _mkgmtime(&tmstruct);
    }

    else {
      return;
    }
  }
}

void LoadWvWObjectives() {
  // https://api.guildwars2.com/v2/wvw/objectives

  wvwPollThread = std::thread([]() {
    std::unordered_map<int, CVector3> wvwObjectiveCoords;
    std::unordered_map<int, CRect> wvwContinentRects;

    auto wvwobjectives =
        FetchHTTPS("api.guildwars2.com", "/v2/wvw/objectives?ids=all");

    Array wvwobjs;
    wvwobjs.parse(wvwobjectives);
    auto objs = wvwobjs.values();

    for (auto& x : objs) {
      if (!x->is<Object>()) continue;

      auto obj = x->get<Object>();

      if (!obj.has<String>("id")) continue;

      auto objid = obj.get<String>("id");

      int mapID, objident;
      if (std::sscanf(objid.c_str(), "%d-%d", &mapID, &objident) != 2) continue;

      if (!obj.has<Number>("map_id")) continue;

      if (obj.get<Number>("map_id") != mapID) continue;

      wvwMapIDs[mapID] = true;

      if (obj.has<Array>("coord")) {
        if (wvwContinentRects.find(mapID) == wvwContinentRects.end()) {
          auto mapPath = FormatString("/v2/maps?id=%d", mapID);
          auto wvwMapData = FetchHTTPS("api.guildwars2.com", mapPath);

          Object map;
          map.parse(wvwMapData);
          if (!map.has<Array>("continent_rect")) continue;

          auto continentRectArray = map.get<Array>("continent_rect").values();
          if (continentRectArray.size() != 2) continue;

          int continentRectCnt = 0;
          int continentRectValues[4];
          bool ok = true;

          for (int x = 0; x < 2; x++) {
            if (!continentRectArray[x]->is<Array>()) {
              ok = false;
              break;
            }
            auto continentRectCoords =
                continentRectArray[x]->get<Array>().values();
            if (continentRectCoords.size() != 2) {
              ok = false;
              break;
            }

            for (int y = 0; y < 2; y++) {
              if (!continentRectCoords[y]->is<Number>()) {
                ok = false;
                break;
              }
              continentRectValues[continentRectCnt++] =
                  static_cast<int>(continentRectCoords[y]->get<Number>());
            }
          }

          if (ok)
            wvwContinentRects[mapID] =
                CRect(continentRectValues[0], continentRectValues[1],
                      continentRectValues[2], continentRectValues[3]);
        }

        if (wvwContinentRects.find(mapID) == wvwContinentRects.end()) {
          continue;
        }

        auto coord = obj.get<Array>("coord").values();
        if (coord.size() == 3) {
          CVector3 v(coord[0]->is<Number>()
                         ? static_cast<float>(coord[0]->get<Number>())
                         : 0,
                     coord[1]->is<Number>()
                         ? static_cast<float>(coord[1]->get<Number>())
                         : 0,
                     coord[2]->is<Number>()
                         ? static_cast<float>(coord[2]->get<Number>())
                         : 0);

          CRect& r = wvwContinentRects[mapID];
          CVector3 offset =
              CVector3((r.x1 + r.x2) / 2.0f, 0, (r.y1 + r.y2) / 2.0f);

          if (objident == 15 && abs(v.x - 11766.3) < 1 &&
              abs(v.y - 14793.5) < 1 &&
              abs(v.z - (-2133.39)) < 1)  // Langor fix-hack
          {
            v.x = 11462.5;
            v.y = 15600 - 2650 / 24;
            v.z -= 500;
          }

          wvwObjectiveCoords[objident] = CVector3(
              GameToWorldCoords((v.x - offset.x) * 24), GameToWorldCoords(-v.z),
              GameToWorldCoords((-(v.y - offset.z)) * 24));
        }
      }

      if (wvwObjectiveCoords.find(objident) == wvwObjectiveCoords.end()) {
        continue;
      }

      WvWObjective o;
      o.id = objid;
      o.mapID = mapID;
      o.objectiveID = objident;
      o.coord = wvwObjectiveCoords[objident];

      if (obj.has<String>("type")) o.type = obj.get<String>("type");

      if (obj.has<String>("name"))
        o.nameToken = o.name = obj.get<String>("name");

      for (char& n : o.nameToken)
        if (!isalnum(n))
          n = '_';
        else
          n = tolower(n);

      extern WBATLASHANDLE DefaultIconHandle;
      extern CSize DefaultIconSize;

      POI poi;
      poi.position = o.coord;
      poi.mapID = o.mapID;
      poi.icon = DefaultIconHandle;
      poi.wvwObjectiveID = wvwObjectives.size();
      // poi.iconSize = DefaultIconSize;

      wvwObjectives.push_back(o);

      CoCreateGuid(&poi.guid);

      auto cat = GetCategory("Tactical.WvW." + o.type);

      extern std::unique_ptr<CWBApplication> App;

      if (cat) poi.SetCategory(App.get(), cat);

      poi.typeData.behavior = POIBehavior::WvWObjective;

      wvwPOIs[o.id] = poi;
    }

    UpdateWvWStatus();

    wvwCanBeRendered = true;
  });
}

bool wvwUpdating = false;
int lastWvWUpdateTime = 0;
std::thread wvwUpdatThread;

#include "MumbleLink.h"

void UpdateWvWStatus() {
  if (wvwUpdating) return;

  if (wvwMapIDs.find(mumbleLink.mapID) == wvwMapIDs.end()) {
    return;
  }

  int currTime = GetTime();
  if (currTime - lastWvWUpdateTime < 5000) return;

  if (wvwUpdatThread.joinable()) wvwUpdatThread.join();

  wvwUpdating = true;

  wvwUpdatThread = std::thread([]() {
    GW2::APIKeyManager::Status status = GW2::apiKeyManager.GetStatus();
    if (status != GW2::APIKeyManager::Status::OK) {
      lastWvWUpdateTime = GetTime();
      wvwUpdating = false;
      return;
    }
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();
    if (!key) {
      lastWvWUpdateTime = GetTime();
      wvwUpdating = false;
      return;
    }

    if (!key->valid) {
      lastWvWUpdateTime = GetTime();
      wvwUpdating = false;
      return;
    }
    if (!key->HasCaps("account")) {
      lastWvWUpdateTime = GetTime();
      wvwUpdating = false;
      return;
    }

    auto apiPath = FormatString("/v2/wvw/matches?world=%d", key->worldId);
    auto wvwobjectiveids = FetchHTTPS("api.guildwars2.com", apiPath);

    Object o;
    o.parse(wvwobjectiveids);
    if (o.has<Array>("maps")) {
      auto m = o.get<Array>("maps").values();
      for (auto& x : m) {
        if (!x->is<Object>()) continue;

        auto map = x->get<Object>();

        if (!map.has<Array>("objectives")) continue;

        auto objs = map.get<Array>("objectives").values();
        for (auto& obj : objs) {
          if (!obj->is<Object>()) continue;
          auto objective = obj->get<Object>();

          std::string id;
          if (objective.has<String>("id"))
            id = objective.get<String>("id");
          else
            continue;

          if (wvwPOIs.find(id) == wvwPOIs.end()) continue;

          auto& poi = wvwPOIs[id];
          poi.typeData.color = CColor{0xffffffff};

          std::string owner;
          if (objective.has<String>("owner"))
            owner = objective.get<String>("owner");

          if (owner == "Red") poi.typeData.color = CColor{0xffe53b3b};

          if (owner == "Green") poi.typeData.color = CColor{0xff3dca67};

          if (owner == "Blue") poi.typeData.color = CColor{0xff3aa2fa};

          std::string lastFlipped;
          if (objective.has<String>("last_flipped"))
            lastFlipped = objective.get<String>("last_flipped");

          time_t flipTime;
          char flags;
          parseISO8601(lastFlipped.c_str(), flipTime, flags);
          poi.lastUpdateTime = flipTime;
        }
      }
    }

    lastWvWUpdateTime = GetTime();
    wvwUpdating = false;
  });
}
