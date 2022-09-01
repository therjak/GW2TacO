#pragma once
#include <cstdint>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

enum class EXPORTIMAGEFORMAT : uint8_t {
  CORE_PNG = 0,
  CORE_TGA = 1,
  CORE_BMP = 2,
};

class CCoreTexture : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreTexture(CCoreDevice* Device) : CCoreResource(Device) {}
  // should remove this texture from the device render state here
  ~CCoreTexture() override = default;

 private:
  virtual bool SetToSampler(const CORESAMPLER Sampler) = 0;
};

class CCoreTexture2D : public CCoreTexture {
 public:
  explicit CCoreTexture2D(CCoreDevice* Device) : CCoreTexture(Device) {
    XRes = YRes = 0;
    Format = COREFORMAT::UNKNOWN;
  }

  virtual bool Create(const int32_t XRes, const int32_t YRes,
                      const uint8_t* Data, const char BytesPerPixel = 4,
                      const COREFORMAT Format = COREFORMAT::A8R8G8B8,
                      const bool RenderTarget = false) = 0;
  virtual bool Create(const uint8_t* Data, int32_t const Size) = 0;
  virtual bool CreateDepthBuffer(const int32_t XRes, const int32_t YRes,
                                 const int32_t MSCount) = 0;
  virtual bool Lock(void** Result, int32_t& pitch) = 0;
  virtual bool UnLock() = 0;

  virtual bool Update(const uint8_t* Data, const int32_t XRes,
                      const int32_t YRes, const char BytesPerPixel = 4) = 0;

  virtual int32_t GetXRes() { return XRes; }
  virtual int32_t GetYRes() { return YRes; }

  virtual void ExportToImage(std::string_view Filename, bool ClearAlpha,
                             EXPORTIMAGEFORMAT Format, bool degamma) = 0;

 protected:
  int32_t XRes, YRes;
  COREFORMAT Format;
};

class CCoreTexture3D : public CCoreTexture {
 public:
  explicit CCoreTexture3D(CCoreDevice* Device) : CCoreTexture(Device) {}
};

class CCoreTextureCube : public CCoreTexture {
 public:
  explicit CCoreTextureCube(CCoreDevice* Device) : CCoreTexture(Device) {}
};
