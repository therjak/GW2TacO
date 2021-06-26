#include "BasePCH.h"
#include "Model.h"
#include "Device.h"

void CCoreModel::UpdateData(CCoreDevice *Device, COREBUFFERSCOPE Scope, CCoreObjectGroup *Group, CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers)
{
	if (!Material) return;
	Material->GatherData(Device, Scope, Group, (CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*>**)Buffers, Layers);
}

void CCoreModel::CreateAtoms(CDictionaryEnumerable<CORERENDERLAYERID, CCoreRenderLayer*> &Layers, CArray<CCoreAtom*> &Atoms, CCoreConstantBuffer *SceneBuffer, CCoreConstantBuffer *ObjectBuffer)
{
	if (!Material) return;

	for (int32_t x = 0; x < Meshes.NumItems(); x++)
	{
		Material->CreateAtoms(Layers, SceneBuffer, ObjectBuffer, (CDictionary<CCoreMaterialTechnique*, CCoreConstantBuffer*>**)Buffers, Atoms, Meshes[x]);
	}
}

void CCoreMesh::SetIndexBuffer(CCoreIndexBuffer *i, int32_t TriangleCount)
{
	Indices = i;
	TriCount = TriangleCount;
}

void CCoreMesh::SetVertexBuffer(CCoreVertexBuffer *b, int32_t Count)
{
	Vertices = b;
	VxCount = Count;
}

void CCoreMesh::SetVertexFormat(CCoreVertexFormat *v)
{
	VxFormat = v;
}

int32_t CCoreMesh::GetVxCount()
{
	return VxCount;
}

int32_t CCoreMesh::GetTriCount()
{
	return TriCount;
}

CCoreIndexBuffer * CCoreMesh::GetIndices()
{
	return Indices;
}

CCoreVertexBuffer * CCoreMesh::GetVertices()
{
	return Vertices;
}

CCoreVertexFormat * CCoreMesh::GetVertexFormat()
{
	return VxFormat;
}
