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
	Entity* Model(DirectX::XMFLOAT3 position, Mesh* mesh);
	Entity* Point(DirectX::XMFLOAT3 position);
	Entity* BezierCurveObject(const std::vector<Entity*>& selection);
	Entity* SplineCurve(const std::vector<Entity*>& selection);
	Entity* InterpCurve(const std::vector<Entity*>& selection);
	Entity* BicubicSurfaceObject(DirectX::XMFLOAT3 position, const BicubicSurfaceParams& params,
		std::vector<Entity*>& outPoints, std::vector<Entity*>& outSegments);
	void FillInSurface(Entity::Selection s, std::vector<Entity*>& outSurfs);
}