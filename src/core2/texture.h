#pragma once
#include <cstdint>
#include <string_view>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

namespace renderer {

enum class ExportImageFormat : uint8_t {
  kPng = 0,
  kTga = 1,
  kBmp = 2,
};

class Texture : public Resource {
  friend class Device;

 public:
  explicit Texture(Device* device) : Resource(device) {}
  // should remove this texture from the device render state here
  ~Texture() override = default;

 private:
  virtual bool SetToSampler(const Sampler sampler) = 0;
};

class Texture2D : public Texture {
 public:
  explicit Texture2D(Device* device) : Texture(device) {
    x_res_ = y_res_ = 0;
    format_ = Format::kUnknown;
  }

  virtual bool Create(const int32_t x_res, const int32_t y_res,
                      const uint8_t* data, const char bytes_per_pixel = 4,
                      const Format format = Format::kA8R8G8B8,
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
  Format format_;
};

class Texture3D : public Texture {
 public:
  explicit Texture3D(Device* device) : Texture(device) {}
};

class TextureCube : public Texture {
 public:
  explicit TextureCube(Device* device) : Texture(device) {}
};

}  // namespace renderer
