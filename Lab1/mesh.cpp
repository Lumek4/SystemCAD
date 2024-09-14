#include "mesh.h"
#include"vecmath.h"
using namespace DirectX;

ID3D11InputLayout* VertexPosition::layout = nullptr;
ID3D11InputLayout* VertexPositionUV::layout = nullptr;

Mesh::Mesh()
{
}

Mesh::Mesh(const std::vector<VertexPosition>& vertices,
	const std::vector<unsigned>& indices,
	D3D11_PRIMITIVE_TOPOLOGY topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST)
{
	m_vertexBuffer = DxDevice::Instance().CreateVertexBuffer(vertices);
	m_indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
	m_indexCount = indices.size();
	m_topologyType = topologyType;
	m_layout = VertexPosition::layout;
}

std::unique_ptr<Mesh> Mesh::Torus(int major, int minor, float r1, float r2)
{
	const auto vertices = CreateTorusVertices(major, minor, r1, r2);
	const auto indices = CreateTorusIndices(major, minor);
	auto mesh = std::unique_ptr<Mesh>(new Mesh());
	mesh->m_layout = VertexPositionUV::layout;

	mesh->m_vertexBuffer = DxDevice::Instance().CreateVertexBuffer(vertices);
	mesh->m_indexBuffer = DxDevice::Instance().CreateIndexBuffer(indices);
	mesh->m_indexCount = indices.size();
	mesh->m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

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
	auto mesh = std::unique_ptr<Mesh>(new Mesh());
	mesh->m_layout = VertexPosition::layout;

	mesh->m_vertexBuffer = DxDevice::Instance().CreateVertexBuffer(quad_v);
	mesh->m_indexBuffer = DxDevice::Instance().CreateIndexBuffer(quad_i);
	mesh->m_indexCount = quad_i.size();
	mesh->m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	return mesh;
}

std::unique_ptr<Mesh> Mesh::Point()
{
	auto p_v = std::vector<VertexPosition>{
		{{}},
	};
	auto p_i = std::vector<unsigned>{ 0 };
	auto mesh = std::unique_ptr<Mesh>(new Mesh());
	mesh->m_layout = VertexPosition::layout;

	mesh->m_vertexBuffer = DxDevice::Instance().CreateVertexBuffer(p_v);
	mesh->m_indexBuffer = DxDevice::Instance().CreateIndexBuffer(p_i);
	mesh->m_indexCount = p_i.size();
	mesh->m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	return mesh;
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

	auto mesh = std::unique_ptr<Mesh>(new Mesh());
	mesh->m_layout = VertexPosition::layout;
	mesh->m_vertexBuffer = DxDevice::Instance().CreateVertexBuffer(verts);
	mesh->m_indexBuffer = DxDevice::Instance().CreateIndexBuffer(ind);
	mesh->m_indexCount = ind.size();
	mesh->m_topologyType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	return mesh;
}


unsigned Mesh::SetBuffers() const
{
	auto& device = DxDevice::Instance();
	device.context()->IASetInputLayout(m_layout);
	device.context()->IASetPrimitiveTopology(m_topologyType);
	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { (m_layout == VertexPosition::layout) ? sizeof(VertexPosition) :
		(m_layout == VertexPositionUV::layout ? sizeof(VertexPositionUV) :
		1)
	};
	UINT offsets[] = { 0 };
	device.context()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	device.context()->IASetIndexBuffer(m_indexBuffer.get(),
		DXGI_FORMAT_R32_UINT, 0);
	return m_indexCount;
}

std::vector<VertexPositionUV> Mesh::CreateTorusVertices(int major, int minor, float r1, float r2)
{
	//std::swap(r1, r2);
	std::vector<VertexPositionUV> verts;
	for (int i = 0; i <= major; i++)
		for (int j = 0; j <= minor; j++)
		{
			float u = i * XM_2PI / major - XM_PI;
			float v = j * XM_2PI / minor - XM_PI;
			float uu = i * 1.0f / major;
			float vv = j * 1.0f / minor;
			verts.push_back(VertexPositionUV({
				(r1 + r2 * cosf(v)) * cosf(u),
				(r1 + r2 * cosf(v))* sinf(u),
				r2* sinf(v),
				},
				{uu,vv}));
		}
	return verts;
}

std::vector<unsigned> Mesh::CreateTorusIndices(int major, int minor)
{
	std::vector<unsigned> ind;
	for (int i = 0; i < major; i++)
		for (int j = 0; j < minor; j++)
		{
			ind.push_back(i * (minor+1) + j);
			ind.push_back(i * (minor+1) + j + 1);
		}
	for (int i = 0; i < major; i++)
		for (int j = 0; j < minor; j++)
		{
			ind.push_back(i * (minor+1) + j);
			ind.push_back((i + 1) * (minor+1) + j);
		}
	return ind;
}