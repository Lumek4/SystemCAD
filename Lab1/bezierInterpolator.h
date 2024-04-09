#pragma once
#include"component.h"
#include"pointCollection.h"
#include"vPointCollection.h"
#include"mesh.h"

class Camera;
class BezierInterpolator : public Component
{
public:
	BezierInterpolator(Entity& owner);
	const char* Name() const override { return "BezierInterpolator"; }

	PointCollection& pointCollection;
	void UpdateBezier();
	//void PropagateFromBezier(DirectX::XMVECTOR changed, int index);
	int GetBezierPointCount() { return bezierPoints; }
	VPointCollection& bCollection;

	Event<void> onPointLengthChanged{ };
private:
	int bezierPoints = 0;
	bool modified = true, moved = false;
	Event<NeedRedrawEventData>::Reaction NeedsRedraw_PointCollection;

	void SetPoint(DirectX::XMFLOAT3 p, int i);

	std::unique_ptr<Mesh> m;

};