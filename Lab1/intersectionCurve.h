#pragma once

#include"component.h"
#include"mesh.h"
#include"pointSource.h"
#include"event.hpp"

class IntersectionCurve : public Component
{
public:
	IntersectionCurve(Entity& owner);
	const char* Name() const override { return "IntersectionCurve"; }
	//Mesh* GetMesh();
	void InitTextures(DxDevice& device, int resolution);
	std::vector<DirectX::XMFLOAT2> pointsA;
	std::vector<DirectX::XMFLOAT2> pointsB;
	Entity* a;
	Entity* b;
	DirectX::XMINT2 wrapModeA, wrapModeB;
	mini::dx_ptr<ID3D11ShaderResourceView> texvA;
	mini::dx_ptr<ID3D11ShaderResourceView> texvB;
private:
	bool m_modified = true;
	mini::dx_ptr<ID3D11Texture2D> texA;
	mini::dx_ptr<ID3D11Texture2D> texB;

	std::unique_ptr<Mesh> m;
};