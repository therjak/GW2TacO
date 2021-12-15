#pragma once
#include <cstdint>
#include <memory>

#include "resource.h"

#define COREBUFFERSCOPECOUNT 6

enum COREBUFFERSCOPE {
  CORE_BUFFER_SCENE = 0,
  CORE_BUFFER_LAYER,
  CORE_BUFFER_GAMEDATA,
  CORE_BUFFER_OBJECT,
  CORE_BUFFER_TECHSTATIC,
  CORE_BUFFER_TECHDYNAMIC
};

class CCoreConstantBuffer : public CCoreResource {
 protected:
  std::unique_ptr<uint8_t[]> Data;
  int32_t BufferLength = 0;
  int32_t DataLength = 0;

 public:
  explicit CCoreConstantBuffer(CCoreDevice* Device);
  ~CCoreConstantBuffer() override;

  void Reset();
  void AddData(void* Data, int32_t Length);
  virtual void Upload();
  virtual void* GetBufferPointer() const = 0;
};
