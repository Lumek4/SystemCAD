#include "bicubicSurface.h"
#include"catalogue.hpp"

void BicubicSurface::DeleteChildrenFunction(void* arg, Entity* e)
{
	auto surf = e->GetComponent<BicubicSurface>();
	for (int i = surf->segments.size() - 1; i >= 0; i--)
	{
		surf->segments[i]->owner.Delete();
	}
}
Event<Entity*>::Reaction BicubicSurface::DeleteChildren{ nullptr, BicubicSurface::DeleteChildrenFunction };

BicubicSurface::BicubicSurface(Entity& owner)
	:Component(ComponentConstructorArgs(BicubicSurface))
{
}

void BicubicSurface::SetSegments(const std::vector<Entity*>& segs)
{
	assert(segments.size() == 0 && "You can only set once");
	for (int i = 0; i < segs.size(); i++)
		segments.push_back(segs[i]->GetComponent<BicubicSegment>());
	owner.preDelete += DeleteChildren;
}
