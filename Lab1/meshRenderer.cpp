#include "meshRenderer.h"
#include"entity.hpp"
#include<cassert>

MeshRenderer::MeshRenderer(Entity& owner)
	:Component(ComponentConstructorArgs(MeshRenderer)),
	transform(RequireComponent(ModelTransform)),
	mesh(nullptr)
{
}
