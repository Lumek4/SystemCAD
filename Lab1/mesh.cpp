#include "mesh.h"
#include"vecmath.h"
using namespace DirectX;

ID3D11InputLayout* VertexPosition::layout = nullptr;

Mesh::Mesh(const std::vector<VertexPosition>& vertices, const std::vector<unsigned>& indices, D3D11_PRIMITIVE_TOPOLOGY topology)
{
	auto& device = DxDevice::Instance();
	m_vertexBuffer = device.CreateVertexBuffer(vertices);
	m_indexBuffer = device.CreateIndexBuffer(indices);
	m_indexCount = indices.size();
	topologyType = topology;
	
}

std::unique_ptr<Mesh> Mesh::Torus(int major, int minor, float r1, float r2)
{
	const auto vertices = CreateTorusVertices(major, minor, r1, r2);
	const auto indices = CreateTorusIndices(major, minor);
	auto mesh = std::make_unique<Mesh>(vertices, indices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	return mesh;
}

std::unique_ptr<Mesh> Mesh::Quad(float size)
{

	auto quad_v = std::vector<VertexPosition>{
		{{-size / 2, -size / 2, 0}},
		{{-size / 2, +size / 2, 0}},
		{{+size / 2, +size / 2, 0}},
		{{+size / 2, -size / 2, 0}},
	};
	auto quad_i = std::vector<unsigned>{ 0,1,2,0,2,3 };

	return std::make_unique<Mesh>(quad_v, quad_i,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

std::unique_ptr<Mesh> Mesh::Point()
{
	auto p_v = std::vector<VertexPosition>{
		{{}},
	};
	auto p_i = std::vector<unsigned>{ 0 };
	return std::make_unique<Mesh>(p_v, p_i,
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

std::unique_ptr<Mesh> Mesh::Grid(int size)
{
	auto verts = std::vector<VertexPosition>{};
	auto ind = std::vector<unsigned>{};

	int j = 0;
	for (int i = -size; i <= size; i++)
	{
		float fi = (float)i, fsize = (float)size;
		verts.push_back({ {fi,-fsize,0} });
		verts.push_back({ {fi,+fsize,0} });
		ind.push_back(j++);
		ind.push_back(j++);

		verts.push_back({ {-fsize,fi,0} });
		verts.push_back({ {+fsize,fi,0} });
		ind.push_back(j++);
		ind.push_back(j++);
	}

	return std::make_unique<Mesh>(verts, ind,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}


unsigned Mesh::SetBuffers() const
{
	auto& device = DxDevice::Instance();
	device.context()->IASetInputLayout(VertexPosition::layout);
	device.context()->IASetPrimitiveTopology(topologyType);
	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPosition) };
	UINT offsets[] = { 0 };
	device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	device.context()->IASetIndexBuffer(m_indexBuffer.get(),
		DXGI_FORMAT_R32_UINT, 0);
	return m_indexCount;
}

std::vector<VertexPosition> Mesh::CreateTorusVertices(int major, int minor, float r1, float r2)
{
	swap(r1, r2);
	std::vector<VertexPosition> verts;
	for (int i = 0; i < major; i++)
		for (int j = 0; j < minor; j++)
		{
			float u = i * g_XMTwoPi.f[0] / major - g_XMPi.f[0];
			float v = j * g_XMTwoPi.f[0] / minor - g_XMPi.f[0];
			verts.push_back(VertexPosition({
				(r1 + r2 * cosf(v)) * cosf(u),
				(r1 + r2 * cosf(v))* sinf(u),
				r2* sinf(v),
				}));
		}
	return verts;
}

std::vector<unsigned> Mesh::CreateTorusIndices(int major, int minor)
{
	std::vector<unsigned> ind;
	for (int i = 0; i < major; i++)
		for (int j = 0; j < minor; j++)
		{
			ind.push_back(i * minor + j);
			ind.push_back(i * minor + ((j + 1) % minor));
		}
	for (int i = 0; i < major; i++)
		for (int j = 0; j < minor; j++)
		{
			ind.push_back(i * minor + j);
			ind.push_back(((i + 1) % major) * minor + j);
		}
	return ind;
}