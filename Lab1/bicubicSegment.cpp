#include "bicubicSegment.h"
#include "bicubicSurface.h"
#include<algorithm>
using namespace DirectX;


const int BicubicSegment::corners[4]{ 0,3,15,12 };

BicubicSegment::BicubicSegment(Entity& owner)
	:Component(ComponentConstructorArgs(BicubicSegment))
{
	//pointSource = owner.GetComponent<VPointCollection>();
	//if (!pointSource)
	//	pointSource = &RequireComponent(PointSource);
	NeedsRedraw_PointCollection = [this](NeedRedrawEventData d)
	{
		if (d.ind == -1)
		{
			wireModified = modified = true;
		}
		else
		{
			auto it = std::find(indices.begin(), indices.end(), d.ind);
			if (it != indices.end())
				wireModified = modified = true;
		}
	};
	PointDeleted = [this](int deleted)
	{
		for (int i = 0; i < 16; i++)
		{
			if (indices[i] == deleted)
			{
				this->owner.Delete();
				return;
			}
			else if (indices[i] > deleted)
				indices[i]--;
		}
	};
	TransferRelation = [this](DirectX::XMINT2 xy)
	{
		int deleted = xy.x, transferred = xy.y;
		auto& segs = surface->GetSegments();
		for (int i = 0; i < 4; i++)
		{
			if (indices[corners[i]]!= deleted ||
				(neighbors[i] != nullptr && neighbors[(i-1+4)%4] != nullptr))
				continue;
			for (int segInd = 0; segInd < segs.size(); segInd++)
			{
				if (segs[segInd] == this)
					break;
				for (int ii = 0; ii < 4; ii++)
				{
					if (deleted != segs[segInd]->indices[corners[ii]])
						continue;

					for (int j = -1; j <= 1; j += 2)
					for (int jj = -1; jj <= 1; jj += 2)
					{
						if (indices[corners[(i + j + 4) % 4]] != segs[segInd]->indices[corners[(ii + jj + 4) % 4]])
							continue;
						if (segs[segInd]->neighbors[(ii + (jj - 1) / 2 + 4) % 4] != nullptr)
							continue;

						ccwNeighbor[(i + (j - 1) / 2 + 4) % 4] =
							segs[segInd]->ccwNeighbor[(ii + (jj - 1) / 2 + 4) % 4] = j != jj;
						neighbors[(i + (j - 1) / 2 + 4) % 4] = &segs[segInd]->owner;
						segs[segInd]->neighbors[(ii + (jj - 1) / 2 + 4) % 4] = &this->owner;
					}
						
				}
			}
		}

		for (int i = 0; i < 16; i++)
			if (indices[i] == deleted)
			{
				indices[i] = transferred;
			}
	};
	ReleaseNeighbors = [this](Entity* e)
	{
		for (int i = 0; i < 4; i++)
		{
			if (neighbors[i] == nullptr)
				continue;

			auto* bs = neighbors[i]->GetComponent<BicubicSegment>();
			if (bs == nullptr)
				continue;
			for (int j = 0; j < bs->neighbors.size(); j++)
				if (bs->neighbors[j] == &this->owner)
					bs->neighbors[j] = nullptr;
		}
	};
	owner.preDelete += ReleaseNeighbors;
	/*DeletePoints = [this](Entity* e)
	{
		int c = pointSource->GetCount();
		for (int i = 0; i < c; i++)
		{

		}
	};
	owner.preDelete += DeletePoints;*/
}
//
//void BicubicSegment::Set(DirectX::XMINT2 start, int stride)
//{
//	assert(stride != -1 && "Segment already set!");
//	this->start = start;
//	this->stride = stride;
//}

std::array<int, 8> BicubicSegment::GetBoundary(int side)
{
	const int ind[4][8] =
	{
		{0,1,2,3,4,5,6,7},
		{3,7,11,15,2,6,10,14},
		{15,14,13,12,11,10,9,8},
		{12,8,4,0,13,9,5,1}
	};
	if (deBoorMode)
		assert(false && "Can't get boundary of C2 segment.");

	std::array<int, 8> ret;
	for (int i = 0; i < 8; i++)
		ret[i] = indices[ind[side][i]];
	return ret;
	/*if (deBoorMode)
	{
		XMVECTOR extBezier[16];
		for (int y = 0; y < 4; y++)
		{
			extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 2.0 / 3);
			extBezier[1 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 1.0 / 3);
			extBezier[2 + y * 4] = XMVectorLerp(p[1 + y * 4], p[2 + y * 4], 2.0 / 3);
			extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 1.0 / 3);
		}
		for (int x = 0; x < 4; x++)
		{
			p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 2.0 / 3);
			p[x + 1 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 1.0 / 3);
			p[x + 2 * 4] = XMVectorLerp(extBezier[x + 1 * 4], extBezier[x + 2 * 4], 2.0 / 3);
			p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 1.0 / 3);
		}

		for (int y = 0; y < 4; y++)
		{
			extBezier[0 + y * 4] = XMVectorLerp(p[0 + y * 4], p[1 + y * 4], 0.5);
			extBezier[1 + y * 4] = p[1 + y * 4];
			extBezier[2 + y * 4] = p[2 + y * 4];
			extBezier[3 + y * 4] = XMVectorLerp(p[2 + y * 4], p[3 + y * 4], 0.5);
		}
		for (int x = 0; x < 4; x++)
		{
			p[x + 0 * 4] = XMVectorLerp(extBezier[x + 0 * 4], extBezier[x + 1 * 4], 0.5);
			p[x + 1 * 4] = extBezier[x + 1 * 4];
			p[x + 2 * 4] = extBezier[x + 2 * 4];
			p[x + 3 * 4] = XMVectorLerp(extBezier[x + 2 * 4], extBezier[x + 3 * 4], 0.5);
		}
	}*/
}

Mesh* BicubicSegment::GetMesh()
{
	assert(pointSource != nullptr && "Point source not set");
	if (!modified)
		return m.get();
	onModified.Notify();
	modified = false;

	std::vector<VertexPosition> vv;
	for (int i = 0; i < 16; i++)
		vv.emplace_back(pointSource->GetPoint(indices[i]));

	std::vector<unsigned> ii;
	for (int i = 0; i < 16; i++)
		ii.emplace_back(i);

	m.reset(new Mesh(vv, ii, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST));
	return m.get();

}

Mesh* BicubicSegment::GetWireMesh()
{
	assert(pointSource != nullptr && "Point source not set");
	if (!wireModified)
		return wm.get();
	wireModified = false;

	std::vector<VertexPosition> vv;
	for (int i = 0; i < 16; i++)
		vv.emplace_back(pointSource->GetPoint(indices[i]));

	std::vector<unsigned> ii;
	for (int i = 0; i < 4; i++)
	{
		ii.emplace_back(0 + i * 4);
		ii.emplace_back(1 + i * 4);
		ii.emplace_back(1 + i * 4);
		ii.emplace_back(2 + i * 4);
		ii.emplace_back(2 + i * 4);
		ii.emplace_back(3 + i * 4);

		ii.emplace_back(i + 0 * 4);
		ii.emplace_back(i + 1 * 4);
		ii.emplace_back(i + 1 * 4);
		ii.emplace_back(i + 2 * 4);
		ii.emplace_back(i + 2 * 4);
		ii.emplace_back(i + 3 * 4);
	}

	wm.reset(new Mesh(vv, ii, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST));
	return wm.get();
}

void BicubicSegment::SetSource(PointSource* ps)
{
	assert(pointSource == nullptr && "Point source already set");
	pointSource = ps;
	owner.Register(pointSource->NeedRedrawEvent(), NeedsRedraw_PointCollection);
	if ((PointCollection*)ps)
	{
		surface = ((PointCollection*)ps)->owner.GetComponent<BicubicSurface>();
		owner.Register(((PointCollection*)pointSource)->onRemove, PointDeleted);
		owner.Register(((PointCollection*)pointSource)->onTransferRelations, TransferRelation);
	}
}
