#pragma once
#include "Resource.h"

class CCoreIndexBuffer : public CCoreResource
{
	friend class CCoreDevice;
	virtual TBOOL Apply() = 0;

public:

	INLINE CCoreIndexBuffer(CCoreDevice *Device) : CCoreResource(Device) {}

	virtual TBOOL Create(const uint32_t IndexCount, const uint32_t IndexSize = 2) = 0;
	virtual TBOOL Lock(void **Result, const uint32_t IndexOffset, const int32_t IndexCount) = 0;
	virtual TBOOL Lock(void **Result) = 0;
	virtual TBOOL UnLock() = 0;
	virtual void* GetHandle() = 0;
};
