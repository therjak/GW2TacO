module;

#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "src/core2/core2.h"

export module whiteboard:atlas;

import math;

export namespace gui {

// cache size must be 2^x
constexpr int32_t kAtlasCacheSize = 32;

typedef int32_t WBATLASHANDLE;

class AtlasImage;

// stores a node for the rectpacker
class AtlasNode {
  friend class Atlas;

 public:
  AtlasNode();
  virtual ~AtlasNode();
  AtlasNode* AddNode(int32_t width, int32_t height);
  math::Rect& GetArea();
  AtlasImage* GetImage();

 private:
  math::Rect area_;
  std::array<std::unique_ptr<AtlasNode>, 2> children_;
  bool occupied_ = false;

  AtlasImage* image_ = nullptr;
};

// stores image data not currently in the atlas
class AtlasImage {
 public:
  AtlasImage();
  AtlasImage(const uint8_t* SourceImage, int32_t SrcXRes, int32_t SrcYRes,
             const math::Rect& Source);
  virtual ~AtlasImage();

  WBATLASHANDLE GetHandle();
  uint8_t* GetImage();
  [[nodiscard]] math::Size GetSize() const;
  void TagRequired();
  void ClearRequired();
  bool IsRequired();

 private:
  std::unique_ptr<uint8_t[]> image_;
  int32_t x_res_, y_res_;
  WBATLASHANDLE handle_;

  bool required_;
};

struct AtlasCacheElement {
  WBATLASHANDLE Handle = 0;
  AtlasNode* Node = nullptr;
};

class Atlas {
  friend class DrawAPI;

 public:
  Atlas(int32_t XSize, int32_t YSize);
  virtual ~Atlas();

  bool InitializeTexture(renderer::Device* Device);
  bool UpdateTexture();
  renderer::Texture2D* GetTexture();

  WBATLASHANDLE AddImage(uint8_t* Image, int32_t XRes, int32_t YRes,
                         const math::Rect& SourceArea);
  // doesn't immediately remove image from atlas
  void DeleteImage(WBATLASHANDLE h);

  bool Optimize(bool DebugMode = false);
  bool Reset();

  math::Size GetSize(WBATLASHANDLE h);
  // returns false only if there was not enough room
  // in the atlas to add the requested image
  bool RequestImageUse(WBATLASHANDLE h, math::Rect* UV);
  math::Point GetWhitePixelUV();

  void ClearImageUsageflags();

  [[nodiscard]] int32_t GetXRes() const { return x_res_; }
  [[nodiscard]] int32_t GetYRes() const { return y_res_; }

  bool Resize(renderer::Device* Device, int32_t XSize, int32_t YSize);

 private:
  bool PackImage(AtlasImage* img);

  void FlushCache();
  AtlasNode* GetNodeCached(WBATLASHANDLE Handle);

  int32_t x_res_, y_res_;
  std::unique_ptr<uint8_t[]> image_;
  std::unique_ptr<renderer::Texture2D> atlas_;

  bool texture_update_needed_;

  std::array<AtlasCacheElement, kAtlasCacheSize> atlas_cache_;

  std::unordered_map<WBATLASHANDLE, AtlasNode*> dictionary_;
  std::unordered_map<WBATLASHANDLE, std::unique_ptr<AtlasImage>> image_storage_;

  std::unique_ptr<AtlasNode> root_;

  AtlasImage* white_pixel_;
  // recalculated on each optimization and reset
  math::Point white_pixel_position_;

  std::mutex mtx_;
};

}  // namespace gui
