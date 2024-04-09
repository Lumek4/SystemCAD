#pragma once
#include<DirectXMath.h>
#include<vector>
#include<memory>
#include"myMat.h"
#include"dxDevice.h"
#include"component.h"
#include<d3d11.h>

struct VertexPosition
{
	VertexPosition(DirectX::XMFLOAT3 position) : position(position) {}
	DirectX::XMFLOAT3 position;
	static ID3D11InputLayout* layout;
};
#define RESTART_STRIP 0xffffffff

class Mesh
{
public:
	Mesh(const std::vector<VertexPosition>& vertices, const std::vector<unsigned>& indices, D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	static std::unique_ptr<Mesh> Torus(int major, int minor, float r1, float r2);
	static std::unique_ptr<Mesh> Quad(float size);
	static std::unique_ptr<Mesh> Point();
	static std::unique_ptr<Mesh> Grid(int size);
public:
	unsigned SetBuffers() const;
private:
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	unsigned m_indexCount;
	
	D3D11_PRIMITIVE_TOPOLOGY topologyType;
private:
	static std::vector<VertexPosition> CreateTorusVertices(int major, int minor, float r1, float r2);
	static std::vector<unsigned> CreateTorusIndices(int major, int minor);
};
