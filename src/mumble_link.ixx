module;
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "src/base/ring_buffer.h"

export module taco.mumble_link;

import math;

export struct MumbleContext {
  unsigned char server_address[28];  // contains sockaddr_in or sockaddr_in6
  uint32_t map_id;
  uint32_t map_type;
  uint32_t shard_id;
  uint32_t instance;
  uint32_t build_id;
  // Additional data beyond the 48 bytes Mumble uses for identification
  uint32_t ui_state;  // Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight,
                      // Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game
                      // has focus, Bit 5 = Is in Competitive game mode, Bit 6 =
                      // Textbox has focus, Bit 7 = Is in Combat
  uint16_t compass_width;   // pixels
  uint16_t compass_height;  // pixels
  float compass_rotation;   // radians
  float player_x;           // continentCoords
  float player_y;           // continentCoords
  float map_center_x;       // continentCoords
  float map_center_y;       // continentCoords
  float map_scale;
  uint32_t process_id;
  uint8_t mount_index;
};

export struct CompassData {
  int compass_width = 0;       // pixels
  int compass_height = 0;      // pixels
  float compass_rotation = 0;  // guessing... radians? :-P
  float player_x = 0;          // continentCoords
  float player_y = 0;          // continentCoords
  float map_center_x = 0;      // continentCoords
  float map_center_y = 0;      // continentCoords
  float map_scale = 0;         // not even sure TBH :-P
  math::Matrix4x4 BuildTransformationMatrix(const math::Rect& mini_rect,
                                            bool ignore_rotation);
};

export struct LinkedMem {
  uint32_t ui_version = 0;
  uint32_t ui_tick = 0;
  float f_avatar_position[3] = {0};
  float f_avatar_front[3] = {0};
  float f_avatar_top[3] = {0};
  wchar_t name[256] = {0};
  float f_camera_position[3] = {0};
  float f_camera_front[3] = {0};
  float f_camera_top[3] = {0};
  wchar_t identity[256] = {0};
  uint32_t context_len = 0;
  unsigned char context[256] = {0};
  wchar_t description[2048] = {0};
};

export constexpr int kAvgCamCounter = 6;

export class CMumbleLink {
 public:
  CMumbleLink(std::string_view mumble_path);
  virtual ~CMumbleLink();

  bool Update();
  bool IsValid();
  float GetFrameRate();

  math::Vector3 char_position;
  math::Vector3 char_eye;
  math::Vector3 cam_position;
  math::Vector3 cam_dir;
  math::Vector3 cam_up;
  float fov = 0;
  int32_t map_id = 0;
  int32_t world_id = 0;
  int32_t map_type = 0;
  int32_t map_instance = 0;
  int32_t char_id_hash = 0;

  int32_t last_map_change_time = 0;
  bool is_map_open = false;  // bit 1: IsMapOpen, bit2: IsCompassTopRight, bit3:
                             // DoesCompassHaveRotationEnabled
  bool is_minimap_top_right = false;
  bool is_minimap_rotating = false;
  bool game_has_focus = false;
  bool is_pvp = false;
  bool textbox_has_focus = false;
  bool is_in_combat = false;

  CompassData mini_map;
  CompassData big_map;

  int32_t ui_size = 1;

  std::string char_name;

  math::Vector4 averaged_char_position;

  std::unique_ptr<CRingBuffer<int32_t, 60>> frame_times;

  int32_t last_frame_time = 0;

  uint64_t last_tick_time = 0;
  uint64_t last_tick_length = 0;

  bool char_pos_changed = false;
  bool char_eye_changed = false;
  bool cam_pos_changed = false;
  bool cam_dir_changed = false;
  bool cam_up_changed = false;

  std::string mumble_path;
  uint32_t last_gw2_process_id = 0;

 private:
  LinkedMem last_data_;
  LinkedMem prev_data_;

  int tick_ = 0;
  double interpolation_ = 0;

  LinkedMem* lm_ = nullptr;
  math::Vector4 cam_char_dist_[kAvgCamCounter];
};

export CMumbleLink mumbleLink("MumbleLink");
export math::Rect GetMinimapRectangle();
