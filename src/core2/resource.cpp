#include "src/core2/resource.h"

#include "src/core2/device.h"

namespace renderer {

CCoreResource::CCoreResource() = default;

CCoreResource::CCoreResource(CCoreDevice* device) {
  device_ = device;
  device_->AddResource(this);
}

CCoreResource::~CCoreResource() {
  if (device_) device_->RemoveResource(this);
}

void CCoreResource::OnDeviceLost() {}

void CCoreResource::OnDeviceReset() {}

}  // namespace renderer
