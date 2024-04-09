#pragma once
#include<DirectXMath.h>
#include"event.hpp"

struct NeedRedrawEventData
{
	int ind = -1;
};

class PointSource
{
public:
	virtual DirectX::XMFLOAT3 GetPoint(int i) = 0;
	virtual int GetCount() = 0;
	virtual Event<NeedRedrawEventData>& NeedRedrawEvent() = 0;
};