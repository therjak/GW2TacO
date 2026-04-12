#pragma once
#include <cstdint>
#include <string_view>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

namespace renderer {

enum class ExportImageFormat : uint8_t {
  kCorePng = 0,
  kCoreTga = 1,
  kCoreBmp = 2,
};

class CCoreTexture : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreTexture(CCoreDevice* device) : CCoreResource(device) {}
  // should remove this texture from the device render state here
  ~CCoreTexture() override = default;

 private:
  virtual bool SetToSampler(const CoreSampler sampler) = 0;
};

class CCoreTexture2D : public CCoreTexture {
 public:
  explicit CCoreTexture2D(CCoreDevice* device) : CCoreTexture(device) {
    x_res_ = y_res_ = 0;
    format_ = CoreFormat::kUnknown;
  }

  virtual bool Create(const int32_t x_res, const int32_t y_res,
                      const uint8_t* data, const char bytes_per_pixel = 4,
                      const CoreFormat format = CoreFormat::kA8R8G8B8,
                      const bool render_target = false) = 0;
  virtual bool Create(const uint8_t* data, int32_t const size) = 0;
  virtual bool CreateDepthBuffer(const int32_t x_res, const int32_t y_res,
                                 const int32_t ms_count) = 0;
  virtual bool Lock(void** result, int32_t& pitch) = 0;
  virtual bool UnLock() = 0;

  virtual bool Update(const uint8_t* data, const int32_t x_res,
                      const int32_t y_res, const char bytes_per_pixel = 4) = 0;

  virtual int32_t GetXRes() { return x_res_; }
  virtual int32_t GetYRes() { return y_res_; }

  virtual void ExportToImage(std::string_view filename, bool clear_alpha,
                             ExportImageFormat format, bool degamma) = 0;

 protected:
  int32_t x_res_, y_res_;
  CoreFormat format_;
};

class CCoreTexture3D : public CCoreTexture {
 public:
  explicit CCoreTexture3D(CCoreDevice* device) : CCoreTexture(device) {}
};

class CCoreTextureCube : public CCoreTexture {
 public:
  explicit CCoreTextureCube(CCoreDevice* device) : CCoreTexture(device) {}
};

}  // namespace renderer
