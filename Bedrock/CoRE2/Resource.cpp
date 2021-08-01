#include "Resource.h"

#include "Device.h"

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
