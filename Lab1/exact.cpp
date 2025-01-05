#include "exact.h"

#include "intersect_interfaces.h"
#include"intersect.h"
#include"bicubicSurface.h"
#include"torusGenerator.h"

static const int headDiameter = 8;
static const int baseWidth = 150;
static const int baseHeight = 50-16;
static const float r = uToW(mmToU(headDiameter / 2.0f)),
r2 = r * r;
static const float heightMultiplier = (float)baseWidth/baseHeight;

using namespace DirectX;
struct modelCase
{
	XMFLOAT3 cursor;
	float detail;
	int ida, idb;
};
modelCase hardcodeSurfSurf[] = {
	{{5.0f, 0.0f, 1.0f},	0.05f,	0,	5},
	{{-2.0f, 10.0f, 3.0f},	0.05f,	0,	3},
	{{-5.0f, 10.0f, 0.0f},	0.1f,	0,	4},
	{{0.0f, 0.0f, 0.0f},	0.1f,	0,	2},
	{{-3.0f, 0.0f, 0.5f},	0.5f,	0,	1},

	{{0,0,0},	0.1f,	1,	2},
	{{0,0,0},	0.6f,	1,	3},
	{{0,0,0},	0.6f,	1,	4},
	{{6.0f,3.0f,0},	0.2f,	1,	5},
};
modelCase hardcodeSurfToru[] = {
	{{0,0,0},	0.05f,	0,	1},
	{{0,0,0},	0.05f,	1,	1},
	{{0,0,0},	0.05f,	3,	1},
};
modelCase hardcodeSurfSegm[] = {
	{{0.0f, 0.0f, 0.0f},	0.2f,	1,	0},
	{{0.0f, 0.0f, 0.0f},	0.05f,	1,	1},
	{{-7.0f, -15.0f, 0.0f},	0.05f,	1,	1},
	{{0.0f, 0.0f, 0.0f},	0.1f,	0,	1},
};
modelCase hardcodeSegmToru[] = {
	{{0.0f, -20.0f, 0.0f},	0.2f,	0,	0},
	{{1.5f, -20.0f, 2.5f},	0.2f,	0,	0},
};
// 0Tors, 1Podstawka, Pletwy: 2Dl, 3DP, 4GP, 5GL
// 0Oko, 1Pletwa Ogonowa (-1 Aureola)
// 0 Glowa przod 1 Glowa tyl

void PrepareIntersections(
	std::vector<IntersectData<Inflation<BicubicSurface>>>& surf,
	std::vector<IntersectData<Inflation<TorusGenerator>>>& toru,
	std::vector<IntersectData<Inflation<BicubicSegment>>>& segm,
	std::vector<std::vector<std::vector<XMFLOAT2>>>& shapes,
	std::vector<std::vector<bool>>& loops)
{
	std::vector<XMFLOAT3> pts;
	std::vector<XMFLOAT2> uva, uvb;
	for (int i = 0; i < sizeof(hardcodeSurfSurf) / sizeof(hardcodeSurfSurf[0]); i++)
	{
		bool loop;
		if (intersect(
			surf[hardcodeSurfSurf[i].ida],
			surf[hardcodeSurfSurf[i].idb],
			false,
			hardcodeSurfSurf[i].cursor,
			hardcodeSurfSurf[i].detail, pts, uva, uvb, loop))
		{
			loops[hardcodeSurfSurf[i].ida].push_back(loop);
			loops[hardcodeSurfSurf[i].idb].push_back(loop);
			shapes[hardcodeSurfSurf[i].ida].push_back(uva);
			shapes[hardcodeSurfSurf[i].idb].push_back(uvb);
			//path.insert(path.end(), pts.begin(), pts.end());
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
	for (int i = 0; i < sizeof(hardcodeSurfToru) / sizeof(hardcodeSurfToru[0]); i++)
	{
		bool loop;
		if (intersect(
			surf[hardcodeSurfToru[i].ida],
			toru[hardcodeSurfToru[i].idb],
			false,
			hardcodeSurfToru[i].cursor,
			hardcodeSurfToru[i].detail, pts, uva, uvb, loop))
		{
			loops[hardcodeSurfToru[i].ida].push_back(loop);
			loops[hardcodeSurfToru[i].idb + surf.size()].push_back(loop);
			shapes[hardcodeSurfToru[i].ida].push_back(uva);
			shapes[hardcodeSurfToru[i].idb + surf.size()].push_back(uvb);
			//path.insert(path.end(), pts.begin(), pts.end());
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
	for (int i = 0; i < sizeof(hardcodeSurfSegm) / sizeof(hardcodeSurfSegm[0]); i++)
	{
		bool loop;
		if (intersect(
			surf[hardcodeSurfSegm[i].ida],
			segm[hardcodeSurfSegm[i].idb],
			false,
			hardcodeSurfSegm[i].cursor,
			hardcodeSurfSegm[i].detail, pts, uva, uvb, loop))
		{
			loops[hardcodeSurfSegm[i].ida].push_back(loop);
			loops[hardcodeSurfSegm[i].idb + surf.size() + toru.size()].push_back(loop);
			shapes[hardcodeSurfSegm[i].ida].push_back(uva);
			shapes[hardcodeSurfSegm[i].idb + surf.size() + toru.size()].push_back(uvb);
			//path.insert(path.end(), pts.begin(), pts.end());
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
	for (int i = 0; i < sizeof(hardcodeSegmToru) / sizeof(hardcodeSegmToru[0]); i++)
	{
		bool loop;
		if (intersect(
			segm[hardcodeSegmToru[i].ida],
			toru[hardcodeSegmToru[i].idb],
			false,
			hardcodeSegmToru[i].cursor,
			hardcodeSegmToru[i].detail, pts, uva, uvb, loop))
		{
			loops[hardcodeSegmToru[i].ida + surf.size() + toru.size()].push_back(loop);
			loops[hardcodeSegmToru[i].idb + surf.size()].push_back(loop);
			shapes[hardcodeSegmToru[i].ida + surf.size() + toru.size()].push_back(uva);
			shapes[hardcodeSegmToru[i].idb + surf.size()].push_back(uvb);
			//path.insert(path.end(), pts.begin(), pts.end());
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
}

inline XMFLOAT3 PathPoint(XMFLOAT3 p)
{
	return { p.x / 54 + 0.5f, p.y / 54 + 0.5f, uWTouH(wToU(p.z - r)) };
}
inline XMFLOAT3 FreePoint(XMFLOAT3 p, float lift)
{
	return { p.x / 54 + 0.5f, p.y / 54 + 0.5f, uWTouH(wToU(lift)) };
}
inline float GetBBRiseHeight(const std::vector<BoundingBox>& bbs, int k)
{
	if (k == -1)
	{
		float maxh = 0;
		for (int i = 0; i < bbs.size(); i++)
			maxh = std::fmaxf(XMVectorGetZ(bbs[i].hi), maxh);
		return maxh;
	}
	return XMVectorGetZ(bbs[k].hi);
}

void Paths::Exact(Entity::Selection& set, float precision, std::vector<DirectX::XMFLOAT3>& path)
{
	const char* cacheFilePath = "../ExactShapes.b";
	std::vector<Inflation<BicubicSurface>> surf_inf;
	std::vector<Inflation<TorusGenerator>> toru_inf;
	std::vector<Inflation<BicubicSegment>> segm_inf;
	std::vector<IntersectData<Inflation<BicubicSurface>>> surf;
	std::vector<IntersectData<Inflation<TorusGenerator>>> toru;
	std::vector<IntersectData<Inflation<BicubicSegment>>> segm;
	int base = -1;
	std::vector<std::vector<std::vector<XMFLOAT2>>> shapes;
	std::vector<std::vector<bool>> loops;
	std::vector<BoundingBox> boundingBoxes;
	for(int i = 0; i<set.size(); i++)
	{
		auto* s = set[i]->GetComponent<BicubicSurface>();
		auto* t = set[i]->GetComponent<TorusGenerator>();
		if (s)
		{
			auto& segs = s->GetSegments();
			if (set[i]->GetName().compare("Glowa") != 0)
			{
				if (set[i]->GetName().compare("Podstawka") == 0)
					base = surf_inf.size();
				surf_inf.push_back({ s,r });
				shapes.push_back({});
				loops.push_back({});
			}
			else
			{
				for (int j = 0; j < segs.size(); j++)
				{
					segm_inf.push_back({ segs[j],r });
					shapes.push_back({});
					loops.push_back({});
					//segm_inf.back().specialTreatment = true;
				}
			}
		}
		if (t)
		{
			toru_inf.push_back({ t,r });
			shapes.push_back({});
			loops.push_back({});
		}
	}
	for (int i = 0; i < surf_inf.size(); i++)
	{
		surf.push_back({ &surf_inf[i] });
		boundingBoxes.push_back(surf[i].GetBB());
	}
	for (int i = 0; i < toru_inf.size(); i++)
	{
		toru.push_back({ &toru_inf[i] });
		boundingBoxes.push_back(toru[i].GetBB());
	}
	for (int i = 0; i < segm_inf.size(); i++)
	{
		segm.push_back({ &segm_inf[i] });
		boundingBoxes.push_back(segm[i].GetBB());
	}
	std::vector<XMFLOAT3> pts;
	std::vector<XMFLOAT2> uva, uvb;

#if 0
	for (int i = 0; i < segm.size(); i++)
	{
		bool loop;
		if (intersect(
			segm[i],
			toru[0],
			false,
			SceneCursor::instance.GetWorld(),
			precision, pts, uva, uvb, loop))
		{
			for (int j = 0; j < pts.size(); j++)
				path.push_back(PathPoint(pts[j]));
			pts.clear();
			uva.clear(); uvb.clear();
		}
	}
#else
	FILE* cacheFile = std::fopen(cacheFilePath, "rb");
	if (!cacheFile)
	{
		PrepareIntersections(surf, toru, segm,
			shapes, loops);
		cacheFile = std::fopen(cacheFilePath, "wb");
		if (cacheFile)
		{
			int count = shapes.size();
			std::fwrite(&count, sizeof(count), 1, cacheFile);
			for (int i = 0; i < count; i++)
			{
				int count2 = shapes[i].size();
				std::fwrite(&count2, sizeof(count2), 1, cacheFile);
				for (int j = 0; j < count2; j++)
				{
					int count3 = shapes[i][j].size();
					std::fwrite(&count3, sizeof(count3), 1, cacheFile);
					std::fwrite(shapes[i][j].data(), sizeof(shapes[0][0][0]), count3, cacheFile);
					bool v = loops[i][j];
					std::fwrite(&v, 1, 1, cacheFile);
				}
			}
			std::fclose(cacheFile);
		}
	}
	else
	{
		int count;
		std::fread(&count, sizeof(count), 1, cacheFile);
		shapes.resize(count);
		loops.resize(count);
		for(int i = 0; i<count; i++)
		{
			int count2;
			std::fread(&count2, sizeof(count2), 1, cacheFile);
			shapes[i].resize(count2);
			loops[i].resize(count2);
			for (int j = 0; j < count2; j++)
			{
				int count3;
				std::fread(&count3, sizeof(count3), 1, cacheFile);
				shapes[i][j].resize(count3);
				std::fread(shapes[i][j].data(), sizeof(shapes[0][0][0]), count3, cacheFile);
				bool v;
				std::fread(&v, 1, 1, cacheFile);
				loops[i][j] = v;
			}
		}
		std::fclose(cacheFile);
	}
	XMFLOAT2 middles[] = {
		{ 0.5f,0.7f },	  // 0Tors,
		{ 0.5f,0.1f },	  // 1Podstawka, 
		{ 0.6f,0.3f },	  // Pletwy: 2Dl, 
		{ 0.5f,0.5f },	  //		 3DP, 
		{ 0.5f,0.5f },	  //		 4GP, 
		{ 0.5f,0.5f },	  //		 5GL
		//{ 0.5f,0.80f }, // -1 Aureola, // large, small
		{ 0.5f,0.5f },	  // 0Oko, 6
		{ 0.375f,0.75f }, // 1Pletwa Ogonowa, 7
		{ 0.85f,0.85f },  // 0Glowa, 8
		{ 0.8f,0.05f },	  // 1Glowa, 9
		{ 0.5f,0.8f },	  // 0Oko 2, 10
		{ 0.5f,0.5f },	  // 0Glowa 2, 11
	};
	/*for (int i = 0; i < shapes[1].size(); i++)
		for (int j = 0; j < shapes[1][i].size(); j++)
		{
			XMFLOAT3 p;
			XMVECTOR pos, u, v;
			surf[1].Point(shapes[1][i][j], pos, u, v);
			XMStoreFloat3(&p, pos);
			path.push_back(PathPoint(p));
		}*/
	std::vector<std::vector<XMFLOAT2>> outline(shapes.size() + 2);
	for (int k = 0; k < shapes.size() + 2; k++)
	//for (int k = 0; k <= 5; k++)
	{
		if (k == base)
			continue;
		int kk = (k == shapes.size()) ? 6 : ((k == shapes.size() + 1) ? 8 : k);
		XMFLOAT2 cursor2d = middles[k];
		std::vector<XMFLOAT2> bb_topleft;
		std::vector<XMFLOAT2> bb_btright;
		auto& my_shapes = shapes[kk];
		auto& my_loops = loops[kk];
		if (my_shapes.size() == 0)
			continue;
		for (int i = 0; i < my_shapes.size(); i++)
		{
			auto& shape = my_shapes[i];
			XMFLOAT2 tl{ INFINITY, INFINITY }, br{ -INFINITY, -INFINITY };
			for (int j = 0; j < shape.size(); j++)
			{
				if (shape[j].x < tl.x)
					tl.x = shape[j].x;
				if (shape[j].y < tl.y)
					tl.y = shape[j].y;

				if (shape[j].x > br.x)
					br.x = shape[j].x;
				if (shape[j].y > br.y)
					br.y = shape[j].y;
			}
			bb_topleft.push_back(tl);
			bb_btright.push_back(br);
		}
		int startingShape{ -1 };
		int startingVert{ -1 };
		int direction{ -1 };
		{
			float distSq = INFINITY;
			for (int i = 0; i < my_shapes.size(); i++)
				for (int j = 0; j < my_shapes[i].size(); j++)
				{
					float x = cursor2d.x - my_shapes[i][j].x,
						y = cursor2d.y - my_shapes[i][j].y;
					float d = x * x + y * y;
					if (d < distSq)
					{
						startingShape = i;
						startingVert = j;
						distSq = d;
					}
				}
			//used[startingShape] = true;

			int nextInd = (startingVert + 1) % my_shapes[startingShape].size();
			if (vecmath::triArea(cursor2d,
				my_shapes[startingShape][startingVert],
					my_shapes[startingShape][nextInd]) > 0)
					direction = 1;
		}
		int shape_i = startingShape,
			vert_i = startingVert;
		int iters = 0;
		XMFLOAT2 a1 = my_shapes[shape_i][vert_i], a2;
		do
		{
			if (iters % 5'000 == 0)
				iters = 0;
			iters++;
			/*XMFLOAT3 p;
			{
				XMVECTOR pos, u, v;
				if (k < surf.size())
					surf[k].Point(my_shapes[shape_i][vert_i], pos, u, v);
				else
					toru[k - surf.size()].Point(my_shapes[shape_i][vert_i], pos, u, v);
				XMStoreFloat3(&p, pos);
			}*/

			outline[k].push_back(a1);
			int nextInd = vert_i;

			if (a1.x == 0 || a1.y == 0 || a1.x == 1 || a1.y == 1)
			{

				int ud = (a1.x == 0) ? -1 : ((a1.x == 1) ? 1 : 0);
				int lr = (a1.y == 0) ? 1 : ((a1.y == 1) ? -1 : 0);
				if (shape_i != -1)
				{
					a1 = { a1.x + lr * precision * 0.05f, a1.y + ud * precision * 0.05f };
					outline[k].push_back(a1);
				}
				a2 = { a1.x + lr * precision * 0.05f, a1.y + ud * precision * 0.05f };
				shape_i = -1;
				nextInd = vert_i = -1;
				if (a2.x < 0 || a2.y < 0 || a2.x > 1 || a2.y > 1)
					break;
			}
			else
			{
				nextInd = (vert_i + direction + my_shapes[shape_i].size())
					% my_shapes[shape_i].size();
				if (nextInd == 0 && direction == 1 && !my_loops[shape_i])
					break;
				if (nextInd == my_shapes[shape_i].size() - 1 && direction == -1 && !my_loops[shape_i])
					break;
				a2 = my_shapes[shape_i][nextInd];
			}
			if (fabsf(a1.x - a2.x) > 1 || fabsf(a1.y - a2.y) > 1)
			{
				vert_i = nextInd;
				a1 = a2;
				continue;
			}
			bool collided = false;
			for (int i = 0; i < my_shapes.size(); i++)
			{
				if (i == shape_i) continue;
				if (k < surf.size() &&
					!vecmath::bb2d(bb_topleft[i], bb_btright[i],
						a2))
					continue;
				for (int j = 0; j < my_shapes[i].size(); j++)
				{
					int jp1 = j + 1;
					if (jp1 >= my_shapes[i].size())
						if (!my_loops[i]) continue;
						else jp1 -= my_shapes.size();

					{
						auto& b1 = my_shapes[i][j];
						auto& b2 = my_shapes[i][jp1];
						if (fabsf(b1.x - b2.x) > 1 || fabsf(b1.y - b2.y) > 1)
							continue;
					}
					if (k < surf.size() &&
						!vecmath::bb2d(my_shapes[i][j], my_shapes[i][jp1],
							a1, a2))
						continue;
					int dir = vecmath::segments2d(
						my_shapes[i][j], my_shapes[i][jp1],
						a1, a2);
					if (dir == 0) // no collision
						continue;
					collided = true;
					direction = -dir;
					shape_i = i;
					nextInd = j + (direction + 1) / 2;
					a2 = my_shapes[shape_i][nextInd];
					break;
				}
				if (collided)
					break;
			}
			a1 = a2;
			vert_i = nextInd;
		} while (shape_i != startingShape || vert_i != startingVert);

	}
	float topRaiseHeight = GetBBRiseHeight(boundingBoxes, -1);
	XMFLOAT3 p;
	//if(false)
	{ // p³etwy
		for (int k = 0; k < 6; k++)
		{
			if (k == base)
				continue;
			bool firstDrop = true;
			auto& bb = surf[k].GetBB();
			float raiseHeight = GetBBRiseHeight(boundingBoxes, k);
			float size = 0;
			for (int i = 0; i < 3; i++)
				size += bb.hi.m128_f32[i] - bb.lo.m128_f32[i];
			int resolution = 0.2f * size / precision;
			XMVECTOR pos, uvec, vvec;
			auto& o = outline[k];
			bool reverse = false;
			bool inside_last = false;
			int udiv = (k == 0) ? 3 : 1;
			for (int u = 0; u < resolution/ udiv; u++)
			{
				inside_last = false;
				int vdirection = (u % 2 == 0) ? 1 : -1;
				int vstart = (u % 2 == 0) ? 0 : resolution-1;
				int vend = (u % 2 == 0) ? resolution-1 : 0;
				for (int v = vstart; v*vdirection <= vend*vdirection; v+= vdirection)
				{
					XMFLOAT2 uv = { (float)(reverse?v:u* udiv) / resolution, (float)(reverse ? u* udiv : v) / resolution };
					bool inside = true;
					for (int i = 0; i < outline[k].size(); i++)
					{
						int ip1 = (i + 1) % outline[k].size();
						if (!vecmath::bb2d(middles[k], uv, outline[k][i], outline[k][ip1]))
							continue;
						if (vecmath::segments2d(middles[k], uv, outline[k][i], outline[k][ip1]))
						{
							inside = !inside;
						}
					}
					if (inside)
					{
						surf[k].Point(uv, pos, uvec, vvec);
						XMStoreFloat3(&p, pos);
						if (firstDrop)
						{
							path.push_back(FreePoint(p, topRaiseHeight));
							firstDrop = false;
						}
						else if (!inside_last)
							path.push_back(FreePoint(p, raiseHeight));
						path.push_back(PathPoint(p));
					}
					else if (inside_last)
						path.push_back(FreePoint(p, raiseHeight));
					inside_last = inside;
				}
			}
			path.push_back(FreePoint(p, topRaiseHeight));
		}
	}
	//if(false)
	{ // oko
		auto& outside = outline[6];
		auto& inside = outline[10];
		XMVECTOR pos, u, v;
		bool firstDrop = true;
		for (int i = 0; i <= 10; i++)
		{
			float t = (float)i / 10;
			int outU = 73;
			int inU = 326;
			int inU2 = 325;
			for (outU; outU != 72; outU = (outU + 2) % outside.size())
			{
				while (inside[inU].x > outside[outU].x)
				{
					inU2 = inU;
					inU = (inU + 1) % inside.size();
				}
				float ft = -(inside[inU].x - outside[outU].x) / (inside[inU].x - inside[inU2].x);
				XMFLOAT2 a = {
					inside[inU].x * (1 - ft) + inside[inU2].x * ft,
					inside[inU].y * (1 - ft) + inside[inU2].y * ft },
					b = outside[outU];
				toru[0].Point({ a.x * t + b.x * (1 - t), a.y * t + b.y * (1 - t) }, pos, u, v);
				XMStoreFloat3(&p, pos);
				if (firstDrop)
				{
					path.push_back(FreePoint(p, topRaiseHeight));
					firstDrop = false;
				}
				path.push_back(PathPoint(p));
			}
		}
		path.push_back(FreePoint(p, topRaiseHeight));
	}//if(false)
	{ // ogon
		XMVECTOR pos, u, v;
		auto& outside = outline[7];
		toru[1].Point(outside[1340], pos, u, v);
		XMStoreFloat3(&p, pos);
		path.push_back(FreePoint(p, topRaiseHeight));
		for (int i = 0; i <= 15; i++)
		{
			float t = (float)i / 30;
			int inU = 1340;
			for (inU; inU != 1339; inU = (inU + 1) % outside.size())
			{
				int outU = (1340 - inU + 3*outside.size()/2) % outside.size();
				XMFLOAT2 a = outside[inU],
					b = outside[outU];
				XMFLOAT2 ds[] = { {b.x - a.x, b.y - a.y},
				{ b.x - a.x + 1, b.y - a.y + 1 },
				{ b.x - a.x - 1, b.y - a.y - 1 }, };
				XMFLOAT2 d = {INFINITY, INFINITY};
				for (int di = 0; di < 3; di++)
				{
					if (fabsf(ds[di].x) < fabsf(d.x))
						d.x = ds[di].x;
					if (fabsf(ds[di].y) < fabsf(d.y))
						d.y = ds[di].y;
				}

				toru[1].Point({ a.x + t*d.x, a.y + t*d.y }, pos, u, v);
				XMStoreFloat3(&p, pos);
				path.push_back(PathPoint(p));
			}
		}
		path.push_back(FreePoint(p, topRaiseHeight));
	}//if(false)
	{ // head

		auto& outside = outline[8];
		auto& inside = outline[11];
		XMFLOAT2 center = {};
		for (int i = 0; i < inside.size(); i++)
			center = { center.x + inside[i].x, center.y + inside[i].y};
		center = { center.x / inside.size(), center.y / inside.size() };
		std::reverse(inside.begin(), inside.end());
		int in_start = 114;
		int ou_start = 678;
		{
			float angle2 = std::atan2f(inside.back().y - center.y, inside.back().x - center.x);
			for (int i = 0; i < inside.size(); i++)
			{
				float angle = std::atan2f(inside[i].y - center.y, inside[i].x - center.x);
				if (std::fabsf(angle - angle2) > 3)
				{
					in_start = i;
					break;
				}
				angle2 = angle;
			}
			angle2 = std::atan2f(outside.back().y - center.y, outside.back().x - center.x);
			for (int i = 0; i < outside.size(); i++)
			{
				float angle = std::atan2f(outside[i].y - center.y, outside[i].x - center.x);
				if (std::fabsf(angle - angle2) > 3)
				{
					ou_start = i;
					break;
				}
				angle2 = angle;
			}
		}
		XMVECTOR pos, u, v;
		bool firstDrop = true;
		for (int i = 0; i <= 20; i++)
		{
			float t = (float)i / 20;
			int inU = in_start;
			int outU = ou_start;
			int outU2 = ou_start-1;
			float outAngle2 = std::atan2f(outside[outU2].y - center.y, outside[outU2].x - center.x);
			for (inU; inU != in_start-1; inU = (inU + 1) % inside.size())
			{
				float inAngle = std::atan2f(inside[inU].y - center.y, inside[inU].x - center.x);
				float outAngle = std::atan2f(outside[outU].y - center.y, outside[outU].x - center.x);
				while (inAngle > outAngle)
				{
					outU2 = outU;
					outU = (outU + 1) % outside.size();
					outAngle2 = outAngle;
					outAngle = std::atan2f(outside[outU].y - center.y, outside[outU].x - center.x);
				}
				//if (inU % 4 == 0)
				{
					float ft = -(inAngle - outAngle) / (outAngle - outAngle2);
					XMFLOAT2 a = {
						outside[outU].x * (1 - ft) + outside[outU2].x * ft,
						outside[outU].y * (1 - ft) + outside[outU2].y * ft },
						b = inside[inU];
					segm[0].Point({ a.x * t + b.x * (1 - t), a.y * t + b.y * (1 - t) }, pos, u, v);
					XMStoreFloat3(&p, pos);
					if (firstDrop)
					{
						path.push_back(FreePoint(p, topRaiseHeight));
						firstDrop = false;
					}
					path.push_back(PathPoint(p));
				}
			}
		}
		path.push_back(FreePoint(p, topRaiseHeight));
	}
	{ // head2
		auto& bb = segm[1].GetBB();
		float raiseHeight = GetBBRiseHeight(boundingBoxes, 9);
		float size = 0;
		for (int i = 0; i < 3; i++)
			size += bb.hi.m128_f32[i] - bb.lo.m128_f32[i];
		int resolution = 0.2f * size / precision;
		XMVECTOR pos, uvec, vvec;
		bool firstDrop = true;
		auto& o = outline[9];
		bool inside_last = false;
		for (int v = 0; v < resolution; v++)
		{
			inside_last = false;
			int udirection = (v % 2 == 0) ? 1 : -1;
			int ustart = (v % 2 == 0) ? 0 : resolution - 1;
			int uend = (v % 2 == 0) ? resolution - 1 : 0;
			for (int u = ustart; u * udirection <= uend * udirection; u += udirection)
			{
				XMFLOAT2 uv = { (float) u / resolution, (float) v / resolution };
				bool inside = true;
				for (int i = 0; i < o.size(); i++)
				{
					int ip1 = (i + 1) % o.size();
					if (!vecmath::bb2d({0.5,0.01}, uv, o[i], o[ip1]))
						continue;
					if (vecmath::segments2d({ 0.5,0.01 }, uv, o[i], o[ip1]))
					{
						inside = !inside;
					}
				}
				if (inside)
				{
					segm[1].Point(uv, pos, uvec, vvec);
					XMStoreFloat3(&p, pos);
					if (firstDrop)
					{
						path.push_back(FreePoint(p, topRaiseHeight));
						firstDrop = false;
					}
					else if (!inside_last)
						path.push_back(FreePoint(p, raiseHeight));
					path.push_back(PathPoint(p));
				}
				else if (inside_last)
					path.push_back(FreePoint(p, raiseHeight));
				inside_last = inside;
			}
		}
		path.push_back(FreePoint(p, topRaiseHeight));
	}
	for(int k = 0; k<outline.size(); k++)
		for (int i = 0; i < outline[k].size(); i++)
		{
			XMVECTOR pos, u, v;
			int kk = (k == shapes.size()) ? 6 : ((k == shapes.size() + 1) ? 8 : k);
			if (kk < surf.size())
				surf[kk].Point(outline[k][i], pos, u, v);
			else if (kk < surf.size() + toru.size())
				toru[kk - surf.size()].Point(outline[k][i], pos, u, v);
			else
				segm[kk - surf.size() - toru.size()].Point(outline[k][i], pos, u, v);
			XMStoreFloat3(&p, pos);
			if (i == 0)
				path.push_back(FreePoint(p, topRaiseHeight));

			path.push_back(PathPoint(p));
			if (i == outline[k].size() - 1)
				path.push_back(FreePoint(p, topRaiseHeight));
		}
#endif
}
