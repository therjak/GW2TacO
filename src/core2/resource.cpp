#include "src/core2/resource.h"

#include "src/core2/device.h"

CCoreResource::CCoreResource() = default;

CCoreResource::CCoreResource(CCoreDevice* h) {
  Device = h;
  Device->AddResource(this);
}

CCoreResource::~CCoreResource() {
  if (Device) Device->RemoveResource(this);
}

void CCoreResource::OnDeviceLost() {}

void CCoreResource::OnDeviceReset() {}
