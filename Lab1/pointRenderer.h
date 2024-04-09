#pragma once

#include"component.h"
#include"mesh.h"
#include"transform.h"

class PointRenderer : public Component
{
public:
	PointRenderer(Entity& owner);
	const char* Name() const override { return "PointRenderer"; };
	Transform& transform;
private:
};