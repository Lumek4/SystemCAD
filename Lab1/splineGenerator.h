#pragma once
#include"component.h"
#include"pointCollection.h"
#include"vPointCollection.h"
#include"mesh.h"
#include"camera.h"

class SplineGenerator : public Component
{
public:
	SplineGenerator(Entity& owner);
	const char* Name() const override { return "SplineGenerator"; }

	Mesh* GetPolygon();
	bool drawPolygon = false;

	PointCollection& pointCollection;
	void UpdateBezier();
	void PropagateFromBezier(int index);
	int GetBezierPointCount() { return bezierPoints; }
	VPointCollection& bCollection;

private:
	int bezierPoints = 0;
	bool modified = true, moved = false;
	Event<NeedRedrawEventData>::Reaction NeedsRedraw_PointCollection;
	Event<NeedRedrawEventData>::Reaction NeedsRedraw_VPointCollection;

	DirectX::XMVECTOR GetBezierPoint(int index);
	DirectX::XMVECTOR GetPoint(int i);
	void SetPoint(int i, DirectX::XMVECTOR v);
	DirectX::XMVECTOR GetBPoint(int i);

	std::unique_ptr<Mesh> m;

};