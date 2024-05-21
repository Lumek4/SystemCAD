#include "gregoryPatch.h"
#include"catalogue.hpp"
#include"bicubicSegment.h"
#include<algorithm>
#include"vecmath.h"

using namespace DirectX;

GregoryPatch::GregoryPatch(Entity& owner)
	:Component(ComponentConstructorArgs(GregoryPatch)),
	neighbors(), neighborSide(), neighborReverse()
{
	NeighborModified = [this]() {
		modified = wireModified = true;
	};
}

Mesh* GregoryPatch::GetMesh()
{
	assert(neighbors[0] != nullptr && neighbors[1] != nullptr && neighbors[2] != nullptr);
	if (!modified)
		return m.get();
	modified = false;

	std::vector<VertexPosition> vv;
	std::array<int, 8> ibounds[3];
	std::array<XMFLOAT3, 8> bounds[3];
	for (int i = 0; i < 3; i++)
	{
		auto* bs = neighbors[i]->GetComponent<BicubicSegment>();
		ibounds[i] = bs->GetBoundary(neighborSide[i]);

		for(int j = 0; j<8; j++)
			bounds[i][j] = bs->GetSource()->GetPoint(ibounds[i][j]);
	}
	XMFLOAT3 P3[3], P2[3], P1[3], P, Q[3], a0[3], b0[3], a3[3], b3[3];

	for (int i = 0; i < 3; i++)
	{
		XMFLOAT3 c1[3], c2[2];
		c1[0] = (bounds[i][0] + bounds[i][1])*0.5f;
		c1[1] = (bounds[i][1] + bounds[i][2])*0.5f;
		c1[2] = (bounds[i][2] + bounds[i][3])*0.5f;

		c2[0] = (c1[0] + c1[1])*0.5f;
		c2[1] = (c1[1] + c1[2])*0.5f;

		P3[i] = (c2[0] + c2[1]) * 0.5f;
		a0[i] = c2[1] - P3[i];
		b0[i] = c2[0] - P3[i];

		XMFLOAT3 cd1[3], cd2[2], cd3;

		cd1[0] = (bounds[i][4] + bounds[i][5])*0.5f;
		cd1[1] = (bounds[i][5] + bounds[i][6])*0.5f;
		cd1[2] = (bounds[i][6] + bounds[i][7])*0.5f;

		cd2[0] = (cd1[0] + cd1[1])*0.5f;
		cd2[1] = (cd1[1] + cd1[2])*0.5f;

		cd3 = (cd2[0] + cd2[1])*0.5f;

		P2[i] = 2*P3[i] - cd3;


		vv.push_back(bounds[i][3]);
		vv.push_back(c1[2]);
		vv.push_back(c2[1]);
		vv.push_back(2*c1[2] - cd1[2]);
		vv.push_back(2*c2[1] - cd2[1]);


		vv.push_back(P3[i]);
		vv.push_back(c2[0]);
		vv.push_back(c1[0]);
		vv.push_back(2*c1[0] - cd1[0]);
		vv.push_back(2*c2[0] - cd2[0]);

	}
	// vv.size() == 30
	for (int i = 0; i < 3; i++)
		Q[i] = P3[i] + 3.0f / 2 * (P2[i] - P3[i]);

	P = (Q[0] + Q[1] + Q[2]) * (1.0f / 3);

	for (int i = 0; i < 3; i++)
		P1[i] = P + 2.0 / 3 * (Q[i] - P);

	for (int i = 0; i < 3; i++)
	{
		a3[i] = P1[(i - 1 + 3) % 3] - P;
		b3[i] = P1[(i + 1) % 3] - P;
	}
	for (int i = 0; i < 3; i++)
	{
		vv.push_back(P2[i] + a0[i]);
		vv.push_back(P2[i]);
		vv.push_back(P2[i] + b0[i]);
	}
	// vv.size() == 39
	for (int i = 0; i < 3; i++)
	{
		vv.push_back(P1[i] + (a0[i] + a3[i])*0.5f);
		vv.push_back(P1[i]);
		vv.push_back(P1[i] + (b0[i] + b3[i])*0.5f);
	}
	// vv.size() == 48
	vv.push_back(P);
	// vv.size() == 49
	/*
	0     1     2     5     6     7    10
	      3     4          8     9   
	             30 31 32        13        
	27 29       ---------         11
	          39 40 41                          
	                      14              
	26 28  47   48   42       12
	    38|46     43 / 33       
	    37|45  44 / 34         
	25  36|      35  15
	        
	  24      18                         
	22          16
	                           
	  23 19                      
	21    17
	                 
	                 
	20
	*/
	/*
    
	0   1   2   5
	             
	17			6

	16			7

	15	12	11	10
	*/
	static const std::vector<unsigned> ii =
	{
		 0, 1, 2,  3, 4,
		 5,31,40, 30,39,
		48,46,37, 47,38,
		25,26,27, 28,29,

		 5, 6, 7,  8, 9,
		10,11,12, 13,14,
		15,34,43, 33,42,
		48,40,31, 41,32,

		48,43,34, 44,35,
		15,16,17, 18,19,
		20,21,22, 23,24,
		25,37,46, 36,45
	};

	m.reset(new Mesh(vv, ii, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST));

	wireModified = false;
	static const std::vector<unsigned> iiw =
	{
		 0, 1, 2, 5,31,40,48,46,37,25,26,27, 0, RESTART_STRIP,
		 5, 6, 7,10,11,12,15,34,43,48, RESTART_STRIP,
		15,16,17,20,21,22,25, RESTART_STRIP,
		 1, 3, RESTART_STRIP,
		 2, 4, RESTART_STRIP,
		 6, 9, RESTART_STRIP,
		 7, 8, RESTART_STRIP,
		11,13, RESTART_STRIP,
		12,14, RESTART_STRIP,
		16,19, RESTART_STRIP,
		17,18, RESTART_STRIP,
		21,23, RESTART_STRIP,
		22,24, RESTART_STRIP,
		26,29, RESTART_STRIP,
		27,28, RESTART_STRIP,
		30,31,32, RESTART_STRIP,
		33,34,35, RESTART_STRIP,
		36,37,38, RESTART_STRIP,
		39,40,41, RESTART_STRIP,
		42,43,44, RESTART_STRIP,
		45,46,47, RESTART_STRIP
	};
	wm.reset(new Mesh(vv, iiw, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP));

	return m.get();
}

Mesh* GregoryPatch::GetWireMesh()
{
	if (!wireModified)
		return wm.get();
	wireModified = false;

	auto* m = GetMesh();

	return wm.get();;
}
