#include "torusGenerator.h"
#include"catalogue.hpp"
#include"vecmath.h"

void TorusGenerator::NeedsRedrawFunction(void* arg)
{
	auto _this = (TorusGenerator*)arg;

}

TorusGenerator::TorusGenerator(Entity& owner)
	:Component(ComponentConstructorArgs(TorusGenerator)),
	NeedsRedraw(this, NeedsRedrawFunction)
{
}

Mesh* TorusGenerator::GetMesh()
{
	if (!modified)
		return m.get();
	modified = false;

	m = Mesh::Torus(
		m_meshData.division.x, m_meshData.division.y,
		m_meshData.radii.x, m_meshData.radii.y);
	return m.get();
}

void TorusGenerator::SetData(const MeshData& data)
{
	modified |=
		data.division != m_meshData.division ||
		data.radii != m_meshData.radii;
	m_meshData = data;
}
