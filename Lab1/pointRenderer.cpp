#include "pointRenderer.h"
#include"transform.h"
#include"entity.hpp"

PointRenderer::PointRenderer(Entity& owner)
	:Component(ComponentConstructorArgs(PointRenderer)),
	transform(RequireComponent(Transform))
{
}
