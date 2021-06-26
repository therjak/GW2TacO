#pragma once
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class CCoreMesh
{
	CCoreVertexFormat *VxFormat;
	CCoreVertexBuffer *Vertices;
	CCoreIndexBuffer *Indices;
	int32_t TriCount;
	int32_t VxCount;

public:

	CCoreMesh();
	virtual ~CCoreMesh();

	CCoreVertexFormat *GetVertexFormat();
	CCoreVertexBuffer *GetVertices();
	CCoreIndexBuffer *GetIndices();
	int32_t GetTriCount();
	int32_t GetVxCount();

	void SetVertexFormat(CCoreVertexFormat *v);
	void SetVertexBuffer(CCoreVertexBuffer *b, int32_t Count);
	void SetIndexBuffer(CCoreIndexBuffer *i, int32_t TriangleCount);

};

class CCoreModel
{

	CCoreMaterial *Material;
	CArray<CCoreMesh*> Meshes;

	CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*> Buffers[3]; //game, static, dynamic

public:

	CCoreModel();
	virtual ~CCoreModel();

	INLINE int32_t GetMeshCount() { return Meshes.NumItems(); }
	INLINE CCoreMesh *GetMesh(int32_t x) { return Meshes[x]; }
	virtual void AddMesh(CCoreMesh *m) { Meshes += m; }
	virtual void SetMaterial(CCoreMaterial *m) { Material = m; }

	virtual void UpdateData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers);
	void CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CArray<CCoreAtom*> &Atoms, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer);

};

