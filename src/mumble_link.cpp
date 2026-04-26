module;
#include <windows.h>

#include <algorithm>
#include <cstring>
#include <cwchar>
#include <memory>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/base/ring_buffer.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/gw2_tactical.h"

module taco.mumble_link;

import math;
import taco.app;
import taco.overlay_config;
import taco.trail_logger;
import time;
import whiteboard;

using math::Matrix4x4;
using math::Rect;
using math::Vector2;
using math::Vector3;
using math::Vector4;

bool frameTriggered = false;

void ChangeUIScale(int size);

float GetUIScale() {
  float scale = 1.0;
  if (mumbleLink.ui_size == 0) scale = 0.9f;
  if (mumbleLink.ui_size == 2) scale = 1.111f;
  if (mumbleLink.ui_size == 3) scale = 1.224f;

  return scale;
}

float GetWindowTooSmallScale();

Rect GetMinimapRectangle() {
  int w = mumbleLink.mini_map.compass_width;
  int h = mumbleLink.mini_map.compass_height;

  Rect pos;
  Rect size = App->GetRoot()->GetClientRect();
  float scale = GetWindowTooSmallScale();

  pos.x1 = static_cast<int>(size.Width() - w * scale);
  pos.x2 = size.Width();

  if (mumbleLink.is_minimap_top_right) {
    pos.y1 = 1;
    pos.y2 = static_cast<int>(h * scale + 1);
  } else {
    int delta = 37;
    if (mumbleLink.ui_size == 0) delta = 33;
    if (mumbleLink.ui_size == 2) delta = 41;
    if (mumbleLink.ui_size == 3) delta = 45;

    pos.y1 = static_cast<int>(size.Height() - h * scale - delta * scale);
    pos.y2 = static_cast<int>(size.Height() - delta * scale);
  }

  return pos;
}

bool CMumbleLink::Update() {
  bool just_connected = false;

  if (!lm_) {
    HANDLE h_map_object =
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                           sizeof(LinkedMem), mumble_path.c_str());

    if (h_map_object == nullptr) {
      return false;
    }

    lm_ = static_cast<LinkedMem*>(MapViewOfFile(
        h_map_object, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LinkedMem)));
    if (lm_ == nullptr) {
      CloseHandle(h_map_object);
      h_map_object = nullptr;
      return false;
    }
    just_connected = true;
  }

  if (!lm_) {
    return false;
  }

  if (tick_ == lm_->ui_tick) {
    return false;
  } else {
    memcpy(&prev_data_, &last_data_, sizeof(LinkedMem));
    memcpy(&last_data_, lm_, sizeof(LinkedMem));

    tick_ = lm_->ui_tick;

    globalTimer.Update();
    int32_t frametime = GetTime();
    frame_times->Add(frametime - last_frame_time);
    last_frame_time = frametime;
    frameTriggered = true;
  }

  float inter = 1.0;  // +( measurement - last_tick_time ) / last_tick_length;

  interpolation_ = inter;

  char_position = math::Lerp(Vector3(prev_data_.f_avatar_position),
                             Vector3(last_data_.f_avatar_position), inter);
  char_eye = math::Lerp(Vector3(prev_data_.f_avatar_top),
                        Vector3(last_data_.f_avatar_top), 1);
  cam_position = math::Lerp(Vector3(prev_data_.f_camera_position),
                            Vector3(last_data_.f_camera_position), 1);
  cam_up = math::Lerp(Vector3(prev_data_.f_camera_top),
                      Vector3(last_data_.f_camera_top), 1);
  cam_dir = math::Lerp(Vector3(prev_data_.f_camera_front),
                       Vector3(last_data_.f_camera_front), inter);

  char_pos_changed = Vector3(prev_data_.f_avatar_position) !=
                     Vector3(last_data_.f_avatar_position);
  char_eye_changed =
      Vector3(prev_data_.f_avatar_top) != Vector3(last_data_.f_avatar_top);
  cam_pos_changed = Vector3(prev_data_.f_camera_position) !=
                    Vector3(last_data_.f_camera_position);
  cam_dir_changed =
      Vector3(prev_data_.f_camera_front) != Vector3(last_data_.f_camera_front);
  cam_up_changed =
      Vector3(prev_data_.f_camera_top) != Vector3(last_data_.f_camera_top);

  if ((Vector3(last_data_.f_avatar_position) -
       Vector3(prev_data_.f_avatar_position))
          .Length() > GameToWorldCoords(2000)) {
    FindClosestRouteMarkers(true);
  }

  int32_t oldmap_id = map_id;
  map_id = -1;

  std::wstring ident(lm_->identity, 255);
  auto id = ident.find(L"\"map_id\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"map_id\":%d", &map_id);
    if (oldmap_id != map_id) FindClosestRouteMarkers(true);
  } else {
    id = ident.find(L"\"map_id\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"map_id\": %d", &map_id);
      if (oldmap_id != map_id) FindClosestRouteMarkers(true);
    }
  }

  GlobalDoTrailLogging(map_id, Vector3(last_data_.f_avatar_position));

  int32_t old_ui_size = ui_size;

  id = ident.find(L"\"uisz\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"uisz\":%d", &ui_size);
    if (old_ui_size != ui_size) {
      ChangeUIScale(ui_size);
    }
  } else {
    id = ident.find(L"\"uisz\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"uisz\": %d", &ui_size);
      if (old_ui_size != ui_size) {
        ChangeUIScale(ui_size);
      }
    }
  }

  if (just_connected) {
    ChangeUIScale(ui_size);
  }

  id = ident.find(L"\"world_id\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"world_id\":%d", &world_id);
  } else {
    id = ident.find(L"\"world_id\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"world_id\": %d", &world_id);
    }
  }

  auto* ctx = reinterpret_cast<MumbleContext*>(last_data_.context);

  map_type = ctx->map_type;
  map_instance = ctx->shard_id;

  if (is_map_open != (ctx->ui_state & 0x01)) {
    last_map_change_time = globalTimer.GetTime();
  }

  is_map_open = (ctx->ui_state & 0x01);
  is_minimap_top_right = (ctx->ui_state & (0x01 << 1)) != 0;
  is_minimap_rotating = (ctx->ui_state & (0x01 << 2)) != 0;

  game_has_focus = (ctx->ui_state & (0x01 << 3)) != 0;
  is_pvp = (ctx->ui_state & (0x01 << 4)) != 0;
  textbox_has_focus = (ctx->ui_state & (0x01 << 5)) != 0;
  is_in_combat = (ctx->ui_state & (0x01 << 6)) != 0;

  float scale = GetUIScale();

  if (!is_map_open) {
    mini_map.compass_width = static_cast<int>(ctx->compass_width * scale);
    mini_map.compass_height = static_cast<int>(ctx->compass_height * scale);
    mini_map.compass_rotation = ctx->compass_rotation;
    mini_map.player_x = ctx->player_x;
    mini_map.player_y = ctx->player_y;
    mini_map.map_center_x = ctx->map_center_x;
    mini_map.map_center_y = ctx->map_center_y;
    mini_map.map_scale = ctx->map_scale;
  } else {
    big_map.compass_width = static_cast<int>(ctx->compass_width * scale);
    big_map.compass_height = static_cast<int>(ctx->compass_height * scale);
    big_map.compass_rotation = ctx->compass_rotation;
    big_map.player_x = ctx->player_x;
    big_map.player_y = ctx->player_y;
    big_map.map_center_x = ctx->map_center_x;
    big_map.map_center_y = ctx->map_center_y;
    big_map.map_scale = ctx->map_scale;
  }

  last_gw2_process_id = ctx->process_id;

  id = ident.find(L"\"name\":");
  if (id != ident.npos) {
    int end = ident.substr(id + 8).find(L'\"');
    if (end != ident.npos) {
      char_name = wstring2string(ident.substr(id + 8, end));
      char_id_hash = CalculateHash(char_name);
    } else {
      char_name = "";
      char_id_hash = 0;
    }
  } else {
    id = ident.find(L"\"name\": ");
    if (id >= 0) {
      int end = ident.substr(id + 9).find(L'\"');
      if (end != ident.npos) {
        char_name = wstring2string(ident.substr(id + 9, end));
        char_id_hash = CalculateHash(char_name);
      } else {
        char_name = "";
        char_id_hash = 0;
      }
    } else {
      char_name = "";
      char_id_hash = 0;
    }
  }

  fov = 0;

  id = ident.find(L"\"fov\":");
  if (id != ident.npos) {
    std::swscanf(ident.substr(id).c_str(), L"\"fov\":%f", &fov);
  } else {
    id = ident.find(L"\"fov\": ");
    if (id != ident.npos) {
      std::swscanf(ident.substr(id).c_str(), L"\"fov\": %f", &fov);
    }
  }

  Matrix4x4 cam;
  cam.SetLookAtLH(cam_position, cam_position + cam_dir, Vector3(0, 1, 0));

  Matrix4x4 cami = cam.Inverted();

  for (int x = 0; x < kAvgCamCounter - 1; x++) {
    cam_char_dist_[x] = cam_char_dist_[x + 1];
  }
  cam_char_dist_[kAvgCamCounter - 1] = char_position * cam;

  Vector4 avg_cam_char_dist(0, 0, 0, 0);

  for (const auto& x : cam_char_dist_) avg_cam_char_dist += x;

  avg_cam_char_dist /= static_cast<float>(kAvgCamCounter);
  averaged_char_position = avg_cam_char_dist * cami;
  averaged_char_position /= averaged_char_position.w;

  if (!GetConfigValue("SmoothCharacterPos")) {
    averaged_char_position =
        Vector4(char_position.x, char_position.y, char_position.z, 1.0f);
  }

  return true;
}

float CMumbleLink::GetFrameRate() {
  int32_t frame_time_acc = 0;
  int32_t frame_count = 0;
  for (int32_t x = 0; x < 60; x++) {
    if (frame_times->NumItems() < x) break;
    frame_time_acc += (*frame_times)[frame_times->NumItems() - 1 - x];
    frame_count++;
  }

  if (!frame_count) return 0;
  if (!frame_time_acc) return 9999;
  return 1000.0f / (frame_time_acc / static_cast<float>(frame_count));
}

CMumbleLink::CMumbleLink(std::string_view mumble_path)
    : frame_times(std::make_unique<CRingBuffer<int32_t, 60>>()),
      last_frame_time(GetTime()),
      mumble_path(mumble_path) {}

CMumbleLink::~CMumbleLink() = default;

bool CMumbleLink::IsValid() {
  return lm_ != nullptr && last_gw2_process_id != 0;
}

Matrix4x4 CompassData::BuildTransformationMatrix(const Rect& mini_rect,
                                                 bool ignore_rotation) {
  Matrix4x4 mini_map_trafo(1 / 0.0254f, 0, 0, 0, 0, 0, 0, 0, 0, 1 / 0.0254f, 0,
                           0, 0, 0, 0, 1);

  Vector2 map_offset = Vector2(WorldToGameCoords(mumbleLink.char_position.x),
                               WorldToGameCoords(mumbleLink.char_position.z));

  float rotation = ignore_rotation ? 0 : compass_rotation;

  mini_map_trafo *=
      Matrix4x4::Translation(Vector3(-map_offset.x, -map_offset.y, 0.0));
  mini_map_trafo *= Matrix4x4::Scaling(Vector3(1, -1, 1));
  mini_map_trafo *= Matrix4x4::Rotation(Vector3(0, 0, 1), rotation);
  mini_map_trafo *= Matrix4x4::Scaling(Vector3(1, 1, 1) / 24.0f);

  Vector2 offset =
      -((Vector2(map_center_x, map_center_y) - Vector2(player_x, player_y)) *
        GetWindowTooSmallScale())
           .Rotated(Vector2(0, 0), rotation);
  mini_map_trafo *= Matrix4x4::Translation(Vector3(offset.x, offset.y, 0.0));
  mini_map_trafo *=
      Matrix4x4::Scaling(Vector3(1, 1, 1) / map_scale * GetUIScale());
  mini_map_trafo *= Matrix4x4::Translation(
      Vector3(static_cast<float>(mini_rect.Center().x),
              static_cast<float>(mini_rect.Center().y), 0.0));

  return mini_map_trafo;
}
