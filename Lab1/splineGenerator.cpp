#include"all_components.h"
#include"catalogue.hpp"
#include"entity.hpp"
using namespace DirectX;

void SplineGenerator::NeedsRedraw_PointCollectionFunction(void* arg, NeedRedrawEventData)
{
	auto _this = (SplineGenerator*)arg;
	_this->modified = true;
	int count = _this->pointCollection.GetCount();
	int requiredPoints = count >= 4 ? (count - 3) * 3 + 1 : 0;
	if (requiredPoints != _this->bezierPoints)
	{
		_this->bezierPoints = requiredPoints;
		_this->bCollection.Resize(_this->bezierPoints);
	}
	_this->UpdateBezier();
}
void SplineGenerator::NeedsRedraw_VPointCollectionFunction(void* arg, NeedRedrawEventData e)
{
	auto _this = (SplineGenerator*)arg;
	if (e.ind != -1)
	{
		_this->PropagateFromBezier(e.ind);
	}
	else
		_this->UpdateBezier();
	_this->modified = true;
}
SplineGenerator::SplineGenerator(Entity& owner)
	:Component(ComponentConstructorArgs(SplineGenerator)),
	pointCollection(RequireComponent(PointCollection)),
	bCollection(RequireComponent(VPointCollection)),
	NeedsRedraw_PointCollection(this, NeedsRedraw_PointCollectionFunction),
	NeedsRedraw_VPointCollection(this, NeedsRedraw_VPointCollectionFunction)
{
	owner.Register(pointCollection.onCollectionModified, NeedsRedraw_PointCollection);
	owner.Register(bCollection.onCollectionModified, NeedsRedraw_VPointCollection);

	NeedsRedraw_PointCollection({ -1 });
}

Mesh* SplineGenerator::GetPolygon()
{
	if (!modified)
		return m.get();
	int count = pointCollection.GetCount();
	modified = false;

	std::vector<VertexPosition> vv;
	std::vector<unsigned> ii;
	for (int i = 0; i < count; i++)
	{
		ii.emplace_back(vv.size());
		vv.emplace_back(pointCollection.GetPoint(i));
	}

	m = std::make_unique<Mesh>(vv, ii, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	return m.get();
}

void SplineGenerator::UpdateBezier()
{
	for (int i = 0; i < bezierPoints; i++)
		if (bCollection.points[i] != bCollection.selection)
		{
			XMFLOAT3 v;
			XMStoreFloat3(
				&v,
				GetBezierPoint(i)
			);
			bCollection.points[i]->GetComponent<Transform>()->SetPosition(v);
		}
}

void SplineGenerator::PropagateFromBezier(int index)
{
	int segment = index / 3;
	auto s0 = GetPoint(segment);
	auto s1 = GetPoint(segment + 1);
	auto s2 = GetPoint(segment + 2);
	
	auto delta = GetBPoint(index) - GetBezierPoint(index);
	float h = 5.0f / 4;
	float l = 0.5f;
	switch (index % 3)
	{
	case 0:
		//SetPoint(segment + 0, s0 + delta);
		SetPoint(segment + 1, s1 + 3.0/2*delta);
		//SetPoint(segment + 2, s2 + delta);
		break;
	case 1:								
		SetPoint(segment + 1, s1 + h*delta);
		SetPoint(segment + 2, s2 + l*delta);
		break;
	case 2:								
		SetPoint(segment + 1, s1 + l*delta);
		SetPoint(segment + 2, s2 + h*delta);
		break;
	}
	//auto* e = bCollection.selection;
	UpdateBezier();
}

DirectX::XMVECTOR SplineGenerator::GetBezierPoint(int index)
{
	int segment = index / 3;
	auto s1 = GetPoint(segment + 1);
	auto s2 = GetPoint(segment + 2);
	switch (index % 3)
	{
	case 0:
		auto s0 = GetPoint(segment);
		return XMVectorLerp(
			XMVectorLerp(s1, s0, 1.0f / 3),
			XMVectorLerp(s1, s2, 1.0f / 3),
			0.5f);
	case 1:
		return XMVectorLerp(s1, s2, 1.0f / 3);
	case 2:
		return XMVectorLerp(s1, s2, 2.0f / 3);
	}
}

DirectX::XMVECTOR SplineGenerator::GetPoint(int i)
{
	auto v = pointCollection.entities[i]->GetComponent<Transform>()->Position();
	XMFLOAT4 vv = { v.x, v.y, v.z, 1 };
	return XMLoadFloat4(&vv);
}

void SplineGenerator::SetPoint(int i, DirectX::XMVECTOR v)
{
	XMStoreFloat3(&pointCollection.entities[i]->GetComponent<Transform>()->localPosition, v);
}

DirectX::XMVECTOR SplineGenerator::GetBPoint(int i)
{
	auto v = bCollection.points[i]->GetComponent<Transform>()->Position();
	XMFLOAT4 vv = { v.x, v.y, v.z, 1 };
	return XMLoadFloat4(&vv);
}