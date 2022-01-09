#include "src/gw2_tactical.h"

#include <mmsystem.h>

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "src/base/file_list.h"
#include "src/base/logger.h"
#include "src/base/plane.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/gw2_api.h"
#include "src/language.h"
#include "src/mumble_link.h"
#include "src/overlay_application.h"
#include "src/overlay_config.h"
#include "src/trail_logger.h"
#include "src/util/jsonxx.h"
#include "src/util/miniz.h"
#include "src/util/png_decompressor.h"
#include "src/white_board/application.h"
#include "src/wvw.h"

using namespace jsonxx;
using math::CMatrix4x4;
using math::CPlane;
using math::CPoint;
using math::CRect;
using math::CSize;
using math::CVector2;
using math::CVector3;
using math::CVector4;

WBATLASHANDLE DefaultIconHandle = -1;
WBATLASHANDLE forbiddenIconHandle = -1;
CSize forbiddenIconSize;
std::unordered_map<std::string, WBATLASHANDLE> MapIcons;
GW2TacticalCategory CategoryRoot;
std::unordered_map<std::string, GW2TacticalCategory*> CategoryMap;
int32_t useMetricDisplay = 0;

std::string emptyString;
std::vector<std::string> stringArray;
float GetUIScale();

float globalOpacity = 1.0f;
float minimapOpacity = 1.0f;

namespace std {
std::size_t hash<POIActivationDataKey>::operator()(
    const POIActivationDataKey& adk) const {
  std::stringstream ss;
  ss << std::hex << adk.guid.Data1 << adk.guid.Data2 << adk.guid.Data3
     << adk.guid.Data4 << adk.uniqueData;
  return std::hash<std::string>()(ss.str());
}
}  // namespace std

int32_t AddStringToMap(std::string_view string) {
  if (string.empty()) return -1;
  auto pos = std::find(stringArray.begin(), stringArray.end(), string);
  if (pos != stringArray.end()) {
    return std::distance(stringArray.begin(), pos);
  }
  stringArray.emplace_back(string);
  return stringArray.size() - 1;
}

std::string& GetStringFromMap(int32_t idx) {
  if (idx < 0 || idx >= stringArray.size()) return emptyString;
  return stringArray[idx];
}

int32_t tacoStartTime = timeGetTime();

int32_t GetTime() { return timeGetTime() - tacoStartTime; }

void UpdateWvWStatus();

void FindClosestRouteMarkers(bool force) {
  for (auto& r : Routes) {
    if (!force && r.activeItem != -1) continue;

    if (r.MapID == mumbleLink.mapID && r.hasResetPos &&
        (r.resetPos - mumbleLink.charPosition).Length() < r.resetRad)
      r.activeItem = 0;

    float closestdist = 1000000000;
    int32_t closest = -1;

    for (int32_t y = 0; y < r.route.size(); y++) {
      GUID g = r.route[y];
      if (POIs.find(g) != POIs.end()) {
        POI& p = POIs[g];
        if (!(p.mapID == mumbleLink.mapID)) continue;

        float dist = (p.position - mumbleLink.charPosition).Length();
        if (dist < closestdist) {
          closestdist = dist;
          closest = y;
          r.activeItem = y;
        }
      }
    }
  }
}

GW2TacticalCategory* GetCategory(std::string_view sv) {
  std::string s(sv);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (CategoryMap.find(s) != CategoryMap.end()) return CategoryMap[s];
  return nullptr;
}

std::unordered_map<std::string, std::unique_ptr<mz_zip_archive>> zipDict;

void FlushZipDict() {
  for (auto& kv : zipDict) {
    if (kv.second) mz_zip_reader_end(kv.second.get());
  }
  zipDict.clear();
}

mz_zip_archive* OpenZipFile(std::string_view zf) {
  std::string zipFile(zf);
  if (zipDict.find(zipFile) == zipDict.end()) {
    auto zip = std::make_unique<mz_zip_archive>();
    memset(zip.get(), 0, sizeof(mz_zip_archive));

    if (!mz_zip_reader_init_file(zip.get(), zipFile.c_str(), 0)) {
      LOG_ERR("[GW2TacO] Failed to open zip archive %s", zipFile.c_str());
      zipDict[zipFile] = nullptr;
    } else {
      zipDict[zipFile] = std::move(zip);
    }
  }

  return zipDict[zipFile].get();
}

WBATLASHANDLE GetMapIcon(CWBApplication* App, std::string_view fname,
                         std::string_view zipFile,
                         std::string_view categoryZip) {
  std::string filename(fname);
  for (char& x : filename)
    if (x == '\\') x = '/';

  auto s = (zipFile.size() ? (std::string(zipFile) + "\\") : "") + filename;
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (MapIcons.find(s) != MapIcons.end()) return MapIcons[s];

  if (DefaultIconHandle == -1) {
    DefaultIconHandle =
        App->GetSkin()
            ->GetElement(App->GetSkin()->GetElementID("defaulticon"))
            ->GetHandle();
  }

  if (forbiddenIconHandle == -1) {
    forbiddenIconHandle =
        App->GetSkin()
            ->GetElement(App->GetSkin()->GetElementID("forbiddenicon"))
            ->GetHandle();
    forbiddenIconSize = App->GetAtlas()->GetSize(forbiddenIconHandle);
  }

  if (!zipFile.empty() || !categoryZip.empty()) {
    // we didn't find an entry from within the zip file, try to load it

    for (int x = 0; x < 2; x++) {
      if (zipFile.empty() && x == 0) continue;

      if (categoryZip.empty() && x == 1) continue;

      mz_zip_archive* zip =
          x == 0 ? OpenZipFile(zipFile) : OpenZipFile(categoryZip);

      if (zip) {
        int idx = mz_zip_reader_locate_file(zip, filename.data(), nullptr, 0);
        if (idx >= 0 && !mz_zip_reader_is_file_a_directory(zip, idx)) {
          mz_zip_archive_file_stat stat;
          if (mz_zip_reader_file_stat(zip, idx, &stat) &&
              stat.m_uncomp_size > 0) {
            auto data = std::make_unique<uint8_t[]>(
                static_cast<int32_t>(stat.m_uncomp_size));

            if (mz_zip_reader_extract_to_mem(
                    zip, idx, data.get(),
                    static_cast<int32_t>(stat.m_uncomp_size), 0)) {
              std::unique_ptr<uint8_t[]> imageData;
              int32_t xres, yres;
              if (DecompressPNG(data.get(),
                                static_cast<int32_t>(stat.m_uncomp_size),
                                imageData, xres, yres)) {
                ARGBtoABGR(imageData.get(), xres, yres);

                auto handle = App->GetAtlas()->AddImage(
                    imageData.get(), xres, yres, CRect(0, 0, xres, yres));

                MapIcons[s] = handle;

                return handle;
              } else {
                LOG_ERR("[GWTacO] Failed to decompress png %s form archive %s",
                        filename.c_str(),
                        x == 0 ? std::string(zipFile).c_str()
                               : std::string(categoryZip).c_str());
              }
            }
          }
        }
      }
    }

    // zipfile load failed, fall back to regular load and add it as an alias
    WBATLASHANDLE handle = GetMapIcon(App, filename, "", "");
    if (handle == DefaultIconHandle) return handle;
    MapIcons[s] = handle;
    return handle;
  }

  CStreamReaderMemory f;
  if (!f.Open(s) && !f.Open("POIs\\" + s)) {
    LOG_ERR("[GWTacO] Failed to open image %s", s.c_str());
    return DefaultIconHandle;
  }

  std::unique_ptr<uint8_t[]> imageData;
  int32_t xres, yres;
  if (!DecompressPNG(f.GetData(), static_cast<int32_t>(f.GetLength()),
                     imageData, xres, yres)) {
    LOG_ERR("[GWTacO] Failed to decompress png %s", s.c_str());
    return DefaultIconHandle;
  }

  ARGBtoABGR(imageData.get(), xres, yres);

  auto handle = App->GetAtlas()->AddImage(imageData.get(), xres, yres,
                                          CRect(0, 0, xres, yres));

  MapIcons[s] = handle;
  return handle;
}

std::unordered_map<GUID, POI> POIs;
std::unordered_map<POIActivationDataKey, POIActivationData> ActivationData;

std::vector<POIRoute> Routes;

uint32_t DictionaryHash(const GUID& i) {
  uint8_t* dta = (uint8_t*)(&i);
  uint32_t Hash = 5381;
  for (int x = 0; x < sizeof(GUID); x++)
    Hash = ((Hash << 5) + Hash) + dta[x];  // hash * 33 + c
  return Hash;
}

uint32_t DictionaryHash(const POIActivationDataKey& i) {
  uint8_t* dta = (uint8_t*)(&i);
  uint32_t Hash = 5381;
  for (int x = 0; x < sizeof(POIActivationDataKey); x++)
    Hash = ((Hash << 5) + Hash) + dta[x];  // hash * 33 + c
  return Hash;
}

float distPointPlane(CVector3 vPoint, CPlane plane) {
  return vPoint * plane.Normal + plane.D;
}

float distRayPlane(CVector3 vRayOrigin, CVector3 vnRayVector,
                   CVector3 vnPlaneNormal, float planeD) {
  float cosAlpha;
  float deltaD;

  cosAlpha = vnRayVector * vnPlaneNormal;
  // parallel to the plane (alpha=90)
  if (cosAlpha == 0) return -1.0f;
  deltaD = planeD - vRayOrigin * vnPlaneNormal;

  return (deltaD / cosAlpha);
}

bool testfrustum(CVector3 c, CPlane planes[4], int skip) {
  bool v = c.z > 0;
  for (int x = 0; x < 4; x++)
    if (x != skip) v = v && (distPointPlane(c, planes[x]) < 0);
  return v;
}

CVector3 GW2TacticalDisplay::ProjectTacticalPos(CVector3 pos, float fov,
                                                float asp) {
  CVector3 p = pos;
  float length = p.Length();

  float yfov = fov / 2.0f;

  CVector3 fln, frn, fun, fdn;
  CMatrix4x4 rotm;

  float xfov = std::atan(asp * std::tan(yfov));

  rotm = CMatrix4x4::Rotation(CVector3(0, 1, 0), -xfov);
  fln = CVector3(CVector3(-1, 0, 0) * rotm);
  rotm = CMatrix4x4::Rotation(CVector3(0, 1, 0), xfov);
  frn = CVector3(CVector3(1, 0, 0) * rotm);
  rotm = CMatrix4x4::Rotation(CVector3(1, 0, 0), -yfov);
  fun = CVector3(CVector3(0, 1, 0) * rotm);
  rotm = CMatrix4x4::Rotation(CVector3(1, 0, 0), yfov);
  fdn = CVector3(CVector3(0, -1, 0) * rotm);

  CPlane fplanes[4];
  fplanes[0] = CPlane(CVector3(0, 0, 0), fln);
  fplanes[1] = CPlane(CVector3(0, 0, 0), frn);
  fplanes[2] = CPlane(CVector3(0, 0, 0), fun);
  fplanes[3] = CPlane(CVector3(0, 0, 0), fdn);

  if (!testfrustum(p, fplanes, -1)) {
    CVector3 o = p;
    CVector3 res[4];
    float di[4];

    CVector3 vn = (p - CVector3(0, 0, 1)).Normalized();
    di[0] = distRayPlane(p, vn, fln, 0);
    di[1] = distRayPlane(p, vn, frn, 0);
    di[2] = distRayPlane(p, vn, fun, 0);
    di[3] = distRayPlane(p, vn, fdn, 0);

    bool ok[4];
    float m = 0;
    for (int x = 0; x < 4; x++) {
      res[x] = o + vn * di[x];
      ok[x] = testfrustum(res[x], fplanes, x);
      if (ok[x]) {
        di[x] = (o - res[x]).Length();
        m = di[x];
        p = res[x];
      }
    }

    for (int x = 0; x < 4; x++)
      if (ok[x] && di[x] < m) {
        p = res[x];
        m = di[x];
      }
  }

  return p.Normalized() * length;
}

float GetMapFade() {
#define MAPFADELENGTH 250

  int lastMapTime = globalTimer.GetTime() - mumbleLink.lastMapChangeTime;
  if (mumbleLink.isMapOpen && lastMapTime > MAPFADELENGTH) return 0.0f;

  float mapFade = 1.0f;

  if (mumbleLink.isMapOpen) {
    lastMapTime = MAPFADELENGTH - lastMapTime;
    mapFade = std::min(1.0f, lastMapTime / float(MAPFADELENGTH));
  }

  return mapFade;
}

void GW2TacticalDisplay::FetchAchievements() {
  if (GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK) return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->valid &&
      (GetTime() - lastFetchTime > 150000 || !lastFetchTime) && !beingFetched &&
      !fetchThread.joinable()) {
    beingFetched = true;
    fetchThread = std::thread([this, key]() {
      auto dungeonFrequenterStatus =
          "{\"achievements\":" + key->QueryAPI("/v2/account/achievements") +
          "}";
      Object json;
      json.parse(dungeonFrequenterStatus);

      if (json.has<Array>("achievements")) {
        auto achiData = json.get<Array>("achievements").values();

        std::unordered_map<int32_t, Achievement> incoming;

        for (auto& x : achiData) {
          if (!x->is<Object>()) continue;
          auto& data = x->get<Object>();

          if (!data.has<Boolean>("done")) continue;

          bool done = data.get<Boolean>("done");

          if (!data.has<Number>("id")) continue;

          int32_t achiId = int32_t(data.get<Number>("id"));
          incoming[achiId].done = done;

          if (!done && data.has<Array>("bits")) {
            auto& bitArray = incoming[achiId].bits;
            auto bits = data.get<Array>("bits").values();
            for (auto& bit : bits) {
              if (!bit->is<Number>()) continue;
              bitArray.push_back(int32_t(bit->get<Number>()));
            }
          } else if (done)
            incoming[achiId].bits.clear();
        }

        {
          std::lock_guard<std::mutex> lockGuard(achievements_mtx);
          achievements = incoming;
        }
      }

      beingFetched = false;
      achievementsFetched = true;
    });
  }

  if (!beingFetched && fetchThread.joinable()) {
    lastFetchTime = GetTime();
    fetchThread.join();
  }
}

void GW2TacticalDisplay::InsertPOI(POI& poi) {
  if (poi.mapID != mumbleLink.mapID) return;

  if (poi.routeMember) {
    bool discard = true;

    for (const auto& r : Routes) {
      if (r.activeItem >= 0) {
        if (r.route[r.activeItem] == poi.guid) {
          discard = false;
          break;
        }
      }
    }

    if (discard) return;
  }

  poi.cameraSpacePosition =
      CVector4(poi.position.x, poi.position.y + poi.typeData.height,
               poi.position.z, 1.0f) *
      cam;

  minimapPOIs.push_back(&poi);

  if (poi.typeData.fadeFar >= 0 && poi.typeData.fadeNear >= 0) {
    float dist = WorldToGameCoords(poi.cameraSpacePosition.Length());
    if (dist > poi.typeData.fadeFar) return;
  }

  mapPOIs.push_back(&poi);
}

void GW2TacticalDisplay::DrawPOI(CWBDrawAPI* API, const tm& ptm,
                                 const time_t& currtime, POI& poi,
                                 bool drawDistance, std::string& infoText) {
  bool drawCountdown = false;
  int32_t timeLeft;
  float alphaMultiplier = 1;

  if (!poi.IsVisible(ptm, currtime, achievementsFetched, achievements,
                     achievements_mtx))
    return;

  if (poi.typeData.behavior == POIBehavior::WvWObjective) {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if (elapsedtime < 300) {
      timeLeft = static_cast<int32_t>(300 - elapsedtime);
      drawCountdown = true;
    }
  }

  if (poi.typeData.behavior == POIBehavior::ReappearAfterTimer) {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if (elapsedtime < poi.typeData.resetLength) {
      if (poi.typeData.bits.hasCountdown) {
        timeLeft = static_cast<int32_t>(poi.typeData.resetLength - elapsedtime);
        drawCountdown = true;
      } else
        return;
    }

    float dist = (poi.position - mumbleLink.charPosition).Length();

    if (!drawCountdown &&
        (poi.typeData.bits.autoTrigger || poi.typeData.bits.hasCountdown) &&
        (dist <= poi.typeData.triggerRange)) {
      // auto trigger
      POIActivationData d;
      time(&d.lastUpdateTime);
      poi.lastUpdateTime = d.lastUpdateTime;
      d.poiguid = poi.guid;

      int data = 0;
      if (poi.typeData.behavior == POIBehavior::OncePerInstance)
        data = mumbleLink.mapInstance;
      if (poi.typeData.behavior == POIBehavior::DailyPerChar)
        data = mumbleLink.charIDHash;
      if (poi.typeData.behavior == POIBehavior::OncePerInstancePerChar)
        data = mumbleLink.charIDHash ^ mumbleLink.mapInstance;

      ActivationData[POIActivationDataKey(poi.guid, data)] = d;
    }
  }

  if (poi.typeData.info >= 0) {
    if ((poi.position - mumbleLink.charPosition).Length() <=
        poi.typeData.infoRange) {
      infoText += GetStringFromMap(poi.typeData.info) + "\n";
    }
  }

  if (poi.routeMember && ((poi.position - mumbleLink.charPosition).Length() <=
                          poi.typeData.triggerRange)) {
    for (auto& r : Routes) {
      if (r.activeItem < 0) return;

      if (r.route[r.activeItem] == poi.guid) {
        // progress route

        if (r.backwards)
          r.activeItem -= 1;
        else
          r.activeItem += 1;

        r.activeItem = (r.activeItem + r.route.size()) % r.route.size();
      }
    }
  }

  // get alpha for map

  float mapFade = GetMapFade();

  if (!poi.icon) {
    poi.icon = GetMapIcon(
        App, GetStringFromMap(poi.iconFile), GetStringFromMap(poi.zipFile),
        poi.category ? GetStringFromMap(poi.category->zipFile) : "");
  }

  WBATLASHANDLE icon = poi.icon;
  float size = poi.typeData.size;
  float Alpha = poi.typeData.alpha;

  auto camspace = poi.cameraSpacePosition;
  auto screenpos = camspace;

  CVector4 camspacex = camspace + CVector4(0.5f, 0, 0, 0) * size;

  if (TacticalIconsOnEdge) {
    screenpos /= screenpos.w;
    CVector3 projpos =
        ProjectTacticalPos(CVector3(screenpos), mumbleLink.fov, asp);
    screenpos.x = projpos.x;
    screenpos.y = projpos.y;
    screenpos.z = projpos.z;
    // screenpos.Normalize();
    // screenpos *= camspace.Length();
    camspace = screenpos;
    camspacex = camspace + CVector4(0.5f, 0, 0, 0) * size;
  }

  if (!TacticalIconsOnEdge && camspace.z <= 0) return;

  float dist = WorldToGameCoords(camspace.Length());
  if (poi.typeData.fadeNear >= 0 && poi.typeData.fadeFar >= 0) {
    float fadeAlpha = 1;

    if (dist > poi.typeData.fadeFar) return;
    if (dist > poi.typeData.fadeNear)
      fadeAlpha = 1 - (dist - poi.typeData.fadeNear) /
                          (poi.typeData.fadeFar - poi.typeData.fadeNear);

    Alpha *= fadeAlpha;
  }

  camspace = camspace * persp;
  camspace /= camspace.w;

  screenpos = screenpos * persp;
  screenpos /= screenpos.w;

  camspacex = camspacex * persp;
  camspacex /= camspacex.w;

  int s = static_cast<int>(std::min<float>(
      poi.typeData.maxSize,
      std::max<float>(poi.typeData.minSize,
                      std::abs((camspacex - camspace).x) * drawrect.Width())));

  if (poi.typeData.behavior == POIBehavior::WvWObjective) {
    alphaMultiplier = std::max(
        0.f, std::min(1.f, std::pow(CVector2(screenpos.x, screenpos.y).Length(),
                                    2.f) +
                               0.3f));
  }

  screenpos = screenpos * 0.5 + CVector4(0.5, 0.5, 0.5, 0.0);

  CPoint p = CPoint(static_cast<int>(screenpos.x * drawrect.Width()),
                    static_cast<int>((1 - screenpos.y) * drawrect.Height()));

  CRect rect = CRect(p - CPoint(s, s), p + CPoint(s, s));

  if (TacticalIconsOnEdge) {
    int32_t edge = poi.typeData.minSize;

    CPoint cp = rect.Center();
    if (cp.x < edge) rect = rect + CPoint(edge - cp.x, 0);
    if (cp.x > drawrect.x2 - edge)
      rect = rect - CPoint(drawrect.x2 - cp.x + edge, 0);

    if (cp.y < edge) rect = rect + CPoint(0, edge - cp.y);
    if (cp.y > drawrect.y2 - edge)
      rect = rect - CPoint(0, drawrect.y2 - cp.y + edge);
  }

  if (!drawCountdown || poi.typeData.behavior == POIBehavior::WvWObjective) {
    CColor col = poi.typeData.color;
    if (icon != DefaultIconHandle)
      col.A() =
          uint8_t(col.A() * Alpha * alphaMultiplier * mapFade * globalOpacity);
    else
      col.A() = uint8_t(col.A() * mapFade * globalOpacity);
    API->DrawAtlasElement(icon, rect, false, false, true, true, col);
  }

  if (drawWvWNames && poi.typeData.behavior == POIBehavior::WvWObjective) {
    CWBFont* f = App->GetDefaultFont();
    extern std::vector<WvWObjective> wvwObjectives;
    std::string wvwObjectiveName;

    if (poi.wvwObjectiveID < wvwObjectives.size())
      wvwObjectiveName =
          DICT(wvwObjectives[poi.wvwObjectiveID].nameToken.c_str(),
               wvwObjectives[poi.wvwObjectiveID].name.c_str());

    if (!wvwObjectiveName.empty()) {
      p = f->GetTextPosition(wvwObjectiveName, rect,
                             WBTEXTALIGNMENTX::WBTA_CENTERX,
                             WBTEXTALIGNMENTY::WBTA_TOP,
                             WBTEXTTRANSFORM::WBTT_UPPERCASE, false) -
          CPoint(0, f->GetLineHeight());
      for (int32_t x = 0; x < 3; x++)
        for (int32_t y = 0; y < 3; y++)
          f->Write(API, wvwObjectiveName, p + CPoint(x - 1, y - 1),
                   CColor(0, 0, 0,
                          uint8_t(255 * alphaMultiplier * globalOpacity *
                                  mapFade / 2.0f)),
                   WBTEXTTRANSFORM::WBTT_UPPERCASE, false);
      f->Write(API, wvwObjectiveName, p,
               CColor(255, 255, 0,
                      uint8_t(255 * alphaMultiplier * mapFade * globalOpacity)),
               WBTEXTTRANSFORM::WBTT_UPPERCASE, false);
    }
  }

  if (drawCountdown) {
    CWBFont* f = GetFont(GetState());
    if (!f) return;

    if (poi.typeData.behavior == POIBehavior::WvWObjective)
      f = App->GetDefaultFont();

    std::string txt;
    int32_t seconds = timeLeft % 60;
    int32_t minutes = (timeLeft - seconds) / 60;
    int32_t hours = (timeLeft - seconds - minutes * 60) / 60;

    if (hours) txt += std::format("{:02d}:", hours);

    if (minutes) txt += std::format("{:02d}:", minutes);

    txt += std::format("{:02d}", seconds);

    int32_t offset = 0;
    if (drawDistance) offset += f->GetLineHeight();

    CPoint p;
    if (poi.typeData.behavior == POIBehavior::WvWObjective) {
      if (forbiddenIconHandle != -1) {
        CColor col(0xffffffff);
        if (icon != DefaultIconHandle)
          col.A() = uint8_t(col.A() * Alpha * alphaMultiplier * mapFade *
                            globalOpacity);
        else
          col.A() = uint8_t(col.A() * mapFade * globalOpacity);
        API->DrawAtlasElement(forbiddenIconHandle, rect, false, false, true,
                              true, col);
      }

      p = f->GetTextPosition(txt, rect, WBTEXTALIGNMENTX::WBTA_CENTERX,
                             WBTEXTALIGNMENTY::WBTA_BOTTOM,
                             WBTEXTTRANSFORM::WBTT_NONE, false) +
          CPoint(0, f->GetLineHeight() + offset);
      for (int32_t x = 0; x < 3; x++)
        for (int32_t y = 0; y < 3; y++)
          f->Write(API, txt, p + CPoint(x - 1, y - 1),
                   CColor(0, 0, 0,
                          uint8_t(255 * alphaMultiplier * globalOpacity *
                                  mapFade / 2.0f)),
                   WBTEXTTRANSFORM::WBTT_NONE, false);
      f->Write(API, txt, p,
               CColor(255, 255, 0,
                      uint8_t(255 * alphaMultiplier * mapFade * globalOpacity)),
               WBTEXTTRANSFORM::WBTT_NONE, false);
    } else {
      p = f->GetTextPosition(txt, rect, WBTEXTALIGNMENTX::WBTA_CENTERX,
                             WBTEXTALIGNMENTY::WBTA_CENTERY,
                             WBTEXTTRANSFORM::WBTT_NONE, false);
      p.y += offset;
      f->Write(API, txt, p,
               CColor(255, 255, 0, uint8_t(255 * mapFade * globalOpacity)),
               WBTEXTTRANSFORM::WBTT_NONE, false);
    }
  }

  if (drawDistance) {
    CWBFont* f = App->GetDefaultFont();
    if (!f) return;

    if (Alpha * alphaMultiplier > 0) {
      float charDist =
          WorldToGameCoords((poi.position - mumbleLink.charPosition).Length());

      std::string txt;

      if (!useMetricDisplay)
        txt = std::format("{:d}", static_cast<int32_t>(charDist));
      else {
        charDist *= 0.0254f;
        txt = std::format("{:.1f}m", charDist);
      }

      p = f->GetTextPosition(txt, rect, WBTEXTALIGNMENTX::WBTA_CENTERX,
                             WBTEXTALIGNMENTY::WBTA_BOTTOM,
                             WBTEXTTRANSFORM::WBTT_NONE, false) +
          CPoint(0, f->GetLineHeight());

      for (int32_t x = 0; x < 3; x++)
        for (int32_t y = 0; y < 3; y++)
          f->Write(API, txt, p + CPoint(x - 1, y - 1),
                   CColor(0, 0, 0,
                          uint8_t(255 * Alpha * alphaMultiplier *
                                  globalOpacity * mapFade / 2.0f)),
                   WBTEXTTRANSFORM::WBTT_NONE, false);
      f->Write(API, txt, p,
               CColor(255, 255, 255,
                      uint8_t(255 * Alpha * alphaMultiplier * mapFade *
                              globalOpacity)),
               WBTEXTTRANSFORM::WBTT_NONE, false);
    }
  }
}

float uiScale = 1.0f;

void GW2TacticalDisplay::DrawPOIMinimap(CWBDrawAPI* API, const CRect& miniRect,
                                        CVector2 pos, const tm& ptm,
                                        const time_t& currtime, POI& poi,
                                        float alpha, float zoomLevel) {
  if (alpha <= 0) return;
  if (!poi.IsVisible(ptm, currtime, achievementsFetched, achievements,
                     achievements_mtx))
    return;

  if (!poi.typeData.bits.keepOnMapEdge &&
      !miniRect.Contains(CPoint(int32_t(pos.x), int32_t(pos.y))))
    return;

  if (poi.typeData.bits.keepOnMapEdge) {
    pos.x = std::min(static_cast<float>(miniRect.x2),
                     std::max(static_cast<float>(miniRect.x1), pos.x));
    pos.y = std::min(static_cast<float>(miniRect.y2),
                     std::max(static_cast<float>(miniRect.y1), pos.y));
  }

  if (!poi.icon) {
    poi.icon = GetMapIcon(
        App, GetStringFromMap(poi.iconFile), GetStringFromMap(poi.zipFile),
        poi.category ? GetStringFromMap(poi.category->zipFile) : "");
  }

  float poiSize = float(poi.typeData.miniMapSize);
  if (poi.typeData.bits.scaleWithZoom) poiSize /= zoomLevel;
  poiSize *= uiScale;

  alpha *=
      1.0f -
      std::max(0.0f,
               std::min(1.0f,
                        (zoomLevel - poi.typeData.miniMapFadeOutLevel) / 2.0f));

  CVector2 startPoint = pos - CVector2(poiSize / 2.0f, poiSize / 2.0f);
  CPoint topLeft = CPoint(int32_t(startPoint.x), int32_t(startPoint.y));

  CRect displayRect(topLeft, topLeft);
  displayRect.x2 = topLeft.x + int32_t(poiSize);
  displayRect.y2 = topLeft.y + int32_t(poiSize);

  CColor col = poi.typeData.color;
  col.A() = uint8_t(col.A() * alpha * minimapOpacity * poi.typeData.alpha);

  API->DrawAtlasElement(poi.icon, displayRect, false, false, true, true, col);
}

void GW2TacticalDisplay::OnDraw(CWBDrawAPI* API) {
  // default values
  if (!HasConfigValue("TacticalIconsOnEdge"))
    SetConfigValue("TacticalIconsOnEdge", 1);

  if (!HasConfigValue("TacticalLayerVisible"))
    SetConfigValue("TacticalLayerVisible", 1);

  if (!HasConfigValue("DrawWvWNames")) SetConfigValue("DrawWvWNames", 1);

  if (!HasConfigValue("TacticalDrawDistance"))
    SetConfigValue("TacticalDrawDistance", 0);

  if (!HasConfigValue("UseMetricDisplay"))
    SetConfigValue("UseMetricDisplay", 0);

  if (!HasConfigValue("OpacityIngame")) SetConfigValue("OpacityIngame", 0);

  if (!HasConfigValue("OpacityMap")) SetConfigValue("OpacityMap", 0);

  if (!HasConfigValue("TacticalInfoTextVisible"))
    SetConfigValue("TacticalInfoTextVisible", 1);

  int opac = GetConfigValue("OpacityIngame");
  if (opac == 0) globalOpacity = 1.0f;
  if (opac == 1) globalOpacity = 2 / 3.0f;
  if (opac == 2) globalOpacity = 1 / 3.0f;

  opac = GetConfigValue("OpacityMap");
  if (opac == 0) minimapOpacity = 1.0f;
  if (opac == 1) minimapOpacity = 2 / 3.0f;
  if (opac == 2) minimapOpacity = 1 / 3.0f;

  useMetricDisplay = GetConfigValue("UseMetricDisplay");

  if (!GetConfigValue("TacticalLayerVisible")) return;

  if (!mumbleLink.IsValid()) return;

  uiScale = GetUIScale();

  if (!HasConfigValue("ShowMinimapMarkers"))
    SetConfigValue("ShowMinimapMarkers", 1);
  int showMinimapMarkers = GetConfigValue("ShowMinimapMarkers");

  if (!HasConfigValue("ShowBigmapMarkers"))
    SetConfigValue("ShowBigmapMarkers", 1);
  int showBigmapMarkers = GetConfigValue("ShowBigmapMarkers");

  if (!HasConfigValue("ShowInGameMarkers"))
    SetConfigValue("ShowInGameMarkers", 1);
  int showIngameMarkers = GetConfigValue("ShowInGameMarkers");

  FetchAchievements();
  UpdateWvWStatus();

  TacticalIconsOnEdge = GetConfigValue("TacticalIconsOnEdge");
  drawWvWNames = GetConfigValue("DrawWvWNames") != 0;
  bool drawDistance = GetConfigValue("TacticalDrawDistance") != 0;

  time_t rawtime;
  time(&rawtime);
  struct tm ptm;
  gmtime_s(&ptm, &rawtime);

  mapPOIs.clear();
  minimapPOIs.clear();

  drawrect = GetClientRect();

  cam.SetLookAtLH(mumbleLink.camPosition,
                  mumbleLink.camPosition + mumbleLink.camDir,
                  CVector3(0, 1, 0));
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, drawrect.Width() / static_cast<float>(drawrect.Height()),
      0.01f, 1000.0f);

  asp = drawrect.Width() / static_cast<float>(drawrect.Height());

  for (auto& poi : POIs) {
    InsertPOI(poi.second);
  }

  extern bool wvwCanBeRendered;

  if (wvwCanBeRendered) {
    for (auto& e : wvwPOIs) {
      InsertPOI(e.second);
    }
  }

  time_t currtime;
  time(&currtime);

  std::sort(mapPOIs.begin(), mapPOIs.end(), [](POI* a, POI* b) {
    return b->cameraSpacePosition.z > a->cameraSpacePosition.z;
  });

  for (auto& r : Routes) {
    if (r.hasResetPos && r.MapID == mumbleLink.mapID &&
        (r.resetPos - mumbleLink.charPosition).Length() < r.resetRad)
      r.activeItem = 0;
  }

  std::string infoText;

  if (showIngameMarkers > 0)
    for (const auto& mp : mapPOIs) {
      if (!mp->typeData.bits.inGameVisible && showIngameMarkers != 2) continue;
      DrawPOI(API, ptm, currtime, *mp, drawDistance, infoText);
    }

  // punch hole in minimap

  CRect miniRect = GetMinimapRectangle();

  API->FlushDrawBuffer();
  API->GetDevice()->SetRenderState(
      (dynamic_cast<COverlayApp*>(App))->holePunchBlendState.get());

  API->DrawRect(miniRect, CColor(0, 0, 0, 0));

  API->FlushDrawBuffer();
  API->SetUIRenderState();

  // draw minimap trails

  GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
      App->GetRoot()->FindChildByID("trail", "gw2Trails"));
  if (trails) trails->DrawProxy(API, true);

  // draw minimap

  float mapFade = GetMapFade();

  if (mapFade > 0 && showMinimapMarkers > 0) {
    CMatrix4x4 miniMapTrafo =
        mumbleLink.miniMap.BuildTransformationMatrix(miniRect, false);
    for (const auto& mmp : minimapPOIs) {
      if (!mmp->typeData.bits.miniMapVisible && showMinimapMarkers != 2)
        continue;
      if (!mmp->IsVisible(ptm, currtime, achievementsFetched, achievements,
                          achievements_mtx))
        continue;

      CVector3 poiPos(mmp->position * miniMapTrafo);
      DrawPOIMinimap(API, miniRect, CVector2(poiPos.x, poiPos.y), ptm, currtime,
                     *mmp, mapFade, mumbleLink.miniMap.mapScale);
    }
  }

  if (mumbleLink.isMapOpen && mapFade < 1.0 && showBigmapMarkers > 0) {
    miniRect = GetClientRect();
    CMatrix4x4 miniMapTrafo =
        mumbleLink.bigMap.BuildTransformationMatrix(miniRect, true);
    for (const auto& mmp : minimapPOIs) {
      if (!mmp->typeData.bits.bigMapVisible && showBigmapMarkers != 2) continue;
      if (!mmp->IsVisible(ptm, currtime, achievementsFetched, achievements,
                          achievements_mtx))
        continue;

      CVector3 poiPos(mmp->position * miniMapTrafo);
      DrawPOIMinimap(API, miniRect, CVector2(poiPos.x, poiPos.y), ptm, currtime,
                     *mmp, 1.0f - mapFade, mumbleLink.bigMap.mapScale);
    }
  }

  if (GetConfigValue("TacticalInfoTextVisible")) {
    auto font = GetApplication()->GetRoot()->GetFont(GetState());
    int32_t width = font->GetWidth(infoText.c_str());
    font->Write(API, infoText.c_str(),
                CPoint(int((GetClientRect().Width() - width) / 2.0f),
                       int(GetClientRect().Height() * 0.15f)));
  }
}

GW2TacticalDisplay::GW2TacticalDisplay(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

GW2TacticalDisplay::~GW2TacticalDisplay() = default;

CWBItem* GW2TacticalDisplay::Factory(CWBItem* Root, const CXMLNode& node,
                                     CRect& Pos) {
  return GW2TacticalDisplay::Create(Root, Pos).get();
}

bool GW2TacticalDisplay::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                            WBMESSAGE MessageType) {
  return true;
}

void GW2TacticalDisplay::RemoveUserMarkersFromMap() {
  if (!mumbleLink.IsValid()) return;

  for (auto& poi : POIs) {
    if (poi.second.mapID == mumbleLink.mapID && !poi.second.External) {
      POIs.erase(poi.first);
    }
  }

  ExportPOIS();
}

bool FindSavedCategory(GW2TacticalCategory* t) {
  if (t->KeepSaveState) return true;
  for (const auto& c : t->children)
    if (FindSavedCategory(c.get())) return true;
  return false;
}

void ExportSavedCategories(CXMLNode* n, GW2TacticalCategory* t) {
  if (!FindSavedCategory(t)) return;
  auto& nn = n->AddChild("MarkerCategory");
  nn.SetAttribute("name", t->name);
  if (t->name != t->displayName) nn.SetAttribute("DisplayName", t->displayName);
  t->data.Write(&nn);
  for (const auto& c : t->children) ExportSavedCategories(&nn, c.get());
}

void ExportPOI(CXMLNode* n, POI& p) {
  CXMLNode* t = &n->AddChild("POI");
  t->SetAttributeFromInteger("MapID", p.mapID);
  t->SetAttributeFromFloat("xpos", p.position.x);
  t->SetAttributeFromFloat("ypos", p.position.y);
  t->SetAttributeFromFloat("zpos", p.position.z);
  if (!p.Type.empty()) t->SetAttribute("type", p.Type);
  t->SetAttribute(
      "GUID", B64Encode(std::string_view(reinterpret_cast<const char*>(&p.guid),
                                         sizeof(GUID))));
  p.typeData.Write(t);
}

void ExportTrail(CXMLNode* n, GW2Trail& p) {
  CXMLNode* t = &n->AddChild("Trail");
  if (!p.Type.empty()) t->SetAttribute("type", p.Type);
  t->SetAttribute(
      "GUID", B64Encode(std::string_view(reinterpret_cast<const char*>(&p.guid),
                                         sizeof(GUID))));
  p.typeData.Write(t);
}

void ExportPOIS() {
  CXMLDocument d;
  CXMLNode root = d.GetDocumentNode();
  CXMLNode& overlayData = root.AddChild("OverlayData");

  for (const auto& c : CategoryRoot.children)
    ExportSavedCategories(&overlayData, c.get());

  CXMLNode* n = &overlayData.AddChild("POIs");

  for (auto& poi : POIs) {
    if (!poi.second.External && !poi.second.routeMember)
      ExportPOI(n, poi.second);
  }

  for (auto& x : trails) {
    auto& p = x.second;
    if (!p->External) ExportTrail(n, *p);
  }

  for (const auto& r : Routes) {
    if (r.external) continue;

    CXMLNode* t = &n->AddChild("Route");
    t->SetAttribute("Name", r.name);
    t->SetAttributeFromInteger("BackwardDirection",
                               static_cast<int32_t>(r.backwards));

    for (const auto& ar : r.route) {
      if (POIs.find(ar) != POIs.end()) ExportPOI(t, POIs[ar]);
    }
  }

  d.SaveToFile("poidata.xml");
}

GUID LoadGUID(CXMLNode& n) {
  auto guidb64 = n.GetAttributeAsString("GUID");

  auto data = B64Decode(guidb64);

  GUID guid;

  if (data.size() == sizeof(GUID))
    memcpy(&guid, data.c_str(), sizeof(GUID));
  else
    CoCreateGuid(&guid);

  return guid;
}

void RecursiveImportPOIType(const CXMLNode& root, GW2TacticalCategory* Root,
                            std::string_view currentCategory,
                            MarkerTypeData& defaults, bool KeepSaveState,
                            std::string_view zipFile) {
  for (int32_t x = 0; x < root.GetChildCount("MarkerCategory"); x++) {
    const auto& n = root.GetChild("MarkerCategory", x);
    if (!n.HasAttribute("name")) continue;

    auto name = n.GetAttribute("name");

    for (int32_t x = 0; x < (int32_t)name.size(); x++)
      if (!isalnum(name[x]) && name[x] != '.') name[x] = '_';

    std::string displayName;
    std::string newCatName(currentCategory);

    auto nameExploded = Split(name, ".");

    GW2TacticalCategory* c = nullptr;

    for (size_t y = 0; y < nameExploded.size(); y++) {
      GW2TacticalCategory* Root2 = Root;

      if (!newCatName.empty()) newCatName += ".";
      newCatName += nameExploded[y];
      std::transform(newCatName.begin(), newCatName.end(), newCatName.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      c = GetCategory(newCatName);

      if (!c) {
        auto nc = std::make_unique<GW2TacticalCategory>();
        c = nc.get();
        Root2->children.emplace_back(std::move(nc));
        c->name = nameExploded[y];
        c->data = defaults;
        CategoryMap[newCatName] = c;
        c->Parent = Root2;
        Root2 = c;
        displayName = c->name;

        std::transform(c->name.begin(), c->name.end(), c->name.begin(),
                       [](unsigned char c) { return std::tolower(c); });
      }
    }

    if (!c) continue;

    if (c->displayName.empty()) c->displayName = displayName;

    if (n.HasAttribute("DisplayName")) {
      displayName = n.GetAttribute("DisplayName");
      c->displayName = displayName;
      localization->ProcessStringForUsedGlyphs(displayName);
    }

    if (n.HasAttribute("IsSeparator")) {
      int separator = 0;
      n.GetAttributeAsInteger("IsSeparator", &separator);
      c->IsOnlySeparator = separator;
    }

    c->data.Read(n, KeepSaveState);
    c->zipFile = AddStringToMap(zipFile);
    c->KeepSaveState = KeepSaveState;

    RecursiveImportPOIType(n, c, newCatName, c->data, KeepSaveState, zipFile);
  }
}

void ImportPOITypes() {
  CXMLDocument d;
  if (!d.LoadFromFile("categorydata.xml")) return;

  if (!d.GetDocumentNode().GetChildCount("OverlayData")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("OverlayData");

  CategoryMap.clear();
  CategoryRoot.children.clear();
  auto defaults = MarkerTypeData();
  RecursiveImportPOIType(root, &CategoryRoot, "", defaults, false, "");
}

void ImportPOI(CWBApplication* App, CXMLNode& t, POI& p,
               std::string_view zipFile) {
  if (t.HasAttribute("MapID")) t.GetAttributeAsInteger("MapID", &p.mapID);
  if (t.HasAttribute("xpos")) t.GetAttributeAsFloat("xpos", &p.position.x);
  if (t.HasAttribute("ypos")) t.GetAttributeAsFloat("ypos", &p.position.y);
  if (t.HasAttribute("zpos")) t.GetAttributeAsFloat("zpos", &p.position.z);
  if (t.HasAttribute("icon")) t.GetAttributeAsInteger("icon", &p.icon);
  if (t.HasAttribute("type")) p.Type = t.GetAttributeAsString("type");

  if (!t.HasAttribute("GUID"))
    CoCreateGuid(&p.guid);
  else
    p.guid = LoadGUID(t);

  p.zipFile = AddStringToMap(zipFile);

  auto* td = GetCategory(p.Type);
  if (td) p.SetCategory(App, td);

  p.typeData.Read(t, true);

  p.iconFile = p.typeData.iconFile;
}

bool ImportTrail(CWBApplication* App, CXMLNode& t, GW2Trail& p,
                 std::string_view zipFile) {
  p.zipFile = zipFile;

  if (t.HasAttribute("type")) p.Type = t.GetAttributeAsString("type");

  if (!t.HasAttribute("GUID"))
    CoCreateGuid(&p.guid);
  else
    p.guid = LoadGUID(t);

  auto* td = GetCategory(p.Type);
  if (td) p.SetCategory(App, td);

  p.typeData.Read(t, true);

  return p.Import(GetStringFromMap(p.typeData.trailData), zipFile);
}

void ImportPOIDocument(CWBApplication* App, CXMLDocument& d, bool External,
                       std::string_view zipFile) {
  if (!d.GetDocumentNode().GetChildCount("OverlayData")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("OverlayData");

  auto defaults = MarkerTypeData();
  RecursiveImportPOIType(root, &CategoryRoot, "", defaults, !External, zipFile);

  if (root.GetChildCount("POIs")) {
    CXMLNode n = root.GetChild("POIs");

    if (n.GetChildCount("POI") > 0) {
      CXMLNode t = n.GetChild("POI", 0);
      do {
        POI p;
        ImportPOI(App, t, p, zipFile);
        p.External = External;
        POIs[p.guid] = p;
      } while (t.Next(t, "POI"));
    }

    for (int32_t x = 0; x < n.GetChildCount("Route"); x++) {
      CXMLNode rn = n.GetChild("Route", x);
      POIRoute r;
      if (rn.HasAttribute("Name")) r.name = rn.GetAttributeAsString("Name");
      int32_t b = false;
      if (rn.HasAttribute("BackwardDirection"))
        rn.GetAttributeAsInteger("BackwardDirection", &b);
      if (rn.HasAttribute("MapID")) rn.GetAttributeAsInteger("MapID", &r.MapID);
      r.backwards = b;
      r.external = External;
      if (rn.HasAttribute("resetposx") && rn.HasAttribute("resetposy") &&
          rn.HasAttribute("resetposz") && rn.HasAttribute("resetrange")) {
        rn.GetAttributeAsFloat("resetposx", &r.resetPos.x);
        rn.GetAttributeAsFloat("resetposy", &r.resetPos.y);
        rn.GetAttributeAsFloat("resetposz", &r.resetPos.z);
        rn.GetAttributeAsFloat("resetrange", &r.resetRad);
        r.hasResetPos = true;
      }

      if (rn.GetChildCount("POI") > 0) {
        CXMLNode t = rn.GetChild("POI", 0);
        do {
          POI p;
          ImportPOI(App, t, p, zipFile);
          p.External = External;
          p.routeMember = true;
          POIs[p.guid] = p;
          r.route.push_back(p.guid);
        } while (t.Next(t, "POI"));
      }

      Routes.push_back(r);
    }

    if (n.GetChildCount("Trail") > 0) {
      CXMLNode t = n.GetChild("Trail", 0);
      do {
        auto p = std::make_unique<GW2Trail>();
        if (ImportTrail(App, t, *p, zipFile)) {
          p->External = External;
          trails[p->guid] = std::move(p);
        }
      } while (t.Next(t, "Trail"));
    }
  }
}

void ImportPOIFile(CWBApplication* App, std::string_view s, bool External) {
  CXMLDocument d;
  if (!d.LoadFromFile(s.data())) return;
  ImportPOIDocument(App, d, External, "");
}

void ImportPOIString(CWBApplication* App, std::string_view data,
                     std::string_view zipFile) {
  CXMLDocument d;
  if (!d.LoadFromString(data)) return;
  ImportPOIDocument(App, d, true, zipFile);
}

void ImportMarkerPack(CWBApplication* App, std::string_view zipFile) {
  mz_zip_archive* zip = OpenZipFile(zipFile.data());
  if (!zip) return;

  for (uint32_t x = 0; x < mz_zip_reader_get_num_files(zip); x++) {
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(zip, x, &stat)) continue;

    if (mz_zip_reader_is_file_a_directory(zip, x)) continue;

    if (stat.m_uncomp_size <= 0) continue;

    std::string fileName(stat.m_filename);
    std::transform(fileName.begin(), fileName.end(), fileName.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (fileName.find(".xml") != fileName.size() - 4) continue;

    auto data =
        std::make_unique<uint8_t[]>(static_cast<int32_t>(stat.m_uncomp_size));

    if (!mz_zip_reader_extract_to_mem(
            zip, x, data.get(), static_cast<int32_t>(stat.m_uncomp_size), 0)) {
      continue;
    }

    std::string_view doc(reinterpret_cast<char*>(data.get()),
                         static_cast<uint32_t>(stat.m_uncomp_size));
    ImportPOIString(App, doc, zipFile);
  }
}

void ImportPOIS(CWBApplication* App) {
  FlushZipDict();
  ImportPOITypes();

  POIs.clear();
  Routes.clear();
  trails.clear();

  {
    CFileList list;
    list.ExpandSearch("*.xml", "POIs", false);
    for (auto& File : list.Files)
      ImportPOIFile(App, File.Path + File.FileName, true);
  }

  {
    CFileList list;
    list.ExpandSearch("*.zip", "POIs", false);
    for (auto& File : list.Files)
      ImportMarkerPack(App, File.Path + File.FileName);
  }

  {
    CFileList list;
    list.ExpandSearch("*.taco", "POIs", false);
    for (auto& File : list.Files)
      ImportMarkerPack(App, File.Path + File.FileName);
  }

  ImportPOIFile(App, "poidata.xml", false);

  LoadMarkerCategoryVisibilityInfo();
}

void ImportPOIActivationData() {
  CXMLDocument d;
  if (!d.LoadFromFile("activationdata.xml")) return;

  if (!d.GetDocumentNode().GetChildCount("OverlayData")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("OverlayData");

  if (root.GetChildCount("Activations")) {
    CXMLNode n = root.GetChild("Activations");

    for (int32_t x = 0; x < n.GetChildCount("POIActivation"); x++) {
      CXMLNode t = n.GetChild("POIActivation", x);

      if (!t.HasAttribute("GUID")) continue;

      POIActivationData p;

      if (t.HasAttribute("lut1"))
        t.GetAttributeAsInteger(
            "lut1", &(reinterpret_cast<int32_t*>(&p.lastUpdateTime))[0]);
      if (t.HasAttribute("lut2"))
        t.GetAttributeAsInteger(
            "lut2", &(reinterpret_cast<int32_t*>(&p.lastUpdateTime))[1]);

      p.poiguid = LoadGUID(t);

      p.uniqueData = 0;
      if (t.HasAttribute("instance"))
        t.GetAttributeAsInteger("instance", &p.uniqueData);

      ActivationData[POIActivationDataKey(p.poiguid, p.uniqueData)] = p;

      if (POIs.find(p.poiguid) != POIs.end())
        POIs[p.poiguid].lastUpdateTime = p.lastUpdateTime;
    }
  }
}

void ExportPOIActivationData() {
  CXMLDocument d;
  CXMLNode root = d.GetDocumentNode();
  auto& overlayData = root.AddChild("OverlayData");

  CXMLNode* n = &overlayData.AddChild("Activations");

  for (auto& ad : ActivationData) {
    auto& dat = ad.second;
    if (POIs.find(dat.poiguid) != POIs.end()) {
      auto& poi = POIs[dat.poiguid];
      if (poi.typeData.behavior == POIBehavior::AlwaysVisible) continue;
    }

    CXMLNode* t = &n->AddChild("POIActivation");
    t->SetAttributeFromInteger(
        "lut1", (reinterpret_cast<int32_t*>(&dat.lastUpdateTime))[0]);
    t->SetAttributeFromInteger(
        "lut2", (reinterpret_cast<int32_t*>(&dat.lastUpdateTime))[1]);
    if (dat.uniqueData) t->SetAttributeFromInteger("instance", dat.uniqueData);
    t->SetAttribute("GUID", B64Encode(std::string_view(
                                reinterpret_cast<const char*>(&dat.poiguid),
                                sizeof(GUID))));
  }

  d.SaveToFile("activationdata.xml");
}

std::string DefaultMarkerCategory = "";

void AddPOI(CWBApplication* App) {
  if (!mumbleLink.IsValid()) return;
  POI poi;
  poi.position = mumbleLink.charPosition;
  poi.mapID = mumbleLink.mapID;
  poi.icon = DefaultIconHandle;

  CoCreateGuid(&poi.guid);

  auto cat = GetCategory(DefaultMarkerCategory);

  if (poi.mapID == -1) return;
  for (auto& poix : POIs) {
    if (poix.second.mapID != poi.mapID) continue;
    CVector3 v = poix.second.position - poi.position;
    if (v.Length() < poix.second.typeData.triggerRange &&
        cat == poix.second.category)
      return;
  }

  if (cat) poi.SetCategory(App, cat);

  POIs[poi.guid] = poi;
  ExportPOIS();
}

void DeletePOI() {
  if (!mumbleLink.IsValid()) return;
  POI poi;
  poi.position = CVector3(mumbleLink.charPosition);
  poi.mapID = mumbleLink.mapID;

  if (poi.mapID == -1) return;
  for (auto poix : POIs) {
    if (poix.second.mapID != poi.mapID) continue;
    CVector3 v = poix.second.position - poi.position;
    if (v.Length() < poix.second.typeData.triggerRange) {
      POIs.erase(poix.first);
      ExportPOIS();
      return;
    }
  }
}

void UpdatePOI() {
  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.mapID == -1) return;

  bool found = false;

  for (auto& poi : POIs) {
    auto& cpoi = poi.second;

    if (cpoi.mapID != mumbleLink.mapID) continue;

    CVector3 v = cpoi.position - CVector3(mumbleLink.charPosition);
    if (v.Length() < cpoi.typeData.triggerRange) {
      auto& str = GetStringFromMap(cpoi.typeData.toggleCategory);
      if (!str.empty()) {
        GW2TacticalCategory* cat = GetCategory(str);
        if (cat) {
          cat->IsDisplayed = !cat->IsDisplayed;
          SetConfigValue(("CategoryVisible_" + cat->GetFullTypeName()),
                         cat->IsDisplayed);
        }
      }

      if (!found && cpoi.typeData.behavior != POIBehavior::AlwaysVisible) {
        POIActivationData d;
        time(&d.lastUpdateTime);
        cpoi.lastUpdateTime = d.lastUpdateTime;
        d.poiguid = cpoi.guid;

        d.uniqueData = 0;
        if (cpoi.typeData.behavior == POIBehavior::OncePerInstance)
          d.uniqueData = mumbleLink.mapInstance;
        if (cpoi.typeData.behavior == POIBehavior::DailyPerChar)
          d.uniqueData = mumbleLink.charIDHash;
        if (cpoi.typeData.behavior == POIBehavior::OncePerInstancePerChar)
          d.uniqueData = mumbleLink.charIDHash ^ mumbleLink.mapInstance;

        ActivationData[POIActivationDataKey(cpoi.guid, d.uniqueData)] = d;
        ExportPOIActivationData();
        found = true;
      }
    }
  }
}

MarkerTypeData::MarkerTypeData() {
  memset(&bits, 0, sizeof(bits));
  bits.miniMapVisible = true;
  bits.bigMapVisible = true;
  bits.inGameVisible = true;
  bits.scaleWithZoom = true;
}

void MarkerTypeData::Read(const CXMLNode& n, bool StoreSaveState) {
  bool _iconFileSaved = n.HasAttribute("iconFile");
  bool _sizeSaved = n.HasAttribute("iconSize");
  bool _alphaSaved = n.HasAttribute("alpha");
  bool _fadeNearSaved = n.HasAttribute("fadeNear");
  bool _fadeFarSaved = n.HasAttribute("fadeFar");
  bool _heightSaved = n.HasAttribute("heightOffset");
  bool _behaviorSaved = n.HasAttribute("behavior");
  bool _resetLengthSaved = n.HasAttribute("resetLength");
  bool _autoTriggerSaved = n.HasAttribute("autoTrigger");
  bool _hasCountdownSaved = n.HasAttribute("hasCountdown");
  bool _triggerRangeSaved = n.HasAttribute("triggerRange");
  bool _minSizeSaved = n.HasAttribute("minSize");
  bool _maxSizeSaved = n.HasAttribute("maxSize");
  bool _colorSaved = n.HasAttribute("color");
  bool _trailDataSaved = n.HasAttribute("trailData");
  bool _animSpeedSaved = n.HasAttribute("animSpeed");
  bool _textureSaved = n.HasAttribute("texture");
  bool _trailScaleSaved = n.HasAttribute("trailScale");
  bool _toggleCategorySaved = n.HasAttribute("toggleCategory");
  bool _achievementIdSaved = n.HasAttribute("achievementId");
  bool _achievementBitSaved = n.HasAttribute("achievementBit");
  bool _miniMapVisibleSaved = n.HasAttribute("miniMapVisibility");
  bool _bigMapVisibleSaved = n.HasAttribute("mapVisibility");
  bool _inGameVisibleSaved = n.HasAttribute("inGameVisibility");
  bool _scaleWithZoomSaved = n.HasAttribute("scaleOnMapWithZoom");
  bool _miniMapSizeSaved = n.HasAttribute("mapDisplaySize");
  bool _miniMapFadeOutLevelSaved = n.HasAttribute("mapFadeoutScaleLevel");
  bool _keepOnMapEdgeSaved = n.HasAttribute("keepOnMapEdge");
  bool _infoSaved = n.HasAttribute("info");
  bool _infoRangeSaved = n.HasAttribute("infoRange");

  if (StoreSaveState) {
    bits.iconFileSaved = _iconFileSaved;
    bits.sizeSaved = _sizeSaved;
    bits.alphaSaved = _alphaSaved;
    bits.fadeNearSaved = _fadeNearSaved;
    bits.fadeFarSaved = _fadeFarSaved;
    bits.heightSaved = _heightSaved;
    bits.behaviorSaved = _behaviorSaved;
    bits.resetLengthSaved = _resetLengthSaved;
    bits.autoTriggerSaved = _autoTriggerSaved;
    bits.hasCountdownSaved = _hasCountdownSaved;
    bits.triggerRangeSaved = _triggerRangeSaved;
    bits.minSizeSaved = _minSizeSaved;
    bits.maxSizeSaved = _maxSizeSaved;
    bits.colorSaved = _colorSaved;
    bits.trailDataSaved = _trailDataSaved;
    bits.animSpeedSaved = _animSpeedSaved;
    bits.textureSaved = _textureSaved;
    bits.trailScaleSaved = _trailScaleSaved;
    bits.toggleCategorySaved = _toggleCategorySaved;
    bits.achievementIdSaved = _achievementIdSaved;
    bits.achievementBitSaved = _achievementBitSaved;
    bits.miniMapVisibleSaved = _miniMapVisibleSaved;
    bits.bigMapVisibleSaved = _bigMapVisibleSaved;
    bits.inGameVisibleSaved = _inGameVisibleSaved;
    bits.scaleWithZoomSaved = _scaleWithZoomSaved;
    bits.miniMapSizeSaved = _miniMapSizeSaved;
    bits.miniMapFadeOutLevelSaved = _miniMapFadeOutLevelSaved;
    bits.keepOnMapEdgeSaved = _keepOnMapEdgeSaved;
    bits.infoSaved = _infoSaved;
    bits.infoRangeSaved = _infoRangeSaved;
  }

  if (_iconFileSaved) iconFile = AddStringToMap(n.GetAttribute("iconFile"));

  if (_sizeSaved) n.GetAttributeAsFloat("iconSize", &size);

  if (_alphaSaved) n.GetAttributeAsFloat("alpha", &alpha);
  if (_fadeNearSaved) n.GetAttributeAsFloat("fadeNear", &fadeNear);
  if (_fadeFarSaved) n.GetAttributeAsFloat("fadeFar", &fadeFar);
  if (_heightSaved) n.GetAttributeAsFloat("heightOffset", &height);
  if (_behaviorSaved) {
    int32_t x;
    n.GetAttributeAsInteger("behavior", &x);
    behavior = static_cast<POIBehavior>(x);
  }
  if (_resetLengthSaved) {
    int32_t val;
    n.GetAttributeAsInteger("resetLength", &val);
    resetLength = val;
  }
  if (_autoTriggerSaved) {
    int32_t val;
    n.GetAttributeAsInteger("autoTrigger", &val);
    bits.autoTrigger = val != 0;
  }
  if (_hasCountdownSaved) {
    int32_t val;
    n.GetAttributeAsInteger("hasCountdown", &val);
    bits.hasCountdown = val != 0;
  }
  if (_triggerRangeSaved) n.GetAttributeAsFloat("triggerRange", &triggerRange);
  if (_minSizeSaved) {
    int32_t val;
    n.GetAttributeAsInteger("minSize", &val);
    minSize = val;
  }
  if (_maxSizeSaved) {
    int32_t val;
    n.GetAttributeAsInteger("maxSize", &val);
    maxSize = val;
  }
  if (_colorSaved) {
    auto colorStr = n.GetAttributeAsString("color");
    uint32_t colHex = 0xffffffff;
    std::sscanf(colorStr.c_str(), "%x", &colHex);
    color = CColor(colHex);
  }
  if (_trailDataSaved) trailData = AddStringToMap(n.GetAttribute("trailData"));
  if (_animSpeedSaved) n.GetAttributeAsFloat("animSpeed", &animSpeed);
  if (_textureSaved) texture = AddStringToMap(n.GetAttribute("texture"));
  if (_trailScaleSaved) n.GetAttributeAsFloat("trailScale", &trailScale);
  if (_toggleCategorySaved)
    toggleCategory = AddStringToMap(n.GetAttribute("toggleCategory"));
  if (_achievementIdSaved) {
    int32_t val;
    n.GetAttributeAsInteger("achievementId", &val);
    achievementId = val;
  }
  if (_achievementBitSaved) {
    int32_t val;
    n.GetAttributeAsInteger("achievementBit", &val);
    achievementBit = val;
  }
  if (_miniMapVisibleSaved) {
    int32_t val;
    n.GetAttributeAsInteger("miniMapVisibility", &val);
    bits.miniMapVisible = val != 0;
  }
  if (_bigMapVisibleSaved) {
    int32_t val;
    n.GetAttributeAsInteger("mapVisibility", &val);
    bits.bigMapVisible = val != 0;
  }
  if (_inGameVisibleSaved) {
    int32_t val;
    n.GetAttributeAsInteger("inGameVisibility", &val);
    bits.inGameVisible = val != 0;
  }
  if (_scaleWithZoomSaved) {
    int32_t val;
    n.GetAttributeAsInteger("scaleOnMapWithZoom", &val);
    bits.scaleWithZoom = val != 0;
  }
  if (_miniMapFadeOutLevelSaved)
    n.GetAttributeAsFloat("mapFadeoutScaleLevel", &miniMapFadeOutLevel);

  if (_miniMapSizeSaved) {
    int32_t x;
    n.GetAttributeAsInteger("mapDisplaySize", &x);
    miniMapSize = x;
  }
  if (_keepOnMapEdgeSaved) {
    int32_t val;
    n.GetAttributeAsInteger("keepOnMapEdge", &val);
    bits.keepOnMapEdge = val != 0;
  }
  if (_infoSaved) info = AddStringToMap(n.GetAttributeAsString("info"));
  if (_infoRangeSaved) n.GetAttributeAsFloat("infoRange", &infoRange);
}

void MarkerTypeData::Write(CXMLNode* n) {
  if (bits.iconFileSaved)
    n->SetAttribute("iconFile", GetStringFromMap(iconFile));
  if (bits.sizeSaved) n->SetAttributeFromFloat("iconSize", size);
  if (bits.alphaSaved) n->SetAttributeFromFloat("alpha", alpha);
  if (bits.fadeNearSaved) n->SetAttributeFromFloat("fadeNear", fadeNear);
  if (bits.fadeFarSaved) n->SetAttributeFromFloat("fadeFar", fadeFar);
  if (bits.heightSaved) n->SetAttributeFromFloat("heightOffset", height);
  if (bits.behaviorSaved)
    n->SetAttributeFromInteger("behavior", static_cast<int32_t>(behavior));
  if (bits.resetLengthSaved)
    n->SetAttributeFromInteger("resetLength", resetLength);
  if (bits.autoTriggerSaved)
    n->SetAttributeFromInteger("autoTrigger", bits.autoTrigger);
  if (bits.hasCountdownSaved)
    n->SetAttributeFromInteger("hasCountdown", bits.hasCountdown);
  if (bits.triggerRangeSaved)
    n->SetAttributeFromFloat("triggerRange", triggerRange);
  if (bits.minSizeSaved) n->SetAttributeFromInteger("minSize", minSize);
  if (bits.maxSizeSaved) n->SetAttributeFromInteger("maxSize", maxSize);
  if (bits.colorSaved)
    n->SetAttribute("color", std::format("{:x}", color.argb()));
  if (bits.trailDataSaved)
    n->SetAttribute("trailData", GetStringFromMap(trailData));
  if (bits.animSpeedSaved) n->SetAttributeFromFloat("animSpeed", animSpeed);
  if (bits.textureSaved) n->SetAttribute("texture", GetStringFromMap(texture));
  if (bits.trailScaleSaved) n->SetAttributeFromFloat("trailScale", trailScale);
  if (bits.toggleCategorySaved)
    n->SetAttribute("toggleCategory", GetStringFromMap(toggleCategory));
  if (bits.achievementIdSaved)
    n->SetAttributeFromInteger("achievementId", achievementId);
  if (bits.achievementBitSaved)
    n->SetAttributeFromInteger("achievementBit", achievementBit);
  if (bits.miniMapVisibleSaved)
    n->SetAttributeFromInteger("miniMapVisibility", bits.miniMapVisible);
  if (bits.bigMapVisibleSaved)
    n->SetAttributeFromInteger("mapVisibility", bits.bigMapVisible);
  if (bits.inGameVisibleSaved)
    n->SetAttributeFromInteger("inGameVisibility", bits.inGameVisible);
  if (bits.scaleWithZoomSaved)
    n->SetAttributeFromInteger("scaleOnMapWithZoom", bits.scaleWithZoom);
  if (bits.miniMapFadeOutLevelSaved)
    n->SetAttributeFromFloat("mapFadeoutScaleLevel", miniMapFadeOutLevel);
  if (bits.miniMapSizeSaved)
    n->SetAttributeFromInteger("mapDisplaySize", miniMapSize);
  if (bits.keepOnMapEdgeSaved)
    n->SetAttributeFromInteger("keepOnMapEdge", bits.keepOnMapEdge);
  if (bits.infoSaved) n->SetAttribute("info", GetStringFromMap(info));
  if (bits.infoRangeSaved) n->SetAttributeFromFloat("infoRange", infoRange);
}

void AddTypeContextMenu(CWBContextItem* ctx,
                        std::vector<GW2TacticalCategory*>& CategoryList,
                        const GW2TacticalCategory* Parent,
                        bool AddVisibilityMarkers, int32_t BaseID,
                        bool closeOnClick) {
  for (const auto& dta : Parent->children) {
    std::string txt;
    if (AddVisibilityMarkers)
      txt += "[" + std::string(dta->IsDisplayed ? "x" : " ") + "] ";
    if (!dta->displayName.empty())
      txt += dta->displayName;
    else
      txt += dta->name;

    if (dta->IsOnlySeparator) {
      ctx->AddSeparator();
      if (!dta->displayName.empty())
        txt = dta->displayName;
      else
        txt = dta->name;
      ctx->AddItem(txt, CategoryList.size() + BaseID, false, closeOnClick);
      CategoryList.push_back(dta.get());
      ctx->AddSeparator();
    } else {
      auto n =
          ctx->AddItem(txt, CategoryList.size() + BaseID,
                       AddVisibilityMarkers && dta->IsDisplayed, closeOnClick);
      CategoryList.push_back(dta.get());
      AddTypeContextMenu(n, CategoryList, dta.get(), AddVisibilityMarkers,
                         BaseID, closeOnClick);
    }
  }
}

void AddTypeContextMenu(CWBContextMenu* ctx,
                        std::vector<GW2TacticalCategory*>& CategoryList,
                        const GW2TacticalCategory* Parent,
                        bool AddVisibilityMarkers, int32_t BaseID,
                        bool closeOnClick) {
  for (const auto& dta : Parent->children) {
    std::string txt;
    if (AddVisibilityMarkers)
      txt += "[" + std::string(dta->IsDisplayed ? "x" : " ") + "] ";
    if (!dta->displayName.empty())
      txt += dta->displayName;
    else
      txt += dta->name;

    if (dta->IsOnlySeparator) {
      ctx->AddSeparator();
      if (!dta->displayName.empty())
        txt = dta->displayName;
      else
        txt = dta->name;
      ctx->AddItem(txt, CategoryList.size() + BaseID, false, closeOnClick);
      CategoryList.push_back(dta.get());
      ctx->AddSeparator();
    } else {
      auto n =
          ctx->AddItem(txt, CategoryList.size() + BaseID,
                       AddVisibilityMarkers && dta->IsDisplayed, closeOnClick);
      CategoryList.push_back(dta.get());
      AddTypeContextMenu(n, CategoryList, dta.get(), AddVisibilityMarkers,
                         BaseID, closeOnClick);
    }
  }
}

void OpenTypeContextMenu(CWBContextItem* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers, int32_t BaseID,
                         bool closeOnClick) {
  CategoryList.clear();
  AddTypeContextMenu(ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers,
                     BaseID, closeOnClick);
}

void OpenTypeContextMenu(CWBContextMenu* ctx,
                         std::vector<GW2TacticalCategory*>& CategoryList,
                         bool AddVisibilityMarkers, int32_t BaseID,
                         bool closeOnClick) {
  CategoryList.clear();
  AddTypeContextMenu(ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers,
                     BaseID, closeOnClick);
}

float WorldToGameCoords(float world) { return world / 0.0254f; }

float GameToWorldCoords(float game) { return game * 0.0254f; }

std::string GW2TacticalCategory::GetFullTypeName() {
  if (!cachedTypeName.empty()) return cachedTypeName;

  if (!Parent) return "";
  if (Parent == &CategoryRoot) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return n;
  }
  std::string pname = Parent->GetFullTypeName();
  std::string s = pname + "." + name;
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  cachedTypeName = s;
  return s;
}

bool GW2TacticalCategory::IsVisible() {
  if (!Parent) return true;
  return Parent->IsVisible() && IsDisplayed;
}

void POI::SetCategory(CWBApplication* App, GW2TacticalCategory* t) {
  category = t;
  typeData = t->data;
  Type = t->GetFullTypeName();
  icon = 0;
  iconFile = typeData.iconFile;
}

bool POI::IsVisible(const tm& ptm, const time_t& currtime,
                    bool achievementsFetched,
                    std::unordered_map<int32_t, Achievement>& achievements,
                    std::mutex& mtx) {
  if (category && !category->IsVisible()) return false;

  if (typeData.behavior == POIBehavior::ReappearOnDailyReset) {
    struct tm lasttime;
    gmtime_s(&lasttime, &lastUpdateTime);
    if (lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon &&
        lasttime.tm_year == ptm.tm_year)
      return false;
  }

  if (typeData.behavior == POIBehavior::ReappearAfterTimer) {
    time_t elapsedtime = currtime - lastUpdateTime;
    if (elapsedtime < typeData.resetLength) {
      if (!typeData.bits.hasCountdown) return false;
    }
  }

  if (typeData.behavior == POIBehavior::OnlyVisibleBeforeActivation) {
    if (lastUpdateTime != time_t(0)) return false;
  }

  if (typeData.behavior == POIBehavior::OncePerInstance) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.mapInstance)) != ActivationData.end())
      return false;
  }

  if (typeData.behavior == POIBehavior::OncePerInstancePerChar) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.mapInstance ^ mumbleLink.charIDHash)) !=
        ActivationData.end())
      return false;
  }

  if (typeData.behavior == POIBehavior::DailyPerChar) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.charIDHash)) != ActivationData.end()) {
      struct tm lasttime;
      gmtime_s(
          &lasttime,
          &ActivationData[POIActivationDataKey(guid, mumbleLink.charIDHash)]
               .lastUpdateTime);

      if (lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon &&
          lasttime.tm_year == ptm.tm_year)
        return false;
    }
  }

  if (routeMember && ((position - mumbleLink.charPosition).Length() <=
                      typeData.triggerRange)) {
    for (const auto& r : Routes) {
      if (r.activeItem < 0) return false;
    }
  }

  if (achievementsFetched && typeData.achievementId != -1) {
    std::lock_guard<std::mutex> lockGuard(mtx);
    if (achievements.find(typeData.achievementId) != achievements.end()) {
      if (achievements[typeData.achievementId].done) {
        return true;
      }
      if (typeData.achievementBit == -1) {
        return false;
      }
      const auto& bits = achievements[typeData.achievementId].bits;
      if (std::find(bits.begin(), bits.end(), typeData.achievementBit) !=
          bits.end()) {
        return false;
      }
    }
  }

  return true;
}
