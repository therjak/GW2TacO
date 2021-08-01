#include "Atlas.h"

#include <algorithm>

static WBATLASHANDLE AtlasHandle = 1;

CAtlasNode::CAtlasNode() = default;

CAtlasNode::~CAtlasNode() = default;

CRect& CAtlasNode::GetArea() { return Area; }

CAtlasNode* CAtlasNode::AddNode(int32_t width, int32_t height) {
  CAtlasNode* NewNode;
  if (Children[0]) {
    NewNode = Children[0]->AddNode(width, height);
    return NewNode ? NewNode : Children[1]->AddNode(width, height);
  }

  if (Occupied || Area.Width() < width || Area.Height() < height)
    return nullptr;

  if (Area.Width() == width && Area.Height() == height) {
    Occupied = true;
    return this;
  }

  Children[0] = std::make_unique<CAtlasNode>();
  Children[1] = std::make_unique<CAtlasNode>();

  if (Area.Width() - width > Area.Height() - height) {
    Children[0]->Area = CRect(Area.x1, Area.y1, Area.x1 + width, Area.y2);
    Children[1]->Area = CRect(Area.x1 + width, Area.y1, Area.x2, Area.y2);
  } else {
    Children[0]->Area = CRect(Area.x1, Area.y1, Area.x2, Area.y1 + height);
    Children[1]->Area = CRect(Area.x1, Area.y1 + height, Area.x2, Area.y2);
  }

  return Children[0]->AddNode(width, height);
}

CAtlasImage* CAtlasNode::GetImage() { return Image; }

CAtlasImage::CAtlasImage() {
  Image = nullptr;
  XRes = YRes = 0;
  Handle = AtlasHandle++;
  Required = false;
}

CAtlasImage::CAtlasImage(const uint8_t* SourceImage, int32_t SrcXRes,
                         int32_t SrcYRes, const CRect& Source) {
  Image = nullptr;
  XRes = Source.Width();
  YRes = Source.Height();
  Handle = AtlasHandle++;
  Required = false;

  if (Source.Area() > 0) {
    Image = std::make_unique<uint8_t[]>(XRes * YRes * 4);
    memset(Image.get(), 0, XRes * YRes * 4);

    uint8_t* i = Image.get();

    for (int32_t y = 0; y < YRes; y++) {
      if (y + Source.y1 < 0 || y + Source.y1 >= SrcYRes) {
        LOG_ERR("[gui] Atlas source image out of bounds, failed to add image");
        return;
      }

      for (int32_t x = 0; x < XRes; x++) {
        if (x + Source.x1 < 0 || x + Source.x1 >= SrcXRes) {
          LOG_ERR(
              "[gui] Atlas source image out of bounds, failed to add image");
          return;
        }

        int32_t k = (x + Source.x1 + (y + Source.y1) * SrcXRes) * 4;
        i[0] = SourceImage[k + 0];
        i[1] = SourceImage[k + 1];
        i[2] = SourceImage[k + 2];
        i[3] = SourceImage[k + 3];
        i += 4;
      }
    }
  }
}

CAtlasImage::~CAtlasImage() = default;

WBATLASHANDLE CAtlasImage::GetHandle() { return Handle; }

uint8_t* CAtlasImage::GetImage() { return Image.get(); }

CSize CAtlasImage::GetSize() const { return CSize(XRes, YRes); }

void CAtlasImage::TagRequired() { Required = true; }

void CAtlasImage::ClearRequired() { Required = false; }

bool CAtlasImage::IsRequired() { return Required; }

CAtlas::CAtlas(int32_t XSize, int32_t YSize) {
  FlushCache();
  XRes = XSize;
  YRes = YSize;
  Image = std::make_unique<uint8_t[]>(XRes * YRes * 4);
  memset(Image.get(), 0, XRes * YRes * 4);
  Root = std::make_unique<CAtlasNode>();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;
  Atlas = nullptr;
  TextureUpdateNeeded = false;

  int32_t White[4];
  memset(White, 0xff, 4 * 4);

  {
    CLightweightCriticalSection cs(&critsec);
    auto img = std::make_unique<CAtlasImage>(reinterpret_cast<uint8_t*>(&White),
                                             2, 2, CRect(0, 0, 2, 2));
    WhitePixel = img.get();
    ImageStorage[img->GetHandle()] = std::move(img);
  }

  CRect r;
  RequestImageUse(WhitePixel->GetHandle(), r);
  WhitePixelPosition = r.TopLeft();
}

CAtlas::~CAtlas() {
  {
    CLightweightCriticalSection cs(&critsec);
    ImageStorage.clear();
  }
}

bool CAtlas::PackImage(CAtlasImage* img) {
  if (!img) return false;

  // LOG(LOG_DEBUG,_T("Packing Image %d"),img->GetHandle());

  FlushCache();
  CSize s = img->GetSize();

  CAtlasNode* n = Root->AddNode(s.x, s.y);
  if (!n) {
    LOG_WARN("[gui] Atlas full. Image can't be added.");
    return 0;
  }

  uint8_t* target = Image.get() + (n->Area.x1 + n->Area.y1 * XRes) * 4;
  uint8_t* source = img->GetImage();

  for (int32_t y = 0; y < s.y; y++) {
    memcpy(target, source, img->GetSize().x * 4);
    target += XRes * 4;
    source += img->GetSize().x * 4;
  }

  WBATLASHANDLE Handle = img->GetHandle();

  n->Image = img;
  Dictionary[Handle] = n;
  img->TagRequired();

  TextureUpdateNeeded = true;

  return img->GetHandle() != 0;
}

bool CAtlas::InitializeTexture(CCoreDevice* Device) {
  if (!Device) return false;
  Atlas = Device->CreateTexture2D(XRes, YRes, Image.get());
  return Atlas.operator bool();
}

WBATLASHANDLE CAtlas::AddImage(uint8_t* i, int32_t xs, int32_t ys,
                               const CRect& a) {
  if (a.Width() == 0 || a.Height() == 0) return 0;

  CLightweightCriticalSection cs(&critsec);

  auto img = std::make_unique<CAtlasImage>(i, xs, ys, a);
  auto h = img->GetHandle();
  ImageStorage[img->GetHandle()] = std::move(img);
  return h;
}

bool CAtlas::UpdateTexture() {
  // LOG_DBG("Atlas Texture Update Request");

  if (!TextureUpdateNeeded) return true;

  // LOG(LOG_DEBUG,_T("Updating Atlas Texture"));

  if (!Atlas || !Atlas->Update(Image.get(), XRes, YRes, 4)) return false;

  TextureUpdateNeeded = false;
  return true;
}

int SortImageStorage(CAtlasImage* const& a, CAtlasImage* const& b) {
  CSize ra = a->GetSize();
  CSize rb = b->GetSize();

  int32_t w = rb.x - ra.x;
  int32_t h = rb.y - ra.y;

  if (w != 0) return w;
  return h;
}

bool CAtlas::Optimize(bool DebugMode) {
  // rearranges the atlas in a more optimal fashion and removes unused

  LOG_DBG("[gui] Optimizing Atlas");

  memset(Image.get(), 0, XRes * YRes * 4);

  if (DebugMode) {
    CLightweightCriticalSection cs(&critsec);
    for (auto& x : ImageStorage) {
      auto handle = x.second->GetHandle();
      if (Dictionary.find(handle) != Dictionary.end()) {
        x.second->TagRequired();
      }
    }
  }

  Root = std::make_unique<CAtlasNode>();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;

  Dictionary.clear();
  FlushCache();

  {
    CLightweightCriticalSection cs(&critsec);
    int32_t RequiredCount = 0;
    for (auto& x : ImageStorage)
      RequiredCount += x.second->IsRequired() ? 1 : 0;

    WhitePixel->TagRequired();

    if (RequiredCount) {
      std::vector<CAtlasImage*> images;
      for (auto& x : ImageStorage) {
        images.push_back(x.second.get());
      }
      std::sort(images.begin(), images.end(),
                [](const CAtlasImage* a, const CAtlasImage* b) {
                  CSize ra = a->GetSize();
                  CSize rb = b->GetSize();
                  // As sort creates an ascending order and we want the largest
                  // first, return true if a > b.
                  if (ra.x != rb.x) return ra.x > rb.x;
                  return ra.y > rb.y;
                });

      for (auto& x : images) {
        if (x->IsRequired()) {
          if (!PackImage(x)) {
            return false;
          }
        }
      }
    } else {
      if (!PackImage(WhitePixel)) return false;
    }
  }

  CRect r;
  RequestImageUse(WhitePixel->GetHandle(), r);
  WhitePixelPosition = r.TopLeft() + CPoint(1, 1);

  TextureUpdateNeeded = true;
  return true;
}

void CAtlas::DeleteImage(const WBATLASHANDLE h) {
  {
    CLightweightCriticalSection cs(&critsec);
    ImageStorage.erase(h);
  }

  CAtlasNode* n = GetNodeCached(h);
  if (!n) return;
  n->Image = nullptr;
  Dictionary.erase(h);
  FlushCache();
  return;
}

CCoreTexture2D* CAtlas::GetTexture() { return Atlas.get(); }

CSize CAtlas::GetSize(WBATLASHANDLE h) {
  CAtlasNode* n = GetNodeCached(h);
  if (n) return n->GetArea().Size();

  {
    CLightweightCriticalSection cs(&critsec);
    auto it = ImageStorage.find(h);
    if (it != ImageStorage.end()) return it->second->GetSize();
  }

  return CSize(0, 0);
}

bool CAtlas::RequestImageUse(WBATLASHANDLE h, CRect& r) {
  if (!h) {
    r = CRect(0, 0, 0, 0);
    return true;
  }

  CAtlasNode* n = GetNodeCached(h);

  if (!n) {
    // image not on atlas, add it
    {
      CLightweightCriticalSection cs(&critsec);
      auto it = ImageStorage.find(h);
      if (it != ImageStorage.end()) {
        if (PackImage(it->second.get())) {
          n = GetNodeCached(h);
        }
      }
    }

    if (!n) {
      r = CRect(0, 0, 0, 0);
      return false;
    }
  }

  // need to tag image as required HERE
  if (n->GetImage()) {
    n->GetImage()->TagRequired();
  }
  r = n->Area;

  return true;
}

CPoint CAtlas::GetWhitePixelUV() { return WhitePixelPosition; }

void CAtlas::ClearImageUsageflags() {
  CLightweightCriticalSection cs(&critsec);
  for (auto& x : ImageStorage) x.second->ClearRequired();
}

void CAtlas::FlushCache() {
  for (auto& x : AtlasCache) {
    x = CAtlasCacheElement{};
  }
}

CAtlasNode* CAtlas::GetNodeCached(WBATLASHANDLE Handle) {
  int32_t idx = Handle & (ATLASCACHESIZE - 1);

  if (AtlasCache[idx].Handle == Handle) return AtlasCache[idx].Node;

  CAtlasNode* n = nullptr;
  auto it = Dictionary.find(Handle);
  if (it != Dictionary.end()) {
    n = it->second;
  }

  AtlasCache[idx].Handle = Handle;
  AtlasCache[idx].Node = n;

  return n;
}

bool CAtlas::Reset() {
  Root = std::make_unique<CAtlasNode>();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;

  Dictionary.clear();
  FlushCache();
  WhitePixel->TagRequired();

  if (!PackImage(WhitePixel)) return false;

  CRect r;
  RequestImageUse(WhitePixel->GetHandle(), r);
  WhitePixelPosition = r.TopLeft() + CPoint(1, 1);

  TextureUpdateNeeded = true;
  return true;
}

bool CAtlas::Resize(CCoreDevice* Device, int32_t XSize, int32_t YSize) {
  Root.reset();
  Image.reset();
  Atlas.reset();

  FlushCache();
  XRes = XSize;
  YRes = YSize;

  Image = std::make_unique<uint8_t[]>(XRes * YRes * 4);
  memset(Image.get(), 0, XRes * YRes * 4);

  Root = std::make_unique<CAtlasNode>();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;

  Dictionary.clear();
  WhitePixel->TagRequired();

  if (!InitializeTexture(Device)) return false;

  if (!PackImage(WhitePixel)) return false;

  CRect r;
  RequestImageUse(WhitePixel->GetHandle(), r);
  WhitePixelPosition = r.TopLeft() + CPoint(1, 1);

  return true;
}
