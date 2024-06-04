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
	Event<Entity*>::Reaction ReleaseNeighbors;

	int surfDetailOffset = 0;
private:
	static Event<void>::Reaction::Function NeighborModifiedFunction;
	static Event<Entity*>::Reaction::Function ReleaseNeighborsFunction;
	bool modified = true, wireModified = true;

	std::unique_ptr<Mesh> m;
	std::unique_ptr<Mesh> wm;
};