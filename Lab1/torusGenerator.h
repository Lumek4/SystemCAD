#pragma once

#include"component.h"
#include"mesh.h"
#include"event.hpp"


class TorusGenerator : public Component
{
public:
	struct MeshData
	{
		DirectX::XMINT2 division{ 10,10 };
		DirectX::XMFLOAT2 radii{ 2, 1 };
	};
public:
	TorusGenerator(Entity& owner);
	const char* Name() const override { return "TorusGenerator"; }
	Mesh* GetMesh();

	MeshData m_meshData{};
	const MeshData& GetData() const { return m_meshData; }
	void SetData(const MeshData& data);

private:
	bool modified = true;
	static Event<void>::Reaction::Function NeedsRedrawFunction;
	Event<void>::Reaction NeedsRedraw;

	std::unique_ptr<Mesh> m;
};