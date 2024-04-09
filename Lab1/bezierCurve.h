#pragma once

#include"component.h"
#include"mesh.h"
#include"pointSource.h"
#include"event.hpp"

class BezierCurve : public Component
{
public:
	BezierCurve(Entity& owner);
	const char* Name() const override { return "BezierCurve"; }
	Mesh* GetMesh();
	bool drawPolygon = false;

private:
	PointSource* pointSource;
	bool modified = true;
	Event<NeedRedrawEventData>::Reaction NeedsRedraw_PointCollection;

	std::unique_ptr<Mesh> m;
};