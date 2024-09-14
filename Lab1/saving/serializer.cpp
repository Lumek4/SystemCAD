#include "serializer.h"
#include"../all_components.h"


void Serializer::Torus(Entity* e, std::string& name,
	DirectX::XMFLOAT2& radii,
	DirectX::XMFLOAT3& position,
	DirectX::XMFLOAT3& rotation,
	DirectX::XMFLOAT3& scale,
	DirectX::XMINT2& samples)
{
	name = e->GetName();
	auto* torus = e->GetComponent<TorusGenerator>();
	radii = torus->GetData().radii;
	std::swap(radii.x, radii.y);
	samples = torus->GetData().division;
	auto* transform = e->GetComponent<ModelTransform>();
	position = transform->Position();
	scale = transform->scale;
	rotation = Quaternion::ToEulerAngles(transform->rotation);
	std::swap(rotation.z, rotation.y);
}

void Serializer::Curve(Entity* e, std::string& name,
	Entity::Selection& controlPoints)
{
	name = e->GetName();
	controlPoints = e->GetComponent<PointCollection>()->Get();
}

void Serializer::Surface(Entity* e, std::string& name,
	std::vector<std::string_view>& segmentNames,
	Entity::Selection& controlPoints,
	DirectX::XMINT2& parameterWrapped,
	DirectX::XMINT2& size)
{
	name = e->GetName();
	auto points = e->GetComponent<PointCollection>()->Get();
	auto* surf = e->GetComponent<BicubicSurface>();
	parameterWrapped = surf->wrapMode;
	size = surf->division;
		for (int y = 0; y < size.y; y++)
			for (int x = 0; x < size.x; x++)
		{
			auto* segment = surf->GetSegments()[x + y * size.x];
			segmentNames.push_back(segment->owner.GetName());
			for (int yy = 0; yy < 4; yy++)
				for (int xx = 0; xx < 4; xx++)
					controlPoints.push_back(points[segment->indices[xx + yy * 4]]);
		}
}
