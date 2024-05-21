#pragma once
#include"entity.hpp"
#include"component.h"
#include<array>
#include"mesh.h"


class GregoryPatch : public Component
{
public:
	GregoryPatch(Entity& owner);
	const char* Name() const override { return "GregoryPatch"; }
	std::array<Entity*, 3> neighbors;
	std::array<int, 3> neighborSide;
	std::array<bool, 3> neighborReverse;
	bool drawPolygon = false;

	Mesh* GetMesh();
	Mesh* GetWireMesh();

	Event<void>::Reaction NeighborModified;
private:
	bool modified = true, wireModified = true;

	std::unique_ptr<Mesh> m;
	std::unique_ptr<Mesh> wm;
};