#pragma once
//////////////////////////////////////////////////////////////////////////
// every allocatable D3D resource is derived from this class
//
// on creation the resource adds itself to the main resource pool of the Api
// handler
//
// on destruction the resource removes itself from the resource pool
//
// on destruction of the Api handler the unfreed resources are deallocated
//
// only the constructor of a resource may add the resource to the
// Api handler, and this MUST be done for each created resource
//
// resource types that need to be reallocated on a lost device MUST
// implement the OnDeviceLost() and OnDeviceReset() functions
class CCoreDevice;

class CCoreResource {
  friend class CCoreDevice;
 public:
  CCoreResource();
  explicit CCoreResource(CCoreDevice* Device);
  virtual ~CCoreResource();

  virtual void OnDeviceLost();
  virtual void OnDeviceReset();

protected:
  CCoreDevice* Device = nullptr;

};
