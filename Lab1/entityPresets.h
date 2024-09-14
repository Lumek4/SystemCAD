#pragma once
#include"entity.hpp"
#include"mesh.h"
#include<DirectXMath.h>
#include<vector>

struct BicubicSurfaceParams
{
	bool deBoor = false;
	DirectX::XMINT2 division{ 2,2 };
	DirectX::XMINT2 wrapMode{ 0,0 };
	DirectX::XMFLOAT2 dimensions{ 3,4 };
};
namespace EntityPresets
{
	Entity* Torus(DirectX::XMFLOAT3 position);
	Entity* Model(DirectX::XMFLOAT3 position, Mesh* mesh);
	Entity* Point(DirectX::XMFLOAT3 position);
	Entity* BezierCurveObject(const std::vector<Entity*>& selection);
	Entity* SplineCurve(const std::vector<Entity*>& selection);
	Entity* InterpCurve(const std::vector<Entity*>& selection);
	Entity* IntersCurve(Entity* a, Entity* b,
		std::vector<DirectX::XMFLOAT2>& pointsA,
		std::vector<DirectX::XMFLOAT2>& pointsB,
		std::vector<DirectX::XMFLOAT3>& pointsWorld,
		int texResolution);
	Entity* BicubicSurfaceObject(DirectX::XMFLOAT3 position, const BicubicSurfaceParams& params,
		std::vector<Entity*>& outPoints, std::vector<Entity*>& outSegments);
	void FillInSurface(Entity::Selection s, std::vector<Entity*>& outSurfs);
}