#pragma once
#include"component.h"
#include"mesh.h"
#include"transform.h"

class MeshRenderer : public Component
{
public:
	MeshRenderer(Entity& owner);
	const char* Name() const override { return "MeshRenderer"; };
	const Mesh* mesh;
	ModelTransform& transform;
private:
};