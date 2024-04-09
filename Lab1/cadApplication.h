#pragma once
#include"dxApplication.h"
#include"entity.hpp"
#include"camera.h"
#include<DirectXMath.h>
#include"mesh.h"
#include"sceneCursor.h"
#include"transform.h"
#include"all_components.h"


class CadApplication : public DxApplication
{
public:
	explicit CadApplication(HINSTANCE hInstance);
	~CadApplication();
private:
	enum class COLORING {
		REGULAR, VIRTUAL_POINT
	};
	void SetColor(Entity* object, COLORING c = COLORING::REGULAR);
	void SetModelMatrix(Transform* object);
	void OnResize() override;
private:
	void GUI() override;
	void Update() override;
	void Render() override;
private:
	const float fov = DirectX::XM_PI / 4;
	const float n = 0.1f, f = 500.0f;

	const float turnRate = 0.01f;
	const float scaleRate = 0.05f;
	const float moveRate = 0.001f;
	Folder* mainFolder = Entity::New()->AddComponent<Folder>();
	//std::vector<Entity*> objectList;

	//SceneCursor sceneCursor;

	//DirectX::XMFLOAT3 translation{}, rotationAxis{ 0,0,1 };
	//float scaleFactor{ 1 }, rotationAngle{};
	//enum transform_type
	//{
	//	center, cursor, local, highlight, global
	//} ttype;
	//DirectX::XMFLOAT3 selectionCenter;

private:
	std::unique_ptr<Mesh> point;
	const float pointSize = 0.02f;
	const float cursorSize = 0.1f;
	const float groupCenterSize = 0.05f;
	DirectX::XMFLOAT4 m_gizmoBuffer{};
	void DrawAxes(DirectX::XMFLOAT3 location, float size);
	void DrawGrid();
	void DrawSingleGrid(float scale, float visibility);
	int surfDetail = 4;

	std::unique_ptr<Mesh> torus;
	std::unique_ptr<Mesh> grid;
private:


	mini::dx_ptr<ID3D11Buffer> m_cbColor;
private:
	mini::dx_ptr<ID3D11DepthStencilState> m_depthStateOver;
	mini::dx_ptr<ID3D11RasterizerState> m_rasterizerNoCull;

	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11PixelShader> m_wirePixelShader;
	mini::dx_ptr<ID3D11GeometryShader> m_pointGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_cursorGeometryShader;

	mini::dx_ptr<ID3D11VertexShader> m_bezierVertexShader;
	mini::dx_ptr<ID3D11HullShader> m_bezierHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_bezierDomainShader;

	mini::dx_ptr<ID3D11HullShader> m_bicubicHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_bicubicDomainShader;
	mini::dx_ptr<ID3D11HullShader> m_gregHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_gregDomainShader;
	mini::dx_ptr<ID3D11GeometryShader> m_bicubicGridGeometryShader;
	mini::dx_ptr<ID3D11PixelShader> m_surfacePixelShader;

	std::vector<mini::dx_ptr<ID3D11InputLayout>> m_layout;


	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_invview;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT4X4 m_invproj;
	mini::dx_ptr<ID3D11Buffer> m_cbworld;
	mini::dx_ptr<ID3D11Buffer> m_cbview;
	mini::dx_ptr<ID3D11Buffer> m_cbproj;

	mini::dx_ptr<ID3D11Buffer> m_cbSurfMode;
	mini::dx_ptr<ID3D11Buffer> m_cbgizmos;
};