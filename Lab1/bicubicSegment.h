#pragma once
#include"component.h"
#include"pointCollection.h"
#include"vPointCollection.h"
#include"mesh.h"
#include"camera.h"
#include<array>

//class BicubicSegment;
//struct Edge
//{
//	const int index;
//	BicubicSegment* const parent;
//	Edge* from, * into;
//};
class BicubicSurface;
class BicubicSegment : public Component
{
public:
	BicubicSegment(Entity& owner);
	const char* Name() const override { return "BicubicSegment"; }
	//void Set(DirectX::XMINT2 start, int stride);
	std::array<int,16> indices;
	static const int corners[4];
	//  0    -y
	// 3 1 -x  +x
	//  2    +y
	std::array<Entity*, 4> neighbors;
	std::array<bool, 4> ccwNeighbor;
	std::array<int, 8> GetBoundary(int side);
	Mesh* GetMesh();
	Mesh* GetWireMesh();
	bool drawPolygon = false;
	bool deBoorMode = false;
	void SetSource(PointSource*);
	PointSource* GetSource() { return pointSource; };

	Event<void> onModified;
private:
	PointSource* pointSource = nullptr;
	BicubicSurface* surface = nullptr;
	bool modified = true, wireModified = true;
	Event<NeedRedrawEventData>::Reaction NeedsRedraw_PointCollection;
	Event<int>::Reaction PointDeleted;
	Event<DirectX::XMINT2>::Reaction TransferRelation;
	Event<Entity*>::Reaction ReleaseNeighbors;

	std::unique_ptr<Mesh> m;
	std::unique_ptr<Mesh> wm;
};