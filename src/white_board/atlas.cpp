module;

#include <algorithm>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "src/base/logger.h"
#include "src/core2/core2.h"

module whiteboard;

import :atlas;
import math;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

static WBATLASHANDLE AtlasHandle = 1;

AtlasNode::AtlasNode() = default;

AtlasNode::~AtlasNode() = default;

Rect& AtlasNode::GetArea() { return area_; }

AtlasNode* AtlasNode::AddNode(int32_t width, int32_t height) {
  AtlasNode* NewNode = nullptr;
  if (children_[0]) {
    NewNode = children_[0]->AddNode(width, height);
    return NewNode ? NewNode : children_[1]->AddNode(width, height);
  }

  if (occupied_ || area_.Width() < width || area_.Height() < height) {
    return nullptr;
  }

  if (area_.Width() == width && area_.Height() == height) {
    occupied_ = true;
    return this;
  }

  children_[0] = std::make_unique<AtlasNode>();
  children_[1] = std::make_unique<AtlasNode>();

  if (area_.Width() - width > area_.Height() - height) {
    children_[0]->area_ = Rect(area_.x1, area_.y1, area_.x1 + width, area_.y2);
    children_[1]->area_ = Rect(area_.x1 + width, area_.y1, area_.x2, area_.y2);
  } else {
    children_[0]->area_ = Rect(area_.x1, area_.y1, area_.x2, area_.y1 + height);
    children_[1]->area_ = Rect(area_.x1, area_.y1 + height, area_.x2, area_.y2);
  }

  return children_[0]->AddNode(width, height);
}

AtlasImage* AtlasNode::GetImage() { return image_; }

AtlasImage::AtlasImage()
    : image_(nullptr),
      x_res_(0),
      y_res_(0),
      handle_(AtlasHandle++),
      required_(false) {}

AtlasImage::AtlasImage(const uint8_t* SourceImage, int32_t SrcXRes,
                       int32_t SrcYRes, const Rect& Source)
    : image_(nullptr),
      x_res_(Source.Width()),
      y_res_(Source.Height()),
      handle_(AtlasHandle++),
      required_(false) {
  if (Source.Area() > 0) {
    image_ = std::make_unique<uint8_t[]>(x_res_ * y_res_ * 4);
    memset(image_.get(), 0, x_res_ * y_res_ * 4);

    uint8_t* i = image_.get();

    for (int32_t y = 0; y < y_res_; y++) {
      if (y + Source.y1 < 0 || y + Source.y1 >= SrcYRes) {
        Log_Err("[gui] Atlas source image out of bounds, failed to add image");
        return;
      }

      for (int32_t x = 0; x < x_res_; x++) {
        if (x + Source.x1 < 0 || x + Source.x1 >= SrcXRes) {
          Log_Err(
              "[gui] Atlas source image out of bounds, failed to add image");
          return;
        }

        const int32_t k = (x + Source.x1 + (y + Source.y1) * SrcXRes) * 4;
        i[0] = SourceImage[k + 0];
        i[1] = SourceImage[k + 1];
        i[2] = SourceImage[k + 2];
        i[3] = SourceImage[k + 3];
        i += 4;
      }
    }
  }
}

AtlasImage::~AtlasImage() = default;

WBATLASHANDLE AtlasImage::GetHandle() { return handle_; }

uint8_t* AtlasImage::GetImage() { return image_.get(); }

Size AtlasImage::GetSize() const { return Size(x_res_, y_res_); }

void AtlasImage::TagRequired() { required_ = true; }

void AtlasImage::ClearRequired() { required_ = false; }

bool AtlasImage::IsRequired() { return required_; }

Atlas::Atlas(int32_t XSize, int32_t YSize)
    : x_res_(XSize),
      y_res_(YSize),
      image_(std::make_unique<uint8_t[]>(XSize * YSize * 4)),
      atlas_(nullptr),
      texture_update_needed_(false),
      root_(std::make_unique<AtlasNode>()) {
  FlushCache();
  memset(image_.get(), 0, x_res_ * y_res_ * 4);
  root_->area_ = Rect(0, 0, x_res_, y_res_);
  root_->occupied_ = false;

  uint8_t White[4 * 4];
  memset(&White[0], 0xff, 4 * 4);

  {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    auto img = std::make_unique<AtlasImage>(&White[0], 2, 2, Rect(0, 0, 2, 2));
    white_pixel_ = img.get();
    image_storage_[img->GetHandle()] = std::move(img);
  }

  Rect r;
  RequestImageUse(white_pixel_->GetHandle(), &r);
  white_pixel_position_ = r.TopLeft();
}

Atlas::~Atlas() {
  {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    image_storage_.clear();
  }
}

bool Atlas::PackImage(AtlasImage* img) {
  if (!img) return false;

  // Log_Dbg("Packing Image {:d}",img->GetHandle());

  FlushCache();
  const Size s = img->GetSize();

  AtlasNode* n = root_->AddNode(s.x, s.y);
  if (!n) {
    Log_Warn("[gui] Atlas full. Image can't be added.");
    return false;
  }

  uint8_t* target = image_.get() + (n->area_.x1 + n->area_.y1 * x_res_) * 4;
  uint8_t* source = img->GetImage();

  for (int32_t y = 0; y < s.y; y++) {
    memcpy(target, source, img->GetSize().x * 4);
    target += x_res_ * 4;
    source += img->GetSize().x * 4;
  }

  const WBATLASHANDLE Handle = img->GetHandle();

  n->image_ = img;
  dictionary_[Handle] = n;
  img->TagRequired();

  texture_update_needed_ = true;

  return img->GetHandle() != 0;
}

bool Atlas::InitializeTexture(renderer::Device* Device) {
  if (!Device) return false;
  atlas_ = Device->CreateTexture2D(x_res_, y_res_, image_.get());
  return atlas_.operator bool();
}

WBATLASHANDLE Atlas::AddImage(uint8_t* i, int32_t xs, int32_t ys,
                              const Rect& a) {
  if (a.Width() == 0 || a.Height() == 0) return 0;

  std::lock_guard<std::mutex> lockGuard(mtx_);

  auto img = std::make_unique<AtlasImage>(i, xs, ys, a);
  const auto h = img->GetHandle();
  image_storage_[img->GetHandle()] = std::move(img);
  return h;
}

bool Atlas::UpdateTexture() {
  // Log_Dbg("Atlas Texture Update Request");

  if (!texture_update_needed_) return true;

  // Log_Dbg("Updating Atlas Texture");

  if (!atlas_ || !atlas_->Update(image_.get(), x_res_, y_res_, 4)) return false;

  texture_update_needed_ = false;
  return true;
}

int SortImageStorage(AtlasImage* const& a, AtlasImage* const& b) {
  const Size ra = a->GetSize();
  const Size rb = b->GetSize();

  const int32_t w = rb.x - ra.x;
  const int32_t h = rb.y - ra.y;

  if (w != 0) return w;
  return h;
}

bool Atlas::Optimize(bool DebugMode) {
  // rearranges the atlas in a more optimal fashion and removes unused

  Log_Dbg("[gui] Optimizing Atlas");

  memset(image_.get(), 0, x_res_ * y_res_ * 4);

  if (DebugMode) {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    for (auto& x : image_storage_) {
      const auto handle = x.second->GetHandle();
      if (dictionary_.find(handle) != dictionary_.end()) {
        x.second->TagRequired();
      }
    }
  }

  root_ = std::make_unique<AtlasNode>();
  root_->area_ = Rect(0, 0, x_res_, y_res_);
  root_->occupied_ = false;

  dictionary_.clear();
  FlushCache();

  {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    int32_t RequiredCount = 0;
    for (auto& x : image_storage_) {
      RequiredCount += x.second->IsRequired() ? 1 : 0;
    }

    white_pixel_->TagRequired();

    if (RequiredCount) {
      std::vector<AtlasImage*> images;
      for (auto& x : image_storage_) {
        images.push_back(x.second.get());
      }
      std::sort(images.begin(), images.end(),
                [](const AtlasImage* a, const AtlasImage* b) {
                  const Size ra = a->GetSize();
                  const Size rb = b->GetSize();
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
      if (!PackImage(white_pixel_)) return false;
    }
  }

  Rect r;
  RequestImageUse(white_pixel_->GetHandle(), &r);
  white_pixel_position_ = r.TopLeft() + Point(1, 1);

  texture_update_needed_ = true;
  return true;
}

void Atlas::DeleteImage(const WBATLASHANDLE h) {
  {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    image_storage_.erase(h);
  }

  AtlasNode* n = GetNodeCached(h);
  if (!n) return;
  n->image_ = nullptr;
  dictionary_.erase(h);
  FlushCache();
  return;
}

renderer::Texture2D* Atlas::GetTexture() { return atlas_.get(); }

Size Atlas::GetSize(WBATLASHANDLE h) {
  AtlasNode* n = GetNodeCached(h);
  if (n) return n->GetArea().Size();

  {
    std::lock_guard<std::mutex> lockGuard(mtx_);
    auto it = image_storage_.find(h);
    if (it != image_storage_.end()) return it->second->GetSize();
  }

  return Size(0, 0);
}

bool Atlas::RequestImageUse(WBATLASHANDLE h, Rect* r) {
  if (!h) {
    *r = Rect(0, 0, 0, 0);
    return true;
  }

  AtlasNode* n = GetNodeCached(h);

  if (!n) {
    // image not on atlas, add it
    {
      std::lock_guard<std::mutex> lockGuard(mtx_);
      auto it = image_storage_.find(h);
      if (it != image_storage_.end()) {
        if (PackImage(it->second.get())) {
          n = GetNodeCached(h);
        }
      }
    }

    if (!n) {
      *r = Rect(0, 0, 0, 0);
      return false;
    }
  }

  // need to tag image as required HERE
  if (n->GetImage()) {
    n->GetImage()->TagRequired();
  }
  *r = n->area_;

  return true;
}

Point Atlas::GetWhitePixelUV() { return white_pixel_position_; }

void Atlas::ClearImageUsageflags() {
  std::lock_guard<std::mutex> lockGuard(mtx_);
  for (auto& x : image_storage_) x.second->ClearRequired();
}

void Atlas::FlushCache() {
  for (auto& x : atlas_cache_) {
    x = AtlasCacheElement{};
  }
}

AtlasNode* Atlas::GetNodeCached(WBATLASHANDLE Handle) {
  const int32_t idx = Handle & (kAtlasCacheSize - 1);

  if (atlas_cache_[idx].Handle == Handle) return atlas_cache_[idx].Node;

  AtlasNode* n = nullptr;
  auto it = dictionary_.find(Handle);
  if (it != dictionary_.end()) {
    n = it->second;
  }

  atlas_cache_[idx].Handle = Handle;
  atlas_cache_[idx].Node = n;

  return n;
}

bool Atlas::Reset() {
  root_ = std::make_unique<AtlasNode>();
  root_->area_ = Rect(0, 0, x_res_, y_res_);
  root_->occupied_ = false;

  dictionary_.clear();
  FlushCache();
  white_pixel_->TagRequired();

  if (!PackImage(white_pixel_)) return false;

  Rect r;
  RequestImageUse(white_pixel_->GetHandle(), &r);
  white_pixel_position_ = r.TopLeft() + Point(1, 1);

  texture_update_needed_ = true;
  return true;
}

bool Atlas::Resize(renderer::Device* Device, int32_t XSize, int32_t YSize) {
  root_.reset();
  image_.reset();
  atlas_.reset();

  FlushCache();
  x_res_ = XSize;
  y_res_ = YSize;

  image_ = std::make_unique<uint8_t[]>(x_res_ * y_res_ * 4);
  memset(image_.get(), 0, x_res_ * y_res_ * 4);

  root_ = std::make_unique<AtlasNode>();
  root_->area_ = Rect(0, 0, x_res_, y_res_);
  root_->occupied_ = false;

  dictionary_.clear();
  white_pixel_->TagRequired();

  if (!InitializeTexture(Device)) return false;

  if (!PackImage(white_pixel_)) return false;

  Rect r;
  RequestImageUse(white_pixel_->GetHandle(), &r);
  white_pixel_position_ = r.TopLeft() + Point(1, 1);

  return true;
}

}  // namespace gui
