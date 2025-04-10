#pragma once
#include"dxApplication.h"
#include"entity.hpp"
#include"camera.h"
#include<DirectXMath.h>
#include"mesh.h"
#include"sceneCursor.h"
#include"transform.h"
#include"all_components.h"


extern enum class SaveResult;
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
	void Draw();
private:
	const float fov = DirectX::XM_PI / 4;
	const float n = 0.1f, f = 500.0f;

	const float turnRate = 0.01f;
	const float scaleRate = 0.05f;
	const float moveRate = 0.001f;
	Folder* mainFolder = nullptr;
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
	float surfaceUVColoring = 0.0f;
	bool anaglyph = false;
	float eyeDistance = 0.1f;
	float planeDistance = 10.0f;
	DirectX::XMFLOAT4 leftTint = { 0.7f,0,0,1 };
	DirectX::XMFLOAT4 rightTint = { 0,0.25f,1,1 };

	std::unique_ptr<Mesh> point;
	const float pointSize = 0.02f;
	const float cursorSize = 0.1f;
	const float groupCenterSize = 0.05f;
	DirectX::XMFLOAT4 m_gizmoBuffer{};
	void DrawAxes(DirectX::XMFLOAT3 location, float size);
	void DrawGrid();
	void DrawSingleGrid(float scale, float visibility);
	int surfDetail = 8;

	std::unique_ptr<Mesh> torus;
	std::unique_ptr<Mesh> grid;
private:


	mini::dx_ptr<ID3D11Buffer> m_cbColor;
private:
	mini::dx_ptr<ID3D11DepthStencilState> m_depthStateOver;
	mini::dx_ptr<ID3D11RasterizerState> m_rasterizerNoCull;

	mini::dx_ptr<ID3D11VertexShader> m_vs_uvs;

	mini::dx_ptr<ID3D11VertexShader> m_vs_transform;
	mini::dx_ptr<ID3D11PixelShader> m_ps_model;
	mini::dx_ptr<ID3D11PixelShader> m_ps_shape;
	mini::dx_ptr<ID3D11GeometryShader> m_pointGeometryShader;
	mini::dx_ptr<ID3D11GeometryShader> m_cursorGeometryShader;

	mini::dx_ptr<ID3D11VertexShader> m_vs_noTransform;
	mini::dx_ptr<ID3D11HullShader> m_bezierHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_bezierDomainShader;

	mini::dx_ptr<ID3D11HullShader> m_bicubicHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_bicubicDomainShader;
	mini::dx_ptr<ID3D11HullShader> m_gregHullShader;
	mini::dx_ptr<ID3D11DomainShader> m_gregDomainShader;
	mini::dx_ptr<ID3D11GeometryShader> m_bicubicGridGeometryShader;
	//mini::dx_ptr<ID3D11PixelShader> m_surfacePixelShader;

	mini::dx_ptr<ID3D11BlendState> m_blendAdd;
	std::vector<mini::dx_ptr<ID3D11InputLayout>> m_layout;


	//DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_invview;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT4X4 m_invproj;
	mini::dx_ptr<ID3D11Buffer> m_cbworld;
	mini::dx_ptr<ID3D11Buffer> m_cbview;
	mini::dx_ptr<ID3D11Buffer> m_cbproj;

	mini::dx_ptr<ID3D11Buffer> m_cbSurfMode;
	mini::dx_ptr<ID3D11Buffer> m_cbgizmos;
	mini::dx_ptr<ID3D11Buffer> m_cbTint;

	friend SaveResult save(const CadApplication& app, const char* filepath);
	friend SaveResult load(CadApplication& app, const char* filepath);
};