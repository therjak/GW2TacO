#include "src/core2/resource.h"

#include "src/core2/device.h"

namespace renderer {

Resource::Resource() = default;

Resource::Resource(Device* device) {
  device_ = device;
  device_->AddResource(this);
}

Resource::~Resource() {
  if (device_) device_->RemoveResource(this);
}

void Resource::OnDeviceLost() {}

void Resource::OnDeviceReset() {}

}  // namespace renderer
