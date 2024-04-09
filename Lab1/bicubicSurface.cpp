#include "bicubicSurface.h"
#include"catalogue.hpp"

BicubicSurface::BicubicSurface(Entity& owner)
	:Component(ComponentConstructorArgs(BicubicSurface))
{
}

void BicubicSurface::SetSegments(const std::vector<Entity*>& segs)
{
	assert(segments.size() == 0 && "You can only set once");
	for (int i = 0; i < segs.size(); i++)
		segments.push_back(segs[i]->GetComponent<BicubicSegment>());
}
