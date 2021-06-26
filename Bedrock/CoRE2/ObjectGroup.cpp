#include "BasePCH.h"
#include "ObjectGroup.h"

CCoreObjectGroup::CCoreObjectGroup()
{
	DataChanged = true;
}

CCoreObjectGroup::~CCoreObjectGroup()
{

}

void CCoreObjectGroup::Reset()
{
	Data.Flush();
}

void CCoreObjectGroup::AddData(CString &Name, float Value)
{
	Data[Name] = Value;
	DataChanged = true;
}

TBOOL CCoreObjectGroup::GetData(CString &Name, float &Value)
{
	if (!Data.HasKey(Name)) return false;
	Value = Data[Name];
	return true;
}

void CCoreObjectGroup::ClearDirtyFlag()
{
	DataChanged = false;
}

TBOOL CCoreObjectGroup::HasDataChanged()
{
	return DataChanged;
}

