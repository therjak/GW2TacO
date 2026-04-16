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

class CAtlasImage;

// stores a node for the rectpacker
class CAtlasNode {
  friend class CAtlas;

 public:
  CAtlasNode();
  virtual ~CAtlasNode();
  CAtlasNode* AddNode(int32_t width, int32_t height);
  math::Rect& GetArea();
  CAtlasImage* GetImage();

 private:
  math::Rect Area;
  std::array<std::unique_ptr<CAtlasNode>, 2> Children;
  bool Occupied = false;

  CAtlasImage* Image = nullptr;
};

// stores image data not currently in the atlas
class CAtlasImage {
 public:
  CAtlasImage();
  CAtlasImage(const uint8_t* SourceImage, int32_t SrcXRes, int32_t SrcYRes,
              const math::Rect& Source);
  virtual ~CAtlasImage();

  WBATLASHANDLE GetHandle();
  uint8_t* GetImage();
  [[nodiscard]] math::Size GetSize() const;
  void TagRequired();
  void ClearRequired();
  bool IsRequired();

 private:
  std::unique_ptr<uint8_t[]> Image;
  int32_t XRes, YRes;
  WBATLASHANDLE Handle;

  bool Required;
};

struct CAtlasCacheElement {
  WBATLASHANDLE Handle = 0;
  CAtlasNode* Node = nullptr;
};

class CAtlas {
  friend class CWBDrawAPI;

 public:
  CAtlas(int32_t XSize, int32_t YSize);
  virtual ~CAtlas();

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
  bool RequestImageUse(WBATLASHANDLE h, math::Rect& UV);
  math::Point GetWhitePixelUV();

  void ClearImageUsageflags();

  [[nodiscard]] int32_t GetXRes() const { return XRes; }
  [[nodiscard]] int32_t GetYRes() const { return YRes; }

  bool Resize(renderer::Device* Device, int32_t XSize, int32_t YSize);

 private:
  bool PackImage(CAtlasImage* img);

  void FlushCache();
  CAtlasNode* GetNodeCached(WBATLASHANDLE Handle);

  int32_t XRes, YRes;
  std::unique_ptr<uint8_t[]> Image;
  std::unique_ptr<renderer::Texture2D> Atlas;

  bool TextureUpdateNeeded;

  std::array<CAtlasCacheElement, kAtlasCacheSize> AtlasCache;

  std::unordered_map<WBATLASHANDLE, CAtlasNode*> Dictionary;
  std::unordered_map<WBATLASHANDLE, std::unique_ptr<CAtlasImage>> ImageStorage;

  std::unique_ptr<CAtlasNode> Root;

  CAtlasImage* WhitePixel;
  // recalculated on each optimization and reset
  math::Point WhitePixelPosition;

  std::mutex mtx;
};

}  // namespace gui
