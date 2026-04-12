#pragma once

#include <cstdint>
#include <vector>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

namespace renderer {

class CCoreVertexShader;

class CCoreVertexFormat : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreVertexFormat(CCoreDevice* device) : CCoreResource(device) {}

  virtual bool Create(const std::vector<CoreVertexAttribute>& attributes,
                      CCoreVertexShader* vertex_shader = nullptr) = 0;
  virtual int32_t GetSize() = 0;

 private:
  virtual bool Apply() = 0;
};

}  // namespace renderer
