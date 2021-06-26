#pragma once
#include "DX11Device.h"
#include "VertexBuffer.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexBuffer : public CCoreVertexBuffer
{
	ID3D11Device *Dev;
	ID3D11DeviceContext *DeviceContext;
	ID3D11Buffer *VertexBufferHandle;

	int32_t Size;
	TBOOL Dynamic;

	virtual void Release();
	virtual TBOOL Apply(const uint32_t Offset);

public:

	CCoreDX11VertexBuffer(CCoreDX11Device *Device);
	virtual ~CCoreDX11VertexBuffer();

	virtual TBOOL Create(const TU8 *Data, const uint32_t Size);
	virtual TBOOL CreateDynamic(const uint32_t Size);
	virtual TBOOL Update(const int32_t Offset, const TU8 *Data, const uint32_t Size);
	virtual TBOOL Lock(void **Result);
	virtual TBOOL Lock(void **Result, const uint32_t Offset, const int32_t size, const int32_t Flags = 0);
	virtual TBOOL UnLock();
	virtual void* GetHandle() { return VertexBufferHandle; }
};

#endif
