#include "outline.h"

#include "intersect_interfaces.h"
#include"intersect.h"
#include"bicubicSurface.h"
#include"torusGenerator.h"

using namespace DirectX;


void InflateNonLoop(std::vector<XMFLOAT2>& outline, float radius)
{
	std::vector<XMFLOAT2> inflated(outline.size()-2);
	for (int j = 1; j < outline.size()-1; j++)
	{
		int j0 = (j - 1);
		int j1 = (j + 0);
		int j2 = (j + 1);
		XMFLOAT2 t = { outline[j2].x - outline[j0].x,
			outline[j2].y - outline[j0].y };
		float r = radius / sqrtf(t.x * t.x + t.y * t.y);
		inflated[j0].x = outline[j1].x + r * t.y;
		inflated[j0].y = outline[j1].y - r * t.x;
		if (isnan(r))
			r = r;
	}
	outline = inflated;
}

void InflateLoop(std::vector<XMFLOAT2>& outline, float radius)
{
	std::vector<XMFLOAT2> inflated(outline.size()-1);
	for (int j = outline.size()-1; j < 2*outline.size()-2; j++)
	{
		int j0 = (j + 0) % outline.size();
		int j1 = (j + 1) % outline.size();
		int j2 = (j + 2) % outline.size();
		XMFLOAT2 t = { outline[j2].x - outline[j0].x,
			outline[j2].y - outline[j0].y };
		float r = radius / sqrtf(t.x * t.x + t.y * t.y);
		inflated[j1].x = outline[j1].x + r * t.y;
		inflated[j1].y = outline[j1].y - r * t.x;
		if (isnan(r))
			r = r;
	}
	outline = inflated;
}

void Paths::OutlineSet(Entity::Selection& set, Entity* plane, XMFLOAT3 cursor,
	float precision,
	std::vector<XMFLOAT2>& outline,
	std::vector<XMFLOAT2>& fill)
{
	auto* base = plane->GetComponent<BicubicSurface>();
	if (!base)
		return;
	XMFLOAT2 cursor2d;
	{
		auto base_data = IntersectData<BicubicSurface>(base);
		cursor2d = nearCursorPoint<BicubicSurface, 1024>(base_data, cursor);
	}

	const float toolDiameter = 10;
	const float baseSize = 150;
	const bool debug = false;

	std::vector<XMFLOAT2> uva, uvb;
	std::vector<XMFLOAT3> pts;
	bool loop;
	std::vector<std::vector<XMFLOAT2>> shapes;
	std::vector<XMFLOAT2> bb_topleft;
	std::vector<XMFLOAT2> bb_btright;
	std::vector<bool> loops;
	std::vector<bool> used;
	for (int i = 0; i < set.size(); i++)
	{
		auto* torus = set[i]->GetComponent<TorusGenerator>();
		auto* surface = set[i]->GetComponent<BicubicSurface>();
		if (torus)
			if (intersect(base, torus, cursor, precision, pts, uva, uvb, loop))
			{
				shapes.push_back(uva);
				loops.push_back(loop);
				used.push_back(false);
			}
			else torus = nullptr;
		//float pp = i == 3 ? precision / 10 : precision;
		if (surface)
			if (intersect(base, surface, cursor, precision, pts, uva, uvb, loop))
			{
				shapes.push_back(uva);
				loops.push_back(loop);
				used.push_back(false);
			}
			else surface = nullptr;
		if (torus || surface)
		{
			uva.clear();
			uvb.clear();
			pts.clear();
		}
	}
	for (int i = 0; i < shapes.size(); i++)
	{
		if (loops[i])
			InflateLoop(shapes[i], toolDiameter/2/baseSize);
		else
			InflateNonLoop(shapes[i], toolDiameter / 2 / baseSize);

		XMFLOAT2 tl{ INFINITY, INFINITY }, br{ -INFINITY, -INFINITY };
		for (int j = 0; j < shapes[i].size(); j++)
		{
			if (shapes[i][j].x < tl.x)
				tl.x = shapes[i][j].x;
			if (shapes[i][j].y < tl.y)
				tl.y = shapes[i][j].y;

			if (shapes[i][j].x > br.x)
				br.x = shapes[i][j].x;
			if (shapes[i][j].y > br.y)
				br.y = shapes[i][j].y;
		}
		bb_topleft.push_back(tl);
		bb_btright.push_back(br);
	}
	if (debug)
	{
		for (int i = 0; i < shapes.size(); i++)
			for (int j = 0; j < shapes[i].size(); j++)
				outline.push_back(shapes[i][j]);
		return;
	}
	int startingShape{ -1 };
	int startingVert{ -1 };
	int direction{ -1 };
	{
		float distSq = INFINITY;
		for (int i = 0; i < shapes.size(); i++)
			for (int j = 0; j < shapes[i].size(); j++)
			{
				float x = cursor2d.x - shapes[i][j].x,
					y = cursor2d.y - shapes[i][j].y;
				float d = x * x + y * y;
				if (d < distSq)
				{
					startingShape = i;
					startingVert = j;
					distSq = d;
				}
			}
		//used[startingShape] = true;

		int nextInd = (startingVert + 1) % shapes[startingShape].size();
		if (vecmath::triArea(cursor2d,
			shapes[startingShape][startingVert],
			shapes[startingShape][nextInd]) > 0)
			direction = 1;
	}

	int shape_i = startingShape,
		vert_i = startingVert;
	int iters = 0;
	do
	{
		if (iters % 5'000 == 0)
			iters = 0;
		iters++;
		outline.push_back(shapes[shape_i][vert_i]);
		int nextInd = (vert_i + direction + shapes[shape_i].size())
			% shapes[shape_i].size();
		int laterInd = (vert_i + 2*direction + shapes[shape_i].size())
			% shapes[shape_i].size();
		if (laterInd == 0 && direction == 1 && !loops[shape_i])
			break;
		if (laterInd == shapes[shape_i].size()-1 && direction == -1 && !loops[shape_i])
			break;
		bool collided = false;
		for (int i = 0; i < shapes.size(); i++)
		{
			if (i == shape_i) continue;
			if (used[i]) continue;
			if (!vecmath::bb2d(bb_topleft[i], bb_btright[i],
				shapes[shape_i][laterInd]))
				continue;
			for (int j = 0; j < shapes[i].size(); j++)
			{
				int jp2 = j+2;
				if (jp2 >= shapes[i].size())
					if (!loops[i]) continue;
					else jp2 -= shapes.size();

				if (!vecmath::bb2d(shapes[i][j], shapes[i][jp2],
					outline.back(), shapes[shape_i][laterInd]))
					continue;
				int dir = vecmath::segments2d(
					shapes[i][j], shapes[i][jp2],
					outline.back(), shapes[shape_i][laterInd]);
				if (dir == 0) // no collision
					continue;
				collided = true;
				direction = -dir;
				shape_i = i;
				nextInd = j;
				break;
			}
			if (collided)
				break;
		}
		vert_i = nextInd;
	}while(shape_i != startingShape || vert_i != startingVert);

	//XMFLOAT2 uv = { 0, 0 };
	//bool moving = true; bool inside = false, was_inside = false;
	//bool backwards = false;
	//int it = 0;
	////if (k == 1)
	////	precision *= 0.5f;
	//while (moving)
	//{
	//	inside = true;

	//	bool moving = true;
	//	while (moving)
	//	{
	//		if (!was_inside && inside)
	//			fill.push_back({NAN, NAN});
	//		was_inside = inside;
	//		if (inside)
	//			fill.push_back(uv);
	//		float nextu = uv.x + (backwards ? -1 : 1) * precision;
	//		if ((!backwards && nextu > 1) ||
	//			(backwards && nextu < 0))
	//		{
	//			uv.x = backwards ? 0 : 1;
	//			break;
	//		}
	//		for (int j = 0; j < outline.size(); j++)
	//		{
	//			int jp1 = (j + 1) % outline.size();
	//			auto& b1 = outline[j];
	//			auto& b2 = outline[jp1];
	//			if (fabsf(b1.x - b2.x) > 1 || fabsf(b1.y - b2.y) > 1)
	//				continue;

	//			if (fminf(b1.y, b2.y) <= uv.y && uv.y <= fmaxf(b1.y, b2.y))
	//				if (vecmath::segments2d(uv, { nextu, uv.y }, b1, b2))
	//				{
	//					inside = !inside; break;
	//				}
	//		}
	//		if (was_inside && !inside)
	//			fill.push_back({ NAN, NAN });

	//		uv.x = nextu;
	//		it++;
	//	}
	//	uv.y += precision;
	//	if (uv.y > 1)
	//		break;
	//	backwards = !backwards;
	//}
	return;
}
