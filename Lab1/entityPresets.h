#pragma once
#include"entity.hpp"
#include"mesh.h"
#include<DirectXMath.h>
#include<vector>

enum SURFACE_WRAP
{
	SURFACE_WRAP_NONE = 0,
	SURFACE_WRAP_U = 1,
	SURFACE_WRAP_V = 2,
	SURFACE_WRAP_UV = SURFACE_WRAP_U | SURFACE_WRAP_V
};
struct BicubicSurfaceParams
{
	bool deBoor = false;
	DirectX::XMINT2 division{ 2,2 };
	SURFACE_WRAP wrapMode{ SURFACE_WRAP_NONE };
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