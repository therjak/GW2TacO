#pragma once
#include "../BaseLib/BaseLib.h"

//////////////////////////////////////////////////////////////////////////
// this class contains data used by several objects at once
// its the main purpose is to set material properties like tint color or tint decay from the game for a number of objects at once

class CCoreObjectGroup
{
	TBOOL DataChanged;
	CDictionary<CString, float> Data;

public:

	CCoreObjectGroup();
	virtual ~CCoreObjectGroup();

	void Reset();
	void AddData(CString &Name, float Value);
	TBOOL GetData(CString &Name, float &Value);
	TBOOL HasDataChanged();
	void ClearDirtyFlag();

};
