#pragma once
#include "Resource.h"

class CCoreVertexBuffer : public CCoreResource
{
	friend class CCoreDevice;
	virtual TBOOL Apply(const TU32 Offset) = 0;

public:

	INLINE CCoreVertexBuffer(CCoreDevice *Device) : CCoreResource(Device) {}

	virtual TBOOL Create(const TU8 *Data, const TU32 Size) = 0;
	virtual TBOOL CreateDynamic(const TU32 Size) = 0;
	virtual TBOOL Update(const int32_t Offset, const TU8 *Data, const TU32 Size) = 0;
	virtual TBOOL Lock(void **Result, const TU32 Offset, const int32_t size, const int32_t Flags) = 0;
	virtual TBOOL Lock(void **Result) = 0;
	virtual TBOOL UnLock() = 0;
	virtual void* GetHandle() = 0;
};
