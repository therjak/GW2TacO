#pragma once
#include <unordered_map>

#include "../BaseLib/Dictionary.h"
#include "../CoRE2/Core2.h"

// cache size must be 2^x
#define ATLASCACHESIZE 32

typedef int32_t WBATLASHANDLE;

class CAtlasImage;

class CAtlasNode // stores a node for the rectpacker
{
  friend class CAtlas;
  CRect Area;
  CAtlasNode* Children[2];
  bool Occupied;

  CAtlasImage* Image;

public:
  CAtlasNode();
  virtual ~CAtlasNode();
  CAtlasNode* AddNode(int32_t width, int32_t height);
  CRect& GetArea();
  CAtlasImage* GetImage();
};

class CAtlasImage // stores image data not currently in the atlas
{
  uint8_t* Image;
  int32_t XRes, YRes;
  WBATLASHANDLE Handle;

  bool Required;

public:
  CAtlasImage();
  CAtlasImage(uint8_t* SourceImage, int32_t SrcXRes, int32_t SrcYRes,
              const CRect& Source);
  virtual ~CAtlasImage();

  WBATLASHANDLE GetHandle();
  uint8_t* GetImage();
  CSize GetSize() const;
  void TagRequired();
  void ClearRequired();
  bool IsRequired();
};

struct CAtlasCacheElement {
  WBATLASHANDLE Handle;
  CAtlasNode* Node;
};

class CAtlas {
  friend class CWBDrawAPI;
  int32_t XRes, YRes;
  uint8_t* Image;
  std::unique_ptr<CCoreTexture2D> Atlas;

  bool TextureUpdateNeeded;

  CAtlasCacheElement AtlasCache[ATLASCACHESIZE];

  std::unordered_map<WBATLASHANDLE, CAtlasNode*> Dictionary;
  std::unordered_map<WBATLASHANDLE, std::unique_ptr<CAtlasImage>> ImageStorage;

  CAtlasNode* Root;

  CAtlasImage* WhitePixel;
  CPoint WhitePixelPosition; // recalculated on each optimization and reset

  bool PackImage(CAtlasImage* img);

  void FlushCache();
  CAtlasNode* GetNodeCached(WBATLASHANDLE Handle);

  LIGHTWEIGHT_CRITICALSECTION critsec;

public:
  CAtlas(int32_t XSize, int32_t YSize);
  virtual ~CAtlas();

  bool InitializeTexture(CCoreDevice* Device);
  bool UpdateTexture();
  CCoreTexture2D* GetTexture();

  WBATLASHANDLE AddImage(uint8_t* Image, int32_t XRes, int32_t YRes,
                         const CRect& SourceArea);
  void
  DeleteImage(WBATLASHANDLE h); // doesn't immediately remove image from atlas

  bool Optimize(bool DebugMode = false);
  bool Reset();

  CSize GetSize(WBATLASHANDLE h);
  bool
  RequestImageUse(WBATLASHANDLE h,
                  CRect& UV); // returns false only if there was not enough room
                              // in the atlas to add the requested image
  CPoint GetWhitePixelUV();

  void ClearImageUsageflags();

  INLINE int32_t GetXRes() const { return XRes; }
  INLINE int32_t GetYRes() const { return YRes; }

  bool Resize(CCoreDevice* Device, int32_t XSize, int32_t YSize);
};
