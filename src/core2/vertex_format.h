#pragma once

#include <cstdint>
#include <vector>

#include "src/core2/enums.h"
#include "src/core2/resource.h"

namespace renderer {

class VertexShader;

class VertexFormat : public Resource {
  friend class Device;

 public:
  explicit VertexFormat(Device* device) : Resource(device) {}

  virtual bool Create(const std::vector<VertexAttribute>& attributes,
                      VertexShader* vertex_shader = nullptr) = 0;
  virtual int32_t GetSize() = 0;

 private:
  virtual bool Apply() = 0;
};

}  // namespace renderer
