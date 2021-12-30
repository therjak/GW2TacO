#pragma once
#include <array>
#include <mutex>
#include <unordered_map>

#include "src/core2/core2.h"

// cache size must be 2^x
#define ATLASCACHESIZE 32

typedef int32_t WBATLASHANDLE;

class CAtlasImage;

// stores a node for the rectpacker
class CAtlasNode {
  friend class CAtlas;
  math::CRect Area;
  std::array<std::unique_ptr<CAtlasNode>, 2> Children;
  bool Occupied = false;

  CAtlasImage* Image = nullptr;

 public:
  CAtlasNode();
  virtual ~CAtlasNode();
  CAtlasNode* AddNode(int32_t width, int32_t height);
  math::CRect& GetArea();
  CAtlasImage* GetImage();
};

// stores image data not currently in the atlas
class CAtlasImage {
  std::unique_ptr<uint8_t[]> Image;
  int32_t XRes, YRes;
  WBATLASHANDLE Handle;

  bool Required;

 public:
  CAtlasImage();
  CAtlasImage(const uint8_t* SourceImage, int32_t SrcXRes, int32_t SrcYRes,
              const math::CRect& Source);
  virtual ~CAtlasImage();

  WBATLASHANDLE GetHandle();
  uint8_t* GetImage();
  math::CSize GetSize() const;
  void TagRequired();
  void ClearRequired();
  bool IsRequired();
};

struct CAtlasCacheElement {
  WBATLASHANDLE Handle = 0;
  CAtlasNode* Node = nullptr;
};

class CAtlas {
  friend class CWBDrawAPI;
  int32_t XRes, YRes;
  std::unique_ptr<uint8_t[]> Image;
  std::unique_ptr<CCoreTexture2D> Atlas;

  bool TextureUpdateNeeded;

  std::array<CAtlasCacheElement, ATLASCACHESIZE> AtlasCache;

  std::unordered_map<WBATLASHANDLE, CAtlasNode*> Dictionary;
  std::unordered_map<WBATLASHANDLE, std::unique_ptr<CAtlasImage>> ImageStorage;

  std::unique_ptr<CAtlasNode> Root;

  CAtlasImage* WhitePixel;
  // recalculated on each optimization and reset
  math::CPoint WhitePixelPosition;

  bool PackImage(CAtlasImage* img);

  void FlushCache();
  CAtlasNode* GetNodeCached(WBATLASHANDLE Handle);

  std::mutex mtx;

 public:
  CAtlas(int32_t XSize, int32_t YSize);
  virtual ~CAtlas();

  bool InitializeTexture(CCoreDevice* Device);
  bool UpdateTexture();
  CCoreTexture2D* GetTexture();

  WBATLASHANDLE AddImage(uint8_t* Image, int32_t XRes, int32_t YRes,
                         const math::CRect& SourceArea);
  // doesn't immediately remove image from atlas
  void DeleteImage(WBATLASHANDLE h);

  bool Optimize(bool DebugMode = false);
  bool Reset();

  math::CSize GetSize(WBATLASHANDLE h);
  // returns false only if there was not enough room
  // in the atlas to add the requested image
  bool RequestImageUse(WBATLASHANDLE h, math::CRect& UV);
  math::CPoint GetWhitePixelUV();

  void ClearImageUsageflags();

  int32_t GetXRes() const { return XRes; }
  int32_t GetYRes() const { return YRes; }

  bool Resize(CCoreDevice* Device, int32_t XSize, int32_t YSize);
};
