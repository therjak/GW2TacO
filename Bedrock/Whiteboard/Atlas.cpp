#include "Atlas.h"

static WBATLASHANDLE AtlasHandle = 1;

CAtlasNode::CAtlasNode() {
  Children[0] = nullptr;
  Children[1] = nullptr;
  Occupied = false;
  Image = nullptr;
}

CAtlasNode::~CAtlasNode() {
  if (Children[0]) SAFEDELETE(Children[0]);
  if (Children[1]) SAFEDELETE(Children[1]);
}

CRect &CAtlasNode::GetArea() { return Area; }

CAtlasNode *CAtlasNode::AddNode(int32_t width, int32_t height) {
  CAtlasNode *NewNode;
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

  Children[0] = new CAtlasNode();
  Children[1] = new CAtlasNode();

  if (Area.Width() - width > Area.Height() - height) {
    Children[0]->Area = CRect(Area.x1, Area.y1, Area.x1 + width, Area.y2);
    Children[1]->Area = CRect(Area.x1 + width, Area.y1, Area.x2, Area.y2);
  } else {
    Children[0]->Area = CRect(Area.x1, Area.y1, Area.x2, Area.y1 + height);
    Children[1]->Area = CRect(Area.x1, Area.y1 + height, Area.x2, Area.y2);
  }

  return Children[0]->AddNode(width, height);
}

CAtlasImage *CAtlasNode::GetImage() { return Image; }

CAtlasImage::CAtlasImage() {
  Image = nullptr;
  XRes = YRes = 0;
  Handle = AtlasHandle++;
  Required = false;
}

CAtlasImage::CAtlasImage(uint8_t *SourceImage, int32_t SrcXRes, int32_t SrcYRes,
                         const CRect &Source) {
  Image = nullptr;
  XRes = Source.Width();
  YRes = Source.Height();
  Handle = AtlasHandle++;
  Required = false;

  if (Source.Area() > 0) {
    Image = new uint8_t[XRes * YRes * 4];
    memset(Image, 0, XRes * YRes * 4);

    uint8_t *i = Image;

    for (int32_t y = 0; y < YRes; y++) {
      if (y + Source.y1 < 0 || y + Source.y1 >= SrcYRes) {
        LOG(LOG_ERROR,
            _T( "[gui] Atlas source image out of bounds, failed to add image" ));
        return;
      }

      for (int32_t x = 0; x < XRes; x++) {
        if (x + Source.x1 < 0 || x + Source.x1 >= SrcXRes) {
          LOG(LOG_ERROR,
              _T( "[gui] Atlas source image out of bounds, failed to add image" ));
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

CAtlasImage::~CAtlasImage() { SAFEDELETEA(Image); }

WBATLASHANDLE CAtlasImage::GetHandle() { return Handle; }

uint8_t *CAtlasImage::GetImage() { return Image; }

CSize CAtlasImage::GetSize() const { return CSize(XRes, YRes); }

void CAtlasImage::TagRequired() { Required = true; }

void CAtlasImage::ClearRequired() { Required = false; }

bool CAtlasImage::IsRequired() { return Required; }

CAtlas::CAtlas(int32_t XSize, int32_t YSize) {
  FlushCache();
  XRes = XSize;
  YRes = YSize;
  Image = new uint8_t[XRes * YRes * 4];
  memset(Image, 0, XRes * YRes * 4);
  Root = new CAtlasNode();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;
  Atlas = nullptr;
  TextureUpdateNeeded = false;

  int32_t White[4];
  memset(White, 0xff, 4 * 4);

  {
    CLightweightCriticalSection cs(&critsec);
    CAtlasImage *img = new CAtlasImage(reinterpret_cast<uint8_t *>(&White), 2,
                                       2, CRect(0, 0, 2, 2));
    WhitePixel = ImageStorage[img->GetHandle()] = img;
  }

  CRect r;
  RequestImageUse(WhitePixel->GetHandle(), r);
  WhitePixelPosition = r.TopLeft();
}

CAtlas::~CAtlas() {
  {
    CLightweightCriticalSection cs(&critsec);
    for (int x = 0; x < ImageStorage.NumItems(); x++)
      delete ImageStorage.GetByIndex(x);
  }
  SAFEDELETE(Root);
  SAFEDELETEA(Image);
}

bool CAtlas::PackImage(CAtlasImage *img) {
  if (!img) return false;

  // LOG(LOG_DEBUG,_T("Packing Image %d"),img->GetHandle());

  FlushCache();
  CSize s = img->GetSize();

  CAtlasNode *n = Root->AddNode(s.x, s.y);
  if (!n) {
    LOG(LOG_WARNING, _T( "[gui] Atlas full. Image can't be added." ));
    return 0;
  }

  uint8_t *target = Image + (n->Area.x1 + n->Area.y1 * XRes) * 4;
  uint8_t *source = img->GetImage();

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

bool CAtlas::InitializeTexture(CCoreDevice *Device) {
  if (!Device) return false;
  Atlas.swap(Device->CreateTexture2D(XRes, YRes, Image));
  return Atlas.operator bool();
}

WBATLASHANDLE CAtlas::AddImage(uint8_t *i, int32_t xs, int32_t ys,
                               const CRect &a) {
  if (a.Width() == 0 || a.Height() == 0) return 0;

  CLightweightCriticalSection cs(&critsec);

  CAtlasImage *img = new CAtlasImage(i, xs, ys, a);
  ImageStorage[img->GetHandle()] = img;
  return img->GetHandle();
}

bool CAtlas::UpdateTexture() {
  // LOG_DBG("Atlas Texture Update Request");

  if (!TextureUpdateNeeded) return true;

  // LOG(LOG_DEBUG,_T("Updating Atlas Texture"));

  if (!Atlas || !Atlas->Update(Image, XRes, YRes, 4)) return false;

  TextureUpdateNeeded = false;
  return true;
}

int SortImageStorage(CAtlasImage *const &a, CAtlasImage *const &b) {
  CSize ra = a->GetSize();
  CSize rb = b->GetSize();

  int32_t w = rb.x - ra.x;
  int32_t h = rb.y - ra.y;

  if (w != 0) return w;
  return h;
}

bool CAtlas::Optimize(bool DebugMode) {
  // rearranges the atlas in a more optimal fashion and removes unused

  LOG(LOG_DEBUG, _T( "[gui] Optimizing Atlas" ));

  memset(Image, 0, XRes * YRes * 4);

  if (DebugMode) {
    CLightweightCriticalSection cs(&critsec);
    for (int32_t x = 0; x < ImageStorage.NumItems(); x++) {
      auto handle = ImageStorage.GetByIndex(x)->GetHandle();
      if (Dictionary.find(handle) != Dictionary.end()) {
        ImageStorage.GetByIndex(x)->TagRequired();
      }
    }
  }

  SAFEDELETE(Root);
  Root = new CAtlasNode();
  Root->Area = CRect(0, 0, XRes, YRes);
  Root->Occupied = false;

  Dictionary.clear();
  FlushCache();

  {
    CLightweightCriticalSection cs(&critsec);
    int32_t RequiredCount = 0;
    for (int32_t x = 0; x < ImageStorage.NumItems(); x++)
      RequiredCount += ImageStorage.GetByIndex(x)->IsRequired();

    WhitePixel->TagRequired();

    if (RequiredCount) {
      ImageStorage.SortByValue(SortImageStorage);
      for (int32_t x = 0; x < ImageStorage.NumItems(); x++) {
        if (ImageStorage.GetByIndex(x)->IsRequired())
          if (!PackImage(ImageStorage.GetByIndex(x))) return false;
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
    if (ImageStorage.HasKey(h)) {
      SAFEDELETE(ImageStorage[h]);
      ImageStorage.Delete(h);
    }
  }

  CAtlasNode *n = GetNodeCached(h);
  if (!n) return;
  n->Image = nullptr;
  Dictionary.erase(h);
  FlushCache();
  return;
}

CCoreTexture2D *CAtlas::GetTexture() { return Atlas.get(); }

CSize CAtlas::GetSize(WBATLASHANDLE h) {
  CAtlasNode *n = GetNodeCached(h);
  if (n) return n->GetArea().Size();

  {
    CLightweightCriticalSection cs(&critsec);
    if (ImageStorage.HasKey(h)) return ImageStorage[h]->GetSize();
  }

  return CSize(0, 0);
}

bool CAtlas::RequestImageUse(WBATLASHANDLE h, CRect &r) {
  if (!h) {
    r = CRect(0, 0, 0, 0);
    return true;
  }

  CAtlasNode *n = GetNodeCached(h);

  if (!n)  // image not on atlas, add it
  {
    {
      CLightweightCriticalSection cs(&critsec);
      if (ImageStorage.HasKey(h)) {
        if (PackImage(ImageStorage[h])) n = GetNodeCached(h);
      }
    }

    if (!n) {
      r = CRect(0, 0, 0, 0);
      return false;
    }
  }

  // need to tag image as required HERE
  if (n->GetImage()) n->GetImage()->TagRequired();

  r = n->Area;

  return true;
}

CPoint CAtlas::GetWhitePixelUV() { return WhitePixelPosition; }

void CAtlas::ClearImageUsageflags() {
  CLightweightCriticalSection cs(&critsec);
  for (int32_t x = 0; x < ImageStorage.NumItems(); x++)
    ImageStorage.GetByIndex(x)->ClearRequired();
}

void CAtlas::FlushCache() { memset(AtlasCache, 0, sizeof(AtlasCache)); }

CAtlasNode *CAtlas::GetNodeCached(WBATLASHANDLE Handle) {
  int32_t idx = Handle & (ATLASCACHESIZE - 1);

  if (AtlasCache[idx].Handle == Handle) return AtlasCache[idx].Node;

  CAtlasNode *n = nullptr;
  auto it = Dictionary.find(Handle);
  if (it != Dictionary.end()) {
    n = it->second;
  }

  AtlasCache[idx].Handle = Handle;
  AtlasCache[idx].Node = n;

  return n;
}

bool CAtlas::Reset() {
  SAFEDELETE(Root);
  Root = new CAtlasNode();
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

bool CAtlas::Resize(CCoreDevice *Device, int32_t XSize, int32_t YSize) {
  SAFEDELETE(Root);
  SAFEDELETEA(Image);
  Atlas.reset();

  FlushCache();
  XRes = XSize;
  YRes = YSize;

  Image = new uint8_t[XRes * YRes * 4];
  memset(Image, 0, XRes * YRes * 4);

  Root = new CAtlasNode();
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
