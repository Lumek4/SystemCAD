#include "bezierInterpolator.h"
#include"component.h"
#include"camera.h"
#include"vecmath.h"
using namespace DirectX;

void BezierInterpolator::NeedsRedraw_PointCollectionFunction(void* arg, NeedRedrawEventData)
{
	auto _this = (BezierInterpolator*)arg;
	_this->modified = true;
	int count = _this->pointCollection.GetCount();
	int segments = count - 1;
	int requiredPoints = count >= 4 ? (segments) * 3 + 1 : 0;
	if (requiredPoints != _this->bezierPoints)
	{
		_this->bezierPoints = requiredPoints;
		_this->bCollection.Resize(_this->bezierPoints);
		_this->onPointLengthChanged.Notify();
	}
	_this->UpdateBezier();
}
BezierInterpolator::BezierInterpolator(Entity& owner)
	:Component(ComponentConstructorArgs(BezierInterpolator)),
	pointCollection(RequireComponent(PointCollection)),
	bCollection(RequireComponent(VPointCollection)),
	NeedsRedraw_PointCollection(this, NeedsRedraw_PointCollectionFunction)
{
	owner.Register(pointCollection.onCollectionModified, NeedsRedraw_PointCollection);
	NeedsRedraw_PointCollection({ -1 });
}
float len(XMFLOAT3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}
void BezierInterpolator::UpdateBezier()
{
	int count = pointCollection.GetCount();
	if (count < 4)
		return;
	int n = count - 1;
	std::vector<XMFLOAT3> c(n+1);
	c[0] = c[n] = {};
	std::vector<float> d(n);
	{
		std::vector<XMFLOAT3> R(n - 1);
		std::vector<float> alpha(n - 1);
		std::vector<float> beta(n - 1);
		{
			XMFLOAT3 P_im1,
				P_i = pointCollection.GetPoint(0),
				P_ip1 = pointCollection.GetPoint(1);

			d[0] = len(P_i - P_ip1);
			for (int i = 0; i < n - 1; i++)
			{
				P_im1 = P_i;
				P_i = P_ip1;
				P_ip1 = pointCollection.GetPoint(i + 2);

				d[i+1] = len(P_i - P_ip1);


				alpha[i] = d[i] / (d[i] + d[i + 1]);
				beta[i] = d[i+1] / (d[i] + d[i + 1]);

				R[i] =
					(P_ip1 - P_i) * (3 / d[i + 1] / (d[i] + d[i + 1])) -
					(P_i - P_im1) * (3 / d[i] / (d[i] + d[i + 1]))
					;
			}
			alpha[0] = beta[n - 2] = 0;
		}
		{
			std::vector<float> betaprim(n - 2);
			std::vector<XMFLOAT3> Rprim(n - 1);
			betaprim[0] = beta[0] / 2;
			for (int i = 1; i < n - 2; i++)
			{
				betaprim[i] = beta[i] / (2 - alpha[i] * betaprim[i - 1]);
			}
			Rprim[0] = R[0] * (1.0f / 2);
			for (int i = 1; i < n - 1; i++)
			{
				Rprim[i] = (R[i] - alpha[i] * Rprim[i - 1]) *
					(1.0f / (2 - alpha[i] * betaprim[i - 1]));
			}

			c[n - 2 + 1] = Rprim[n - 2];
			for (int i = n - 3; i >= 0; i--)
			{
				c[i + 1] = Rprim[i] - betaprim[i] * c[i + 2];
			}
		}
	}
	{
		XMFLOAT3 A, A1 = pointCollection.GetPoint(0), B, C, D;
		XMFLOAT3 bezier[3] = {};
		XMFLOAT3 del1[2] = {};
		XMFLOAT3 del2[1] = {};
		for (int i = 0; i < n; i++)
		{
			A = A1; A1 = pointCollection.GetPoint(i + 1);
			D = (c[i + 1] - c[i])*(1.0f/3/d[i]);
			C = c[i];
			B = (A1 - A) * (1 / d[i]) - (C + D * d[i]) * d[i];

			del2[0] = C * (1.0f/3) * d[i]*d[i];
			del1[0] = B * (1.0f/3) * d[i];
			bezier[0] = A;

			del1[1] = del1[0] + del2[0];
			bezier[1] = del1[0] + bezier[0];

			bezier[2] = bezier[1] + del1[1];

			SetPoint(bezier[0], i * 3 + 0);
			SetPoint(bezier[1], i * 3 + 1);
			SetPoint(bezier[2], i * 3 + 2);
		}
		SetPoint(A1, n * 3);
	}
	bCollection.onCollectionModified.Notify({ -1 });
}

void BezierInterpolator::SetPoint(DirectX::XMFLOAT3 p, int i)
{
	bCollection.points[i]->GetComponent<Transform>()->SetPosition(p);
}
