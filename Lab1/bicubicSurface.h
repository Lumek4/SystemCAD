#pragma once
#include"mesh.h"
#include"bicubicSegment.h"
#include<vector>


class BicubicSurface : public Component
{
public:
	BicubicSurface(Entity& owner);
	const char* Name() const override { return "BicubicSurface"; }

	DirectX::XMINT2 division;
	DirectX::XMINT2 wrapMode;

	void SetSegments(const std::vector<Entity*>& segs);
	std::vector<BicubicSegment*>& GetSegments()
	{
		return segments;
	}
	int surfDetailOffset = 0;
private:
	std::vector<BicubicSegment*> segments;
	static Event<Entity*>::Reaction::Function DeleteChildrenFunction;
	static Event<Entity*>::Reaction DeleteChildren;
};