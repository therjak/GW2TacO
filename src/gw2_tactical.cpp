#include "src/gw2_tactical.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <future>
#include <iterator>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/base/file_list.h"
#include "src/base/logger.h"
#include "src/base/stream_reader.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/util/jsonxx.h"
#include "src/util/miniz.h"
#include "src/util/png_decompressor.h"

import math;
import taco.gw2;
import taco.language;
import taco.marker_data;
import taco.mumble_link;
import taco.overlay_application;
import taco.overlay_config;
import taco.string_set;
import taco.tactical_category;
import taco.trail_logger;
import taco.wvw;
import time;
import whiteboard;
import xml;

using math::Matrix4x4;
using math::Plane;
using math::Point;
using math::Rect;
using math::Size;
using math::Vector2;
using math::Vector3;
using math::Vector4;

gui::WBATLASHANDLE DefaultIconHandle = -1;
gui::WBATLASHANDLE forbiddenIconHandle = -1;
Size forbiddenIconSize;
std::unordered_map<std::string, gui::WBATLASHANDLE> MapIcons;
int32_t useMetricDisplay = 0;

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

void FindClosestRouteMarkers(bool force) {
  const auto& pois = GetMapPOIs();
  for (auto& r : Routes) {
    if (!force && r.activeItem != -1) continue;

    if (r.map_id == mumbleLink.map_id && r.hasResetPos &&
        (r.resetPos - mumbleLink.char_position).Length() < r.resetRad) {
      r.activeItem = 0;
    }

    float closestdist = 1000000000;
    int32_t closest = -1;

    for (int32_t y = 0; y < r.route.size(); y++) {
      GUID g = r.route[y];
      const auto& fpoi = pois.find(g);
      if (fpoi != pois.end()) {
        const POI& p = fpoi->second;
        if (!(p.map_id == mumbleLink.map_id)) continue;

        float dist = (p.position - mumbleLink.char_position).Length();
        if (dist < closestdist) {
          closestdist = dist;
          closest = y;
          r.activeItem = y;
        }
      }
    }
  }
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
      Log_Err("[GW2TacO] Failed to open zip archive {:s}", zipFile);
      zipDict[zipFile] = nullptr;
    } else {
      zipDict[zipFile] = std::move(zip);
    }
  }

  return zipDict[zipFile].get();
}

gui::WBATLASHANDLE GetMapIcon(gui::CWBApplication* App, std::string_view fname,
                              std::string_view zipFile,
                              std::string_view categoryZip) {
  std::string filename(fname);
  for (char& x : filename) {
    if (x == '\\') x = '/';
  }

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
              int32_t xres = 0, yres = 0;
              if (DecompressPNG(data.get(),
                                static_cast<int32_t>(stat.m_uncomp_size),
                                imageData, xres, yres)) {
                ARGBtoABGR(imageData.get(), xres, yres);

                auto handle = App->GetAtlas()->AddImage(
                    imageData.get(), xres, yres, Rect(0, 0, xres, yres));

                MapIcons[s] = handle;

                return handle;
              } else {
                Log_Err(
                    "[GWTacO] Failed to decompress png {:s} form archive {:s}",
                    filename, x == 0 ? zipFile : categoryZip);
              }
            }
          }
        }
      }
    }

    // zipfile load failed, fall back to regular load and add it as an alias
    gui::WBATLASHANDLE handle = GetMapIcon(App, filename, "", "");
    if (handle == DefaultIconHandle) return handle;
    MapIcons[s] = handle;
    return handle;
  }

  CStreamReaderMemory f;
  if (!f.Open(s) && !f.Open("POIs\\" + s)) {
    Log_Err("[GWTacO] Failed to open image {:s}", s);
    return DefaultIconHandle;
  }

  std::unique_ptr<uint8_t[]> imageData;
  int32_t xres = 0, yres = 0;
  if (!DecompressPNG(f.GetData(), static_cast<int32_t>(f.GetLength()),
                     imageData, xres, yres)) {
    Log_Err("[GWTacO] Failed to decompress png {:s}", s);
    return DefaultIconHandle;
  }

  ARGBtoABGR(imageData.get(), xres, yres);

  auto handle = App->GetAtlas()->AddImage(imageData.get(), xres, yres,
                                          Rect(0, 0, xres, yres));

  MapIcons[s] = handle;
  return handle;
}

std::unordered_map<int, POISet> POIs;
POISet& GetMapPOIs() { return POIs[mumbleLink.map_id]; }
std::unordered_map<POIActivationDataKey, POIActivationData> ActivationData;

std::vector<POIRoute> Routes;

uint32_t DictionaryHash(const GUID& i) {
  auto* dta = (uint8_t*)(&i);
  uint32_t Hash = 5381;
  for (int x = 0; x < sizeof(GUID); x++) {
    Hash = ((Hash << 5) + Hash) + dta[x];  // hash * 33 + c
  }
  return Hash;
}

uint32_t DictionaryHash(const POIActivationDataKey& i) {
  auto* dta = (uint8_t*)(&i);
  uint32_t Hash = 5381;
  for (int x = 0; x < sizeof(POIActivationDataKey); x++) {
    Hash = ((Hash << 5) + Hash) + dta[x];  // hash * 33 + c
  }
  return Hash;
}

float distPointPlane(Vector3 vPoint, Plane plane) {
  return vPoint * plane.Normal + plane.D;
}

float distRayPlane(Vector3 vRayOrigin, Vector3 vnRayVector,
                   Vector3 vnPlaneNormal, float planeD) {
  float cosAlpha = NAN;
  float deltaD = NAN;

  cosAlpha = vnRayVector * vnPlaneNormal;
  // parallel to the plane (alpha=90)
  if (cosAlpha == 0) return -1.0f;
  deltaD = planeD - vRayOrigin * vnPlaneNormal;

  return (deltaD / cosAlpha);
}

bool testfrustum(Vector3 c, Plane planes[4], int skip) {
  bool v = c.z > 0;
  for (int x = 0; x < 4; x++) {
    if (x != skip) v = v && (distPointPlane(c, planes[x]) < 0);
  }
  return v;
}

Vector3 GW2TacticalDisplay::ProjectTacticalPos(Vector3 pos, float fov,
                                               float asp) {
  Vector3 p = pos;
  float length = p.Length();

  float yfov = fov / 2.0f;

  Vector3 fln, frn, fun, fdn;
  Matrix4x4 rotm;

  float xfov = std::atan(asp * std::tan(yfov));

  rotm = Matrix4x4::Rotation(Vector3(0, 1, 0), -xfov);
  fln = Vector3(Vector3(-1, 0, 0) * rotm);
  rotm = Matrix4x4::Rotation(Vector3(0, 1, 0), xfov);
  frn = Vector3(Vector3(1, 0, 0) * rotm);
  rotm = Matrix4x4::Rotation(Vector3(1, 0, 0), -yfov);
  fun = Vector3(Vector3(0, 1, 0) * rotm);
  rotm = Matrix4x4::Rotation(Vector3(1, 0, 0), yfov);
  fdn = Vector3(Vector3(0, -1, 0) * rotm);

  Plane fplanes[4];
  fplanes[0] = Plane(Vector3(0, 0, 0), fln);
  fplanes[1] = Plane(Vector3(0, 0, 0), frn);
  fplanes[2] = Plane(Vector3(0, 0, 0), fun);
  fplanes[3] = Plane(Vector3(0, 0, 0), fdn);

  if (!testfrustum(p, fplanes, -1)) {
    Vector3 o = p;
    Vector3 res[4];
    float di[4];

    Vector3 vn = (p - Vector3(0, 0, 1)).Normalized();
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

    for (int x = 0; x < 4; x++) {
      if (ok[x] && di[x] < m) {
        p = res[x];
        m = di[x];
      }
    }
  }

  return p.Normalized() * length;
}

float GetMapFade() {
  constexpr int32_t kMapFadeLength = 250;

  int lastMapTime = globalTimer.GetTime() - mumbleLink.last_map_change_time;
  if (mumbleLink.is_map_open && lastMapTime > kMapFadeLength) return 0.0f;

  float mapFade = 1.0f;

  if (mumbleLink.is_map_open) {
    lastMapTime = kMapFadeLength - lastMapTime;
    mapFade = std::min(1.0f, lastMapTime / static_cast<float>(kMapFadeLength));
  }

  return mapFade;
}

std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data) {
  std::unordered_map<int32_t, Achievement> result;

  jsonxx::Object json;
  json.parse(achievements_data);

  if (!json.has<jsonxx::Array>("achievements")) return result;

  auto achi_data = json.get<jsonxx::Array>("achievements").values();

  for (auto& x : achi_data) {
    if (!x->is<jsonxx::Object>()) continue;
    auto& data = x->get<jsonxx::Object>();

    if (!data.has<jsonxx::Boolean>("done")) continue;
    bool done = data.get<jsonxx::Boolean>("done");

    if (!data.has<jsonxx::Number>("id")) continue;
    int32_t achi_id = int32_t(data.get<jsonxx::Number>("id"));
    result[achi_id].done = done;

    if (!done && data.has<jsonxx::Array>("bits")) {
      auto& bit_array = result[achi_id].bits;
      auto bits = data.get<jsonxx::Array>("bits").values();
      for (auto& bit : bits) {
        if (!bit->is<jsonxx::Number>()) continue;
        bit_array.push_back(static_cast<int32_t>(bit->get<jsonxx::Number>()));
      }
    } else if (done) {
      result[achi_id].bits.clear();
    }
  }

  return result;
}

void GW2TacticalDisplay::FetchAchievements() {
  if (GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK) return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (fetchTask.valid() &&
      fetchTask.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
    return;

  if (key && key->Valid() &&
      (GetTime() - lastFetchTime > 150000 || !lastFetchTime)) {
    lastFetchTime = GetTime();
    fetchTask = std::async(std::launch::async, [this, key]() {
      auto achievements_data =
          "{\"achievements\":" + key->QueryAPI("/v2/account/achievements") +
          "}";
      auto incoming = ParseAchievements(achievements_data);
      achievements_queue.push(std::move(incoming));
    });
  }
}

void GW2TacticalDisplay::InsertPOI(POI& poi) {
  // if (poi.map_id != mumbleLink.map_id) return;

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
      Vector4(poi.position.x, poi.position.y + poi.type_data_.height_,
              poi.position.z, 1.0f) *
      cam;

  minimapPOIs.push_back(&poi);

  if (poi.type_data_.fade_far_ >= 0 && poi.type_data_.fade_near_ >= 0) {
    float dist = WorldToGameCoords(poi.cameraSpacePosition.Length());
    if (dist > poi.type_data_.fade_far_) return;
  }

  mapPOIs.push_back(&poi);
}

void GW2TacticalDisplay::DrawPOI(gui::CWBDrawAPI* API, const tm& ptm,
                                 const time_t& currtime, POI& poi,
                                 bool drawDistance, std::string& infoText) {
  bool drawCountdown = false;
  int32_t timeLeft = 0;
  float alphaMultiplier = 1;

  if (!poi.IsVisible(ptm, currtime, achievements)) {
    return;
  }

  if (poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if (elapsedtime < 300) {
      timeLeft = static_cast<int32_t>(300 - elapsedtime);
      drawCountdown = true;
    }
  }

  if (poi.type_data_.behavior_ == POIBehavior::ReappearAfterTimer) {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if (elapsedtime < poi.type_data_.reset_length_) {
      if (poi.type_data_.bits_.has_countdown_) {
        timeLeft =
            static_cast<int32_t>(poi.type_data_.reset_length_ - elapsedtime);
        drawCountdown = true;
      } else {
        return;
      }
    }

    float dist = (poi.position - mumbleLink.char_position).Length();

    if (!drawCountdown &&
        (poi.type_data_.bits_.auto_trigger_ ||
         poi.type_data_.bits_.has_countdown_) &&
        (dist <= poi.type_data_.trigger_range_)) {
      // auto trigger
      POIActivationData d;
      time(&d.lastUpdateTime);
      poi.lastUpdateTime = d.lastUpdateTime;
      d.poiguid = poi.guid;

      int data = 0;
      if (poi.type_data_.behavior_ == POIBehavior::OncePerInstance) {
        data = mumbleLink.map_instance;
      }
      if (poi.type_data_.behavior_ == POIBehavior::DailyPerChar) {
        data = mumbleLink.char_id_hash;
      }
      if (poi.type_data_.behavior_ == POIBehavior::OncePerInstancePerChar) {
        data = mumbleLink.char_id_hash ^ mumbleLink.map_instance;
      }

      ActivationData[POIActivationDataKey(poi.guid, data)] = d;
    }
  }

  if (!poi.type_data_.info_.empty()) {
    if ((poi.position - mumbleLink.char_position).Length() <=
        poi.type_data_.info_range_) {
      infoText += std::string(poi.type_data_.info_) + "\n";
    }
  }

  if (poi.routeMember && ((poi.position - mumbleLink.char_position).Length() <=
                          poi.type_data_.trigger_range_)) {
    for (auto& r : Routes) {
      if (r.activeItem < 0) {
        continue;
      }

      if (r.route[r.activeItem] == poi.guid) {
        // progress route

        if (r.backwards) {
          r.activeItem -= 1;
        } else {
          r.activeItem += 1;
        }

        r.activeItem = (r.activeItem + r.route.size()) % r.route.size();
      }
    }
  }

  // get alpha for map

  float mapFade = GetMapFade();

  gui::WBATLASHANDLE icon = poi.icon;
  float size = poi.type_data_.size_;
  float Alpha = poi.type_data_.alpha_;

  auto camspace = poi.cameraSpacePosition;
  auto screenpos = camspace;

  Vector4 camspacex = camspace + Vector4(0.5f, 0, 0, 0) * size;

  if (TacticalIconsOnEdge) {
    screenpos /= screenpos.w;
    Vector3 projpos =
        ProjectTacticalPos(Vector3(screenpos), mumbleLink.fov, asp);
    screenpos.x = projpos.x;
    screenpos.y = projpos.y;
    screenpos.z = projpos.z;
    // screenpos.Normalize();
    // screenpos *= camspace.Length();
    camspace = screenpos;
    camspacex = camspace + Vector4(0.5f, 0, 0, 0) * size;
  }

  if (!TacticalIconsOnEdge && camspace.z <= 0) return;

  float dist = WorldToGameCoords(camspace.Length());
  if (poi.type_data_.fade_near_ >= 0 && poi.type_data_.fade_far_ >= 0) {
    float fadeAlpha = 1;

    if (dist > poi.type_data_.fade_far_) return;
    if (dist > poi.type_data_.fade_near_) {
      fadeAlpha =
          1 - (dist - poi.type_data_.fade_near_) /
                  (poi.type_data_.fade_far_ - poi.type_data_.fade_near_);
    }

    Alpha *= fadeAlpha;
  }

  camspace = camspace * persp;
  camspace /= camspace.w;

  screenpos = screenpos * persp;
  screenpos /= screenpos.w;

  camspacex = camspacex * persp;
  camspacex /= camspacex.w;

  int s = static_cast<int>(std::min<float>(
      poi.type_data_.max_size_,
      std::max<float>(poi.type_data_.min_size_,
                      std::abs((camspacex - camspace).x) * drawrect.Width())));

  if (poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
    alphaMultiplier = std::max(
        0.f, std::min(1.f, std::pow(Vector2(screenpos.x, screenpos.y).Length(),
                                    2.f) +
                               0.3f));
  }

  screenpos = screenpos * 0.5 + Vector4(0.5, 0.5, 0.5, 0.0);

  Point p = Point(static_cast<int>(screenpos.x * drawrect.Width()),
                  static_cast<int>((1 - screenpos.y) * drawrect.Height()));

  Rect rect = Rect(p - Point(s, s), p + Point(s, s));

  if (TacticalIconsOnEdge) {
    int32_t edge = poi.type_data_.min_size_;

    Point cp = rect.Center();
    if (cp.x < edge) rect = rect + Point(edge - cp.x, 0);
    if (cp.x > drawrect.x2 - edge) {
      rect = rect - Point(drawrect.x2 - cp.x + edge, 0);
    }

    if (cp.y < edge) rect = rect + Point(0, edge - cp.y);
    if (cp.y > drawrect.y2 - edge) {
      rect = rect - Point(0, drawrect.y2 - cp.y + edge);
    }
  }

  if (!drawCountdown || poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
    CColor col = poi.type_data_.color_;
    if (icon != DefaultIconHandle) {
      col.A() = static_cast<uint8_t>(col.A() * Alpha * alphaMultiplier *
                                     mapFade * globalOpacity);
    } else {
      col.A() = static_cast<uint8_t>(col.A() * mapFade * globalOpacity);
    }
    API->DrawAtlasElement(icon, rect, false, false, true, true, col);
  }

  if (drawWvWNames && poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
    gui::CWBFont* f = App->GetDefaultFont();
    std::string wvw_objective_name;

    if (poi.wvw_objective_id < wvw_objectives.size()) {
      wvw_objective_name =
          DICT(wvw_objectives[poi.wvw_objective_id].name_token_,
               wvw_objectives[poi.wvw_objective_id].name_);
    }

    if (!wvw_objective_name.empty()) {
      p = f->GetTextPosition(wvw_objective_name, rect,
                             gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
                             gui::WBTEXTALIGNMENTY::WBTA_TOP,
                             gui::WBTEXTTRANSFORM::WBTT_UPPERCASE, false) -
          Point(0, f->GetLineHeight());
      /*
      for (int32_t x = 0; x < 3; x++)
        for (int32_t y = 0; y < 3; y++)
          f->Write(API, wvw_objective_name, p + Point(x - 1, y - 1),
                   CColor(0, 0, 0,
                          uint8_t(255 * alphaMultiplier * globalOpacity *
                                  mapFade / 2.0f)),
                   gui::WBTEXTTRANSFORM::WBTT_UPPERCASE, false);
      */
      f->Write(API, wvw_objective_name, p,
               CColor(255, 255, 0,
                      static_cast<uint8_t>(255 * alphaMultiplier * mapFade *
                                           globalOpacity)),
               gui::WBTEXTTRANSFORM::WBTT_UPPERCASE, false);
    }
  }

  if (drawCountdown) {
    gui::CWBFont* f = GetFont(GetState());
    if (!f) return;

    if (poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
      f = App->GetDefaultFont();
    }

    std::string txt;
    int32_t seconds = timeLeft % 60;
    int32_t minutes = (timeLeft - seconds) / 60;
    int32_t hours = (timeLeft - seconds - minutes * 60) / 60;

    if (hours) txt += std::format("{:02d}:", hours);

    if (minutes) txt += std::format("{:02d}:", minutes);

    txt += std::format("{:02d}", seconds);

    int32_t offset = 0;
    if (drawDistance) offset += f->GetLineHeight();

    Point p;
    if (poi.type_data_.behavior_ == POIBehavior::WvwObjective) {
      if (forbiddenIconHandle != -1) {
        CColor col(0xffffffff);
        if (icon != DefaultIconHandle) {
          col.A() = static_cast<uint8_t>(col.A() * Alpha * alphaMultiplier *
                                         mapFade * globalOpacity);
        } else {
          col.A() = static_cast<uint8_t>(col.A() * mapFade * globalOpacity);
        }
        API->DrawAtlasElement(forbiddenIconHandle, rect, false, false, true,
                              true, col);
      }

      p = f->GetTextPosition(txt, rect, gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
                             gui::WBTEXTALIGNMENTY::WBTA_BOTTOM,
                             gui::WBTEXTTRANSFORM::WBTT_NONE, false) +
          Point(0, f->GetLineHeight() + offset);
      /*
      for (int32_t x = 0; x < 3; x++)
        for (int32_t y = 0; y < 3; y++)
          f->Write(API, txt, p + Point(x - 1, y - 1),
                   CColor(0, 0, 0,
                          uint8_t(255 * alphaMultiplier * globalOpacity *
                                  mapFade / 2.0f)),
                   gui::WBTEXTTRANSFORM::WBTT_NONE, false);
      */
      f->Write(API, txt, p,
               CColor(255, 255, 0,
                      static_cast<uint8_t>(255 * alphaMultiplier * mapFade *
                                           globalOpacity)),
               gui::WBTEXTTRANSFORM::WBTT_NONE, false);
    } else {
      p = f->GetTextPosition(txt, rect, gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
                             gui::WBTEXTALIGNMENTY::WBTA_CENTERY,
                             gui::WBTEXTTRANSFORM::WBTT_NONE, false);
      p.y += offset;
      f->Write(API, txt, p,
               CColor(255, 255, 0,
                      static_cast<uint8_t>(255 * mapFade * globalOpacity)),
               gui::WBTEXTTRANSFORM::WBTT_NONE, false);
    }
  }

  if (drawDistance) {
    gui::CWBFont* f = App->GetDefaultFont();
    if (!f) return;

    if (Alpha * alphaMultiplier > 0) {
      float charDist =
          WorldToGameCoords((poi.position - mumbleLink.char_position).Length());

      std::string txt;

      if (!useMetricDisplay) {
        txt = std::format("{:d}", static_cast<int32_t>(charDist));
      } else {
        charDist *= 0.0254f;
        txt = std::format("{:.1f}m", charDist);
      }

      p = f->GetTextPosition(txt, rect, gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
                             gui::WBTEXTALIGNMENTY::WBTA_BOTTOM,
                             gui::WBTEXTTRANSFORM::WBTT_NONE, false) +
          Point(0, f->GetLineHeight());
      f->Write(API, txt, p,
               CColor(255, 255, 255,
                      static_cast<uint8_t>(255 * Alpha * alphaMultiplier *
                                           mapFade * globalOpacity)),
               gui::WBTEXTTRANSFORM::WBTT_NONE, false);
    }
  }
}

float uiScale = 1.0f;

void GW2TacticalDisplay::DrawPOIMinimap(gui::CWBDrawAPI* API,
                                        const Rect& miniRect, Vector2 pos,
                                        const tm& ptm, const time_t& currtime,
                                        const POI& poi, float alpha,
                                        float zoomLevel) {
  if (alpha <= 0) return;
  if (!poi.IsVisible(ptm, currtime, achievements)) {
    return;
  }

  if (!poi.type_data_.bits_.keep_on_map_edge_ &&
      !miniRect.Contains(
          Point(static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y)))) {
    return;
  }

  if (poi.type_data_.bits_.keep_on_map_edge_) {
    pos.x = std::min(static_cast<float>(miniRect.x2),
                     std::max(static_cast<float>(miniRect.x1), pos.x));
    pos.y = std::min(static_cast<float>(miniRect.y2),
                     std::max(static_cast<float>(miniRect.y1), pos.y));
  }

  auto poiSize = static_cast<float>(poi.type_data_.mini_map_size_);
  if (poi.type_data_.bits_.scale_with_zoom_) poiSize /= zoomLevel;
  poiSize *= uiScale;

  alpha *=
      1.0f -
      std::max(0.0f, std::min(1.0f, (zoomLevel -
                                     poi.type_data_.mini_map_fade_out_level_) /
                                        2.0f));

  Vector2 startPoint = pos - Vector2(poiSize / 2.0f, poiSize / 2.0f);
  Point topLeft = Point(static_cast<int32_t>(startPoint.x),
                        static_cast<int32_t>(startPoint.y));

  Rect displayRect(topLeft, topLeft);
  displayRect.x2 = topLeft.x + static_cast<int32_t>(poiSize);
  displayRect.y2 = topLeft.y + static_cast<int32_t>(poiSize);

  CColor col = poi.type_data_.color_;
  col.A() = static_cast<uint8_t>(col.A() * alpha * minimapOpacity *
                                 poi.type_data_.alpha_);

  API->DrawAtlasElement(poi.icon, displayRect, false, false, true, true, col);
}

void GW2TacticalDisplay::OnDraw(gui::CWBDrawAPI* API) {
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

  int showMinimapMarkers = GetConfigValue("ShowMinimapMarkers");

  int showBigmapMarkers = GetConfigValue("ShowBigmapMarkers");

  int showIngameMarkers = GetConfigValue("ShowInGameMarkers");

  FetchAchievements();
  UpdateWvwStatus();

  TacticalIconsOnEdge = GetConfigValue("TacticalIconsOnEdge");
  drawWvWNames = GetConfigValue("DrawWvWNames") != 0;
  bool drawDistance = GetConfigValue("TacticalDrawDistance") != 0;

  time_t rawtime = 0;
  time(&rawtime);
  struct tm ptm {};
  gmtime_s(&ptm, &rawtime);

  mapPOIs.clear();
  minimapPOIs.clear();

  drawrect = GetClientRect();

  cam.SetLookAtLH(mumbleLink.cam_position,
                  mumbleLink.cam_position + mumbleLink.cam_dir,
                  Vector3(0, 1, 0));
  persp.SetPerspectiveFovLH(
      mumbleLink.fov, drawrect.Width() / static_cast<float>(drawrect.Height()),
      0.01f, 1000.0f);

  asp = drawrect.Width() / static_cast<float>(drawrect.Height());

  const int mumblemap_id = mumbleLink.map_id;

  auto& mPOIs = GetMapPOIs();

  for (auto& poi : mPOIs) {
    if (poi.second.map_id != mumblemap_id) {
      continue;
    }
    InsertPOI(poi.second);
  }

  if (wvw_can_be_rendered) {
    const auto& updates = wvw_poi_updates.pop();
    if (updates.has_value()) {
      for (auto& e : updates.value()) {
        if (wvw_pois.find(e.id_) == wvw_pois.end()) continue;
        auto& poi = wvw_pois[e.id_];
        poi.lastUpdateTime = e.last_flipped_;
        switch (e.owner_) {
          case WvwPoiUpdate::Team::kRed:
            poi.type_data_.color_ = CColor{0xffe53b3b};
            break;
          case WvwPoiUpdate::Team::kBlue:
            poi.type_data_.color_ = CColor{0xff3aa2fa};
            break;
          case WvwPoiUpdate::Team::kGreen:
            poi.type_data_.color_ = CColor{0xff3dca67};
            break;
          default:
            poi.type_data_.color_ = CColor{0xffffffff};
            break;
        }
      }
    }
    for (auto& e : wvw_pois) {
      if (e.second.map_id != mumblemap_id) {
        continue;
      }
      InsertPOI(e.second);
    }
  }

  time_t currtime = 0;
  time(&currtime);

  std::sort(mapPOIs.begin(), mapPOIs.end(), [](POI* a, POI* b) {
    return b->cameraSpacePosition.z > a->cameraSpacePosition.z;
  });

  for (auto& r : Routes) {
    if (r.hasResetPos && r.map_id == mumbleLink.map_id &&
        (r.resetPos - mumbleLink.char_position).Length() < r.resetRad) {
      r.activeItem = 0;
    }
  }

  std::string infoText;

  if (showIngameMarkers > 0) {
    for (const auto& mp : mapPOIs) {
      if (!mp->type_data_.bits_.in_game_visible_ && showIngameMarkers != 2)
        continue;
      if (!mp->icon) {
        mp->icon = GetMapIcon(App, mp->icon_file_, mp->zip_file_,
                              mp->category ? mp->category->zip_file : "");
      }
      DrawPOI(API, ptm, currtime, *mp, drawDistance, infoText);
    }
  }

  // punch hole in minimap

  Rect miniRect = GetMinimapRectangle();

  API->FlushDrawBuffer();
  API->GetDevice()->SetRenderState(
      (dynamic_cast<OverlayApplication*>(App))->hole_punch_blend_state.get());

  API->DrawRect(miniRect, CColor(0, 0, 0, 0));

  API->FlushDrawBuffer();
  API->SetUIRenderState();

  // draw minimap trails

  auto* trails = dynamic_cast<GW2TrailDisplay*>(
      App->GetRoot()->FindChildByID("trail", "gw2Trails"));
  if (trails) trails->DrawProxy(API, true);

  // draw minimap

  float mapFade = GetMapFade();

  const auto& new_achievements = achievements_queue.pop();
  if (new_achievements.has_value()) {
    auto achievemenst_data = new_achievements.value();
    std::swap(achievements, achievemenst_data);
  }

  if (mapFade > 0 && showMinimapMarkers > 0) {
    Matrix4x4 miniMapTrafo =
        mumbleLink.mini_map.BuildTransformationMatrix(miniRect, false);
    for (const auto& mmp : minimapPOIs) {
      if (!mmp->type_data_.bits_.mini_map_visible_ && showMinimapMarkers != 2) {
        continue;
      }
      if (!mmp->IsVisible(ptm, currtime, achievements)) {
        continue;
      }

      Vector3 poiPos(mmp->position * miniMapTrafo);
      if (!mmp->icon) {
        mmp->icon = GetMapIcon(App, mmp->icon_file_, mmp->zip_file_,
                               mmp->category ? mmp->category->zip_file : "");
      }
      DrawPOIMinimap(API, miniRect, Vector2(poiPos.x, poiPos.y), ptm, currtime,
                     *mmp, mapFade, mumbleLink.mini_map.map_scale);
    }
  }

  if (mumbleLink.is_map_open && mapFade < 1.0 && showBigmapMarkers > 0) {
    miniRect = GetClientRect();
    Matrix4x4 miniMapTrafo =
        mumbleLink.big_map.BuildTransformationMatrix(miniRect, true);
    for (const auto& mmp : minimapPOIs) {
      if (!mmp->type_data_.bits_.big_map_visible_ && showBigmapMarkers != 2)
        continue;
      if (!mmp->IsVisible(ptm, currtime, achievements)) {
        continue;
      }

      Vector3 poiPos(mmp->position * miniMapTrafo);
      if (!mmp->icon) {
        mmp->icon = GetMapIcon(App, mmp->icon_file_, mmp->zip_file_,
                               mmp->category ? mmp->category->zip_file : "");
      }
      DrawPOIMinimap(API, miniRect, Vector2(poiPos.x, poiPos.y), ptm, currtime,
                     *mmp, 1.0f - mapFade, mumbleLink.big_map.map_scale);
    }
  }

  if (GetConfigValue("TacticalInfoTextVisible")) {
    auto font = GetApplication()->GetRoot()->GetFont(GetState());
    int32_t width = font->GetWidth(infoText);
    font->Write(
        API, infoText,
        Point(static_cast<int>((GetClientRect().Width() - width) / 2.0f),
              static_cast<int>(GetClientRect().Height() * 0.15f)));
  }
}

GW2TacticalDisplay::GW2TacticalDisplay() : CWBGuiType() {}

GW2TacticalDisplay::~GW2TacticalDisplay() {}

gui::CWBItem* GW2TacticalDisplay::Factory(gui::CWBItem* Root,
                                          const CXMLNode& node, Rect& Pos) {
  return GW2TacticalDisplay::Create(Root, Pos);
}

bool GW2TacticalDisplay::IsMouseTransparent(const Point& ClientSpacePoint,
                                            gui::WBMESSAGE MessageType) {
  return true;
}

void GW2TacticalDisplay::RemoveUserMarkersFromMap() {
  if (!mumbleLink.IsValid()) return;

  auto& mPOIs = GetMapPOIs();
  for (auto& poi : mPOIs) {
    if (poi.second.map_id == mumbleLink.map_id && !poi.second.External) {
      mPOIs.erase(poi.first);
    }
  }

  ExportPOIS();
}

bool FindSavedCategory(GW2TacticalCategory* t) {
  if (t->keep_save_state) return true;
  for (const auto& c : t->children) {
    if (FindSavedCategory(c.get())) return true;
  }
  return false;
}

void ExportSavedCategories(CXMLNode* n, GW2TacticalCategory* t) {
  if (!FindSavedCategory(t)) return;
  auto& nn = n->AddChild("MarkerCategory");
  nn.SetAttribute("name", t->name);
  if (t->name != t->display_name)
    nn.SetAttribute("DisplayName", t->display_name);
  t->data.Write(&nn);
  for (const auto& c : t->children) ExportSavedCategories(&nn, c.get());
}

void ExportPOI(CXMLNode* n, POI& p) {
  CXMLNode* t = &n->AddChild("POI");
  t->SetAttributeFromInteger("MapID", p.map_id);
  t->SetAttributeFromFloat("xpos", p.position.x);
  t->SetAttributeFromFloat("ypos", p.position.y);
  t->SetAttributeFromFloat("zpos", p.position.z);
  if (!p.Type.empty()) {
    t->SetAttribute("type", p.Type);
  }
  t->SetAttribute(
      "GUID", B64Encode(std::string_view(reinterpret_cast<const char*>(&p.guid),
                                         sizeof(GUID))));
  p.type_data_.Write(t);
}

void ExportTrail(CXMLNode* n, GW2Trail& p) {
  CXMLNode* t = &n->AddChild("Trail");
  if (!p.type_.empty()) t->SetAttribute("type", p.type_);
  t->SetAttribute("GUID",
                  B64Encode(std::string_view(
                      reinterpret_cast<const char*>(&p.guid_), sizeof(GUID))));
  p.type_data_.Write(t);
}

void ExportPOIS() {
  CXMLDocument d;
  CXMLNode root = d.GetDocumentNode();
  CXMLNode& overlayData = root.AddChild("OverlayData");

  for (const auto& c : CategoryRoot.children) {
    ExportSavedCategories(&overlayData, c.get());
  }

  CXMLNode* n = &overlayData.AddChild("POIs");

  auto& mPOIs = GetMapPOIs();
  for (auto& poi : mPOIs) {
    if (!poi.second.External && !poi.second.routeMember) {
      ExportPOI(n, poi.second);
    }
  }

  for (auto& x : trails) {
    for (auto& t : x.second) {
      auto& p = t.second;
      if (!p->external_) ExportTrail(n, *p);
    }
  }

  for (const auto& r : Routes) {
    if (r.external) continue;

    CXMLNode* t = &n->AddChild("Route");
    t->SetAttribute("Name", r.name);
    t->SetAttributeFromInteger("BackwardDirection",
                               static_cast<int32_t>(r.backwards));

    for (const auto& ar : r.route) {
      const auto& fpoi = mPOIs.find(ar);
      if (fpoi != mPOIs.end()) {
        ExportPOI(t, fpoi->second);
      }
    }
  }

  d.SaveToFile("poidata.xml");
}

GUID LoadGUID(CXMLNode& n) {
  auto guidb64 = n.GetAttributeAsString("GUID");

  auto data = B64Decode(guidb64);

  GUID guid;

  if (data.size() == sizeof(GUID)) {
    memcpy(&guid, data.c_str(), sizeof(GUID));
  } else {
    CoCreateGuid(&guid);
  }

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

    for (char& x : name) {
      if (!isalnum(x) && x != '.') x = '_';
    }

    std::string displayName;
    std::string newCatName(currentCategory);

    auto nameExploded = Split(name, ".");

    GW2TacticalCategory* c = nullptr;

    for (auto& y : nameExploded) {
      GW2TacticalCategory* Root2 = Root;

      if (!newCatName.empty()) newCatName += ".";
      newCatName += y;
      std::transform(newCatName.begin(), newCatName.end(), newCatName.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      c = GetCategory(newCatName);

      if (!c) {
        auto nc = std::make_unique<GW2TacticalCategory>();
        c = nc.get();
        Root2->children.emplace_back(std::move(nc));
        c->name = y;
        c->data = defaults;
        CategoryMap[newCatName] = c;
        c->parent = Root2;
        Root2 = c;
        displayName = c->name;

        std::transform(c->name.begin(), c->name.end(), c->name.begin(),
                       [](unsigned char c) { return std::tolower(c); });
      }
    }

    if (!c) continue;

    if (c->display_name.empty()) c->display_name = displayName;

    if (n.HasAttribute("DisplayName")) {
      displayName = n.GetAttribute("DisplayName");
      c->display_name = displayName;
      localization->ProcessStringForUsedGlyphs(displayName);
    }

    if (n.HasAttribute("IsSeparator")) {
      int separator = 0;
      n.GetAttributeAsInteger("IsSeparator", &separator);
      c->is_only_separator = separator;
    }

    c->data.Read(n, KeepSaveState);
    c->zip_file = AddStringToSet(zipFile);
    c->keep_save_state = KeepSaveState;

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

void ImportPOI(CXMLNode& t, POI& p, std::string_view zipFile) {
  if (t.HasAttribute("MapID")) t.GetAttributeAsInteger("MapID", &p.map_id);
  if (t.HasAttribute("xpos")) t.GetAttributeAsFloat("xpos", &p.position.x);
  if (t.HasAttribute("ypos")) t.GetAttributeAsFloat("ypos", &p.position.y);
  if (t.HasAttribute("zpos")) t.GetAttributeAsFloat("zpos", &p.position.z);
  if (t.HasAttribute("icon")) t.GetAttributeAsInteger("icon", &p.icon);
  if (t.HasAttribute("type")) {
    p.Type = AddStringToSet(t.GetAttributeAsString("type"));
  }

  if (!t.HasAttribute("GUID")) {
    CoCreateGuid(&p.guid);
  } else {
    p.guid = LoadGUID(t);
  }

  p.zip_file_ = AddStringToSet(zipFile);

  auto* td = GetCategory(p.Type);
  if (td) p.SetCategory(td);

  p.type_data_.Read(t, true);

  p.icon_file_ = p.type_data_.icon_file_;
}

bool ImportTrail(CXMLNode& t, GW2Trail& p, std::string_view zipFile) {
  p.zip_file_ = zipFile;

  if (t.HasAttribute("type")) p.type_ = t.GetAttributeAsString("type");

  if (!t.HasAttribute("GUID")) {
    CoCreateGuid(&p.guid_);
  } else {
    p.guid_ = LoadGUID(t);
  }

  auto* td = GetCategory(p.type_);
  if (td) p.SetCategory(td);

  p.type_data_.Read(t, true);

  return p.Import(p.type_data_.trail_data_, zipFile);
}

void ImportPOIDocument(CXMLDocument& d, bool External,
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
        ImportPOI(t, p, zipFile);
        p.External = External;
        POIs[p.map_id][p.guid] = p;
      } while (t.Next(t, "POI"));
    }

    for (int32_t x = 0; x < n.GetChildCount("Route"); x++) {
      CXMLNode rn = n.GetChild("Route", x);
      POIRoute r;
      if (rn.HasAttribute("Name")) r.name = rn.GetAttributeAsString("Name");
      int32_t b = false;
      if (rn.HasAttribute("BackwardDirection")) {
        rn.GetAttributeAsInteger("BackwardDirection", &b);
      }
      if (rn.HasAttribute("MapID"))
        rn.GetAttributeAsInteger("MapID", &r.map_id);
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
          ImportPOI(t, p, zipFile);
          p.External = External;
          p.routeMember = true;
          POIs[p.map_id][p.guid] = p;
          r.route.push_back(p.guid);
        } while (t.Next(t, "POI"));
      }

      Routes.push_back(r);
    }

    if (n.GetChildCount("Trail") > 0) {
      CXMLNode t = n.GetChild("Trail", 0);
      do {
        auto p = std::make_unique<GW2Trail>();
        if (ImportTrail(t, *p, zipFile)) {
          p->external_ = External;
          trails[p->map_][p->guid_] = std::move(p);
        }
      } while (t.Next(t, "Trail"));
    }
  }
}

void ImportPOIFile(std::string_view s, bool External) {
  CXMLDocument d;
  if (!d.LoadFromFile(s.data())) return;
  ImportPOIDocument(d, External, "");
}

void ImportPOIString(std::string_view data, std::string_view zipFile) {
  CXMLDocument d;
  if (!d.LoadFromString(data)) return;
  ImportPOIDocument(d, true, zipFile);
}

void ImportMarkerPack(std::string_view zipFile) {
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
    ImportPOIString(doc, zipFile);
  }
}

void ImportPOIS() {
  FlushZipDict();
  ImportPOITypes();

  POIs.clear();
  Routes.clear();
  trails.clear();

  {
    CFileList list;
    list.ExpandSearch("*.xml", "POIs", false);
    for (auto& File : list.Files) {
      ImportPOIFile(File.Path + File.FileName, true);
    }
  }

  {
    CFileList list;
    list.ExpandSearch("*.zip", "POIs", false);
    for (auto& File : list.Files) {
      ImportMarkerPack(File.Path + File.FileName);
    }
  }

  {
    CFileList list;
    list.ExpandSearch("*.taco", "POIs", false);
    for (auto& File : list.Files) {
      ImportMarkerPack(File.Path + File.FileName);
    }
  }

  ImportPOIFile("poidata.xml", false);

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

      if (t.HasAttribute("lut1")) {
        t.GetAttributeAsInteger(
            "lut1", &(reinterpret_cast<int32_t*>(&p.lastUpdateTime))[0]);
      }
      if (t.HasAttribute("lut2")) {
        t.GetAttributeAsInteger(
            "lut2", &(reinterpret_cast<int32_t*>(&p.lastUpdateTime))[1]);
      }

      p.poiguid = LoadGUID(t);

      p.uniqueData = 0;
      if (t.HasAttribute("instance")) {
        t.GetAttributeAsInteger("instance", &p.uniqueData);
      }

      ActivationData[POIActivationDataKey(p.poiguid, p.uniqueData)] = p;

      for (auto& mPOIs : POIs) {
        const auto& fpoi = mPOIs.second.find(p.poiguid);
        if (fpoi != mPOIs.second.end()) {
          fpoi->second.lastUpdateTime = p.lastUpdateTime;
        }
      }
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
    for (auto& mPOIs : POIs) {
      const auto& fpoi = mPOIs.second.find(dat.poiguid);
      if (fpoi != mPOIs.second.end()) {
        auto& poi = fpoi->second;
        if (poi.type_data_.behavior_ == POIBehavior::AlwaysVisible) {
          continue;
        }
      }
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

std::string default_marker_category = "";

void AddPOI() {
  if (!mumbleLink.IsValid()) {
    return;
  }
  POI poi = {
      .icon = DefaultIconHandle,
      .position = mumbleLink.char_position,
      .map_id = mumbleLink.map_id,
  };

  CoCreateGuid(&poi.guid);

  auto cat = GetCategory(default_marker_category);

  if (poi.map_id == -1) {
    return;
  }
  auto& mPOIs = GetMapPOIs();
  for (auto& poix : mPOIs) {
    if (poix.second.map_id != poi.map_id) {
      continue;
    }
    Vector3 v = poix.second.position - poi.position;
    if (v.Length() < poix.second.type_data_.trigger_range_ &&
        cat == poix.second.category) {
      return;
    }
  }

  if (cat) {
    poi.SetCategory(cat);
  }

  mPOIs[poi.guid] = poi;
  ExportPOIS();
}

void DeletePOI() {
  if (!mumbleLink.IsValid()) {
    return;
  }
  math::Vector3 poi_position = Vector3(mumbleLink.char_position);
  int32_t poi_map_id = mumbleLink.map_id;

  if (poi_map_id == -1) {
    return;
  }
  auto& mPOIs = GetMapPOIs();
  for (const auto& poix : mPOIs) {
    if (poix.second.map_id != poi_map_id) {
      continue;
    }
    Vector3 v = poix.second.position - poi_position;
    if (v.Length() < poix.second.type_data_.trigger_range_) {
      mPOIs.erase(poix.first);
      ExportPOIS();
      return;
    }
  }
}

void UpdatePOI() {
  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.map_id == -1) return;

  bool found = false;

  auto& mPOIs = GetMapPOIs();
  for (auto& poi : mPOIs) {
    auto& cpoi = poi.second;

    if (cpoi.map_id != mumbleLink.map_id) continue;

    Vector3 v = cpoi.position - Vector3(mumbleLink.char_position);
    if (v.Length() < cpoi.type_data_.trigger_range_) {
      const auto& str = cpoi.type_data_.toggle_category_;
      if (!str.empty()) {
        GW2TacticalCategory* cat = GetCategory(str);
        if (cat) {
          cat->is_displayed = !cat->is_displayed;
          CategoryRoot.CalculateVisibilityCache();
          SetConfigValue(("CategoryVisible_" + cat->GetFullTypeName()),
                         cat->is_displayed);
        }
      }

      if (!found && cpoi.type_data_.behavior_ != POIBehavior::AlwaysVisible) {
        POIActivationData d;
        time(&d.lastUpdateTime);
        cpoi.lastUpdateTime = d.lastUpdateTime;
        d.poiguid = cpoi.guid;

        d.uniqueData = 0;
        if (cpoi.type_data_.behavior_ == POIBehavior::OncePerInstance) {
          d.uniqueData = mumbleLink.map_instance;
        }
        if (cpoi.type_data_.behavior_ == POIBehavior::DailyPerChar) {
          d.uniqueData = mumbleLink.char_id_hash;
        }
        if (cpoi.type_data_.behavior_ == POIBehavior::OncePerInstancePerChar) {
          d.uniqueData = mumbleLink.char_id_hash ^ mumbleLink.map_instance;
        }

        ActivationData[POIActivationDataKey(cpoi.guid, d.uniqueData)] = d;
        ExportPOIActivationData();
        found = true;
      }
    }
  }
}

void AddTypeContextMenu(gui::CWBContextItem* ctx,
                        std::vector<GW2TacticalCategory*>& category_list_,
                        const GW2TacticalCategory* Parent,
                        bool AddVisibilityMarkers, int32_t BaseID,
                        bool closeOnClick) {
  for (const auto& dta : Parent->children) {
    std::string txt;
    if (AddVisibilityMarkers) {
      txt += "[" + std::string(dta->is_displayed ? "x" : " ") + "] ";
    }
    if (!dta->display_name.empty()) {
      txt += dta->display_name;
    } else {
      txt += dta->name;
    }

    if (dta->is_only_separator) {
      ctx->AddSeparator();
      if (!dta->display_name.empty()) {
        txt = dta->display_name;
      } else {
        txt = dta->name;
      }
      ctx->AddItem(txt, category_list_.size() + BaseID, false, closeOnClick);
      category_list_.push_back(dta.get());
      ctx->AddSeparator();
    } else {
      auto n =
          ctx->AddItem(txt, category_list_.size() + BaseID,
                       AddVisibilityMarkers && dta->is_displayed, closeOnClick);
      category_list_.push_back(dta.get());
      AddTypeContextMenu(n, category_list_, dta.get(), AddVisibilityMarkers,
                         BaseID, closeOnClick);
    }
  }
}

void AddTypeContextMenu(gui::CWBContextMenu* ctx,
                        std::vector<GW2TacticalCategory*>& category_list_,
                        const GW2TacticalCategory* Parent,
                        bool AddVisibilityMarkers, int32_t BaseID,
                        bool closeOnClick) {
  for (const auto& dta : Parent->children) {
    std::string txt;
    if (AddVisibilityMarkers) {
      txt += "[" + std::string(dta->is_displayed ? "x" : " ") + "] ";
    }
    if (!dta->display_name.empty()) {
      txt += dta->display_name;
    } else {
      txt += dta->name;
    }

    if (dta->is_only_separator) {
      ctx->AddSeparator();
      if (!dta->display_name.empty()) {
        txt = dta->display_name;
      } else {
        txt = dta->name;
      }
      ctx->AddItem(txt, category_list_.size() + BaseID, false, closeOnClick);
      category_list_.push_back(dta.get());
      ctx->AddSeparator();
    } else {
      auto n =
          ctx->AddItem(txt, category_list_.size() + BaseID,
                       AddVisibilityMarkers && dta->is_displayed, closeOnClick);
      category_list_.push_back(dta.get());
      AddTypeContextMenu(n, category_list_, dta.get(), AddVisibilityMarkers,
                         BaseID, closeOnClick);
    }
  }
}

void OpenTypeContextMenu(gui::CWBContextItem* ctx,
                         std::vector<GW2TacticalCategory*>& category_list_,
                         bool AddVisibilityMarkers, int32_t BaseID,
                         bool closeOnClick) {
  category_list_.clear();
  AddTypeContextMenu(ctx, category_list_, &CategoryRoot, AddVisibilityMarkers,
                     BaseID, closeOnClick);
}

void OpenTypeContextMenu(gui::CWBContextMenu* ctx,
                         std::vector<GW2TacticalCategory*>& category_list_,
                         bool AddVisibilityMarkers, int32_t BaseID,
                         bool closeOnClick) {
  category_list_.clear();
  AddTypeContextMenu(ctx, category_list_, &CategoryRoot, AddVisibilityMarkers,
                     BaseID, closeOnClick);
}

float WorldToGameCoords(float world) { return world / 0.0254f; }

float GameToWorldCoords(float game) { return game * 0.0254f; }

void POI::SetCategory(GW2TacticalCategory* t) {
  category = t;
  type_data_ = t->data;
  Type = AddStringToSet(t->GetFullTypeName());
  icon = 0;
  icon_file_ = type_data_.icon_file_;
}

bool POI::IsVisible(
    const tm& ptm, const time_t& currtime,
    const std::unordered_map<int32_t, Achievement>& achievements) const {
  if (category && !category->IsVisible()) return false;

  if (type_data_.behavior_ == POIBehavior::ReappearOnDailyReset) {
    struct tm lasttime {};
    gmtime_s(&lasttime, &lastUpdateTime);
    if (lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon &&
        lasttime.tm_year == ptm.tm_year) {
      return false;
    }
  }

  if (type_data_.behavior_ == POIBehavior::ReappearAfterTimer) {
    time_t elapsedtime = currtime - lastUpdateTime;
    if (elapsedtime < type_data_.reset_length_) {
      if (!type_data_.bits_.has_countdown_) return false;
    }
  }

  if (type_data_.behavior_ == POIBehavior::OnlyVisibleBeforeActivation) {
    if (lastUpdateTime != static_cast<time_t>(0)) return false;
  }

  if (type_data_.behavior_ == POIBehavior::OncePerInstance) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.map_instance)) != ActivationData.end()) {
      return false;
    }
  }

  if (type_data_.behavior_ == POIBehavior::OncePerInstancePerChar) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.map_instance ^ mumbleLink.char_id_hash)) !=
        ActivationData.end()) {
      return false;
    }
  }

  if (type_data_.behavior_ == POIBehavior::DailyPerChar) {
    if (ActivationData.find(POIActivationDataKey(
            guid, mumbleLink.char_id_hash)) != ActivationData.end()) {
      struct tm lasttime {};
      gmtime_s(
          &lasttime,
          &ActivationData[POIActivationDataKey(guid, mumbleLink.char_id_hash)]
               .lastUpdateTime);

      if (lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon &&
          lasttime.tm_year == ptm.tm_year) {
        return false;
      }
    }
  }

  if (routeMember && ((position - mumbleLink.char_position).Length() <=
                      type_data_.trigger_range_)) {
    for (const auto& r : Routes) {
      if (r.activeItem < 0) return false;
    }
  }

  if (type_data_.achievement_id_ != -1) {
    const auto& achievement = achievements.find(type_data_.achievement_id_);
    if (achievement != achievements.end()) {
      const bool done = !achievement->second.done;
      if (type_data_.achievement_bit_ == -1) {
        return !done;
      }
      const auto& bits = achievement->second.bits;
      const bool hasBit = std::find(bits.begin(), bits.end(),
                                    type_data_.achievement_bit_) != bits.end();
      return !done && !hasBit;
    }
  }

  return true;
}
