#include "cadApplication.h"
#include<windowsx.h>
#include"imgui/imgui_impl_dx11.h"
#include"imgui/imgui_impl_win32.h"
#include"all_components.h"
#include"vecmath.h"
#include"myGui.h"
#include"colorPaletteh.h"
#include"entityPresets.h"
#include"saving/saving.h"

using namespace mini;
using namespace DirectX;

CadApplication::CadApplication(HINSTANCE hInstance)
	:DxApplication(hInstance)
{


	

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
		{
		const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
		const auto wirePSBytes = DxDevice::LoadByteCode(L"wirePS.cso");
		const auto pointgsBytes = DxDevice::LoadByteCode(L"pointgs.cso");
		const auto cursorgsBytes = DxDevice::LoadByteCode(L"curgs.cso");
		m_vertexShader = m_device.CreateVertexShader(vsBytes);
		m_pixelShader = m_device.CreatePixelShader(psBytes);
		m_wirePixelShader = m_device.CreatePixelShader(wirePSBytes);
		m_pointGeometryShader = m_device.CreateGeometryShader(pointgsBytes);
		m_cursorGeometryShader = m_device.CreateGeometryShader(cursorgsBytes);
	}
	{

		const auto bezierVSBytes = DxDevice::LoadByteCode(L"bezierVS.cso");
		const auto bezierHSBytes = DxDevice::LoadByteCode(L"bezierHS.cso");
		const auto bezierDSBytes = DxDevice::LoadByteCode(L"bezierDS.cso");
		m_bezierVertexShader = m_device.CreateVertexShader(bezierVSBytes);
		m_bezierHullShader = m_device.CreateHullShader(bezierHSBytes);
		m_bezierDomainShader = m_device.CreateDomainShader(bezierDSBytes);
	}
	{

		const auto bicubicHSBytes = DxDevice::LoadByteCode(L"bicubicHS.cso");
		const auto bicubicDSBytes = DxDevice::LoadByteCode(L"bicubicDS.cso");
		const auto bicubicGridGSBytes = DxDevice::LoadByteCode(L"bicubicGridGS.cso");
		const auto surfpsBytes = DxDevice::LoadByteCode(L"surfPS.cso");
		m_bicubicHullShader = m_device.CreateHullShader(bicubicHSBytes);
		m_bicubicDomainShader = m_device.CreateDomainShader(bicubicDSBytes);
		m_surfacePixelShader = m_device.CreatePixelShader(surfpsBytes);
		m_bicubicGridGeometryShader = m_device.CreateGeometryShader(bicubicGridGSBytes);
	}
	{

		const auto gregHSBytes = DxDevice::LoadByteCode(L"gregHS.cso");
		const auto gregDSBytes = DxDevice::LoadByteCode(L"gregDS.cso");
		m_gregHullShader = m_device.CreateHullShader(gregHSBytes);
		m_gregDomainShader = m_device.CreateDomainShader(gregDSBytes);
	}

	auto statedesc = DepthStencilDescription{};
	statedesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	m_depthStateOver = m_device.CreateDepthStencilState(statedesc);

	auto rstatedesc = RasterizerStateDescription{};
	rstatedesc.FillMode = D3D11_FILL_WIREFRAME;
	m_rasterizerNoCull = m_device.CreateRasterizerState(rstatedesc);

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout.push_back(m_device.CreateInputLayout(elements, vsBytes));
	VertexPosition::layout = m_layout[m_layout.size() - 1].get();
	torus = Mesh::Torus(10, 10, 2, 1);
	point = Mesh::Point();
	grid = Mesh::Grid(100);


	m_cbworld = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbview = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbproj = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();

	m_cbgizmos = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbColor = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbSurfMode = m_device.CreateConstantBuffer<DirectX::XMINT4>();

	OnResize();

	Camera::mainCamera = std::make_unique<Camera>(m_device);
	Camera::mainCamera->Rotate({ 0, 2 * XM_PI / 3 });
	//shapes.push_back(ImplicitShape::Ellipsoid(radii));

	mainFolder = Entity::New()->AddComponent<Folder>();
	mainFolder->Add(EntityPresets::Point({}));
}

CadApplication::~CadApplication()
{
	Entity::Clear();
}

void CadApplication::GUI()
{
	auto viewportDock = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
	
	static SaveResult saveRes = SaveResult::Undefined;
	if (saveRes == SaveResult::Undefined)
		//saveRes = load(*this, "../gregory_test_2024.json");
		saveRes = load(*this, "../referenceScene.json");

	ImGui::Begin("Options");
	MyGui::GuiOptions();
	ImGui::SliderInt("Detail Level of Surfaces", &surfDetail, 1, 255);
	ImGui::End(); // Options

	ImGui::Begin("Torus model data");
	static MyGui::TorusMeshData torusCreationData{};
	MyGui::TorusMeshWidget(torus, torusCreationData);
	ImGui::End(); // Torus model data

	ImGui::Begin("Entities");
	MyGui::EntityListWidget(mainFolder->Get());
	ImGui::End(); // Entities

	ImGui::Begin("Toolbox");
	ImVec2 buttonDims(70, 40);
	if (ImGui::Button("+Torus", buttonDims))
		mainFolder->Add(EntityPresets::Model(
			SceneCursor::instance.GetWorld(), torus.get()
		));
	MyGui::SameLineIfFits(buttonDims.x);
	if (ImGui::Button("+(P)oint", buttonDims) || ImGui::IsKeyPressed(ImGuiKey_P, false))
	{
		auto p = EntityPresets::Point(
			SceneCursor::instance.GetWorld()
		);
		mainFolder->Add(p);
		auto collections = Entity::GetSelected<PointCollection>();
		if (!collections.empty())
			collections.back()->Add(p);
	}
	MyGui::SameLineIfFits(buttonDims.x);
	if (ImGui::Button("+Bezier", buttonDims))
		mainFolder->Add(EntityPresets::BezierCurveObject(
			Entity::selection
		));
	MyGui::SameLineIfFits(buttonDims.x);
	if (ImGui::Button("+Spline", buttonDims))
		if (Entity::selection.size() >= 4)
			mainFolder->Add(EntityPresets::SplineCurve(
				Entity::selection
			));
	float f = ImGui::GetCursorPosX();
	MyGui::SameLineIfFits(buttonDims.x);
	float fd = ImGui::GetCursorPosX();
	if (ImGui::Button("+Interp.\nCurve", buttonDims))
		if (Entity::selection.size() >= 4)
			mainFolder->Add(EntityPresets::InterpCurve(
				Entity::selection
			));
	MyGui::SameLineIfFits(buttonDims.x);
	{
		static BicubicSurfaceParams bicubic;
		bool modify = false, create = false;

		if (ImGui::Button("+Bicubic\nSurface", buttonDims))
			MyGui::ShowBicubicSurfacePopup(bicubic);

		MyGui::BicubicSurfacePopup(bicubic, modify, create);
		if (create)
		{
			std::vector<Entity*> pts{}, segs{};
			mainFolder->Add(EntityPresets::BicubicSurfaceObject(
				SceneCursor::instance.GetWorld(), bicubic, pts, segs
			));
			for (int i = 0; i < segs.size(); i++)
				mainFolder->Add(segs[i]);
			for (int i = 0; i < pts.size(); i++)
				mainFolder->Add(pts[i]);
		}
	}
	MyGui::SameLineIfFits(buttonDims.x);
	{
		if (ImGui::Button("+Gregory\nPatch", buttonDims))
		{
			std::vector<Entity*> patches;
			EntityPresets::FillInSurface(Entity::selection, patches);
			for (int i = 0; i < patches.size(); i++)
				mainFolder->Add(patches[i]);
		}
	}
	ImGui::End(); // Toolbox

	ImGui::Begin("3D Cursor");
	MyGui::SceneCursorWidget(&SceneCursor::instance);
	SceneCursor::instance.Correct(m_view, m_proj, m_invview, m_invproj);
	ImGui::End(); // 3D Cursor
	auto selectedBeziers = Entity::GetSelected<BezierCurve>();
	if (selectedBeziers.size() == 1)
	{
		ImGui::Begin("Bezier Curve");
		ImGui::Checkbox("Draw Bezier Polygon", &selectedBeziers.back()->drawPolygon);
		ImGui::End();
	}
	auto selectedSplines = Entity::GetSelected<SplineGenerator>();
	if (selectedSplines.size() == 1)
	{
		ImGui::Begin("Spline Curve");
		ImGui::Checkbox("Draw DeBoor Polygon", &selectedSplines.back()->drawPolygon);
		ImGui::End();
	}
	auto selectedVirtualCollections = Entity::GetSelected<VPointCollection>();
	if (selectedVirtualCollections.size() == 1)
	{
		ImGui::Begin("Virtual Points");
		auto* last = selectedVirtualCollections.back();
		if (!last->owner.GetComponent<BezierInterpolator>())
		{
			bool showing = last->ShowingVirtualPoints();
			if (ImGui::Checkbox("Show Virtual Points", &showing))
				last->ShowVirtualPoints(showing);
			ImGui::LabelText("Count", "%d", last->GetCount());
		}
		ImGui::End();
	}
	{
		auto selectedBicubicSurfaces = Entity::GetSelected<BicubicSurface>();
		if (selectedBicubicSurfaces.size() == 1)
		{
			ImGui::Begin("Bicubic Surface");
			auto* last = selectedBicubicSurfaces.back();
			ImGui::DragInt("Detail Level Offset", &last->surfDetailOffset, 1, -255, 255);

			ImGui::End();
		}
	}
	{
		auto selectedBicubicSegments = Entity::GetSelected<BicubicSegment>();
		if (selectedBicubicSegments.size() > 0)
		{
			ImGui::Begin("Bicubic Segment");
			const int ANY = 0b01;
			const int ALL = 0b11;
			int on = 0b10;
			for (auto it = selectedBicubicSegments.begin(); it < selectedBicubicSegments.end(); it++)
			{
				if ((*it)->drawPolygon)
					on |= 0b01;
				else
					on &= 0b01;
			}

			if (ImGui::CheckboxFlags("Draw Grid", &on, 0b11))
				if (on == ALL)
					for (auto it = selectedBicubicSegments.begin(); it < selectedBicubicSegments.end(); it++)
						(*it)->drawPolygon = true;
				else
					for (auto it = selectedBicubicSegments.begin(); it < selectedBicubicSegments.end(); it++)
						(*it)->drawPolygon = false;

			ImGui::End();
		}
	}
	{
		auto selectedGregs = Entity::GetSelected<GregoryPatch>();
		if (selectedGregs.size() > 0)
		{
			ImGui::Begin("Gregory Patch");
			const int ANY = 0b01;
			const int ALL = 0b11;
			int on = 0b10;
			for (auto it = selectedGregs.begin(); it < selectedGregs.end(); it++)
			{
				if ((*it)->drawPolygon)
					on |= 0b01;
				else
					on &= 0b01;
			}

			if (ImGui::CheckboxFlags("Draw Grid", &on, 0b11))
				if (on == ALL)
					for (auto it = selectedGregs.begin(); it < selectedGregs.end(); it++)
						(*it)->drawPolygon = true;
				else
					for (auto it = selectedGregs.begin(); it < selectedGregs.end(); it++)
						(*it)->drawPolygon = false;
			if (selectedGregs.size() == 1)
				ImGui::DragInt("Detail Level Offset", &selectedGregs.back()->surfDetailOffset, 1, -255, 255);
			ImGui::End();
		}
	}
	auto selectedTransforms = Entity::GetSelected<Transform>();
	if (!selectedTransforms.empty())
	{
		if (MyGui::SceneTransformWidget(&SceneTransform::instance))
			;// SceneTransform::instance.onModified.Notify();
	}
	else
	{
		SceneTransform::instance.Reset();
	}
}

void CadApplication::Update() {
	auto& io = ImGui::GetIO();
	if (io.MouseDown[1] && !io.WantCaptureMouse && io.KeyShift)
	{
		if (io.KeyCtrl)
			Camera::mainCamera->Move({ -io.MouseDelta.x * moveRate, io.MouseDelta.y * moveRate });
		else
			Camera::mainCamera->Rotate({ io.MouseDelta.x * turnRate, io.MouseDelta.y * turnRate });
	}
	if (io.MouseWheel != 0 && !io.WantCaptureMouse)
	{
		Camera::mainCamera->Zoom(io.MouseWheel * scaleRate);
	}
	if (io.MouseClicked[1] && !io.WantCaptureMouse && !io.KeyShift)
	{
		auto size = io.DisplaySize;
		auto screen = io.MousePos;
		ImVec2 norm = { 2 * screen.x / size.x - 1, -2 * screen.y / size.y + 1 };

		int rekInd = -1;
		float rekVal = INFINITY;
		auto& points = Catalogue<PointTransform>::Instance.GetAll();
		for (int i = 0; i < points.size(); i++)
		{
			auto pos = points[i]->Position();
			XMFLOAT4 v = { pos.x, pos.y, pos.z, 1 };

			XMStoreFloat4(&v,
				XMVector4Transform(
					XMVector4Transform(
						XMLoadFloat4(&v),
						XMLoadFloat4x4(&m_view)
					),
					XMLoadFloat4x4(&m_proj)
				)
			);
			ImVec2 dif = { norm.x - v.x / v.w, (norm.y - v.y / v.w) * size.y / size.x };
			float dist = sqrtf(dif.x * dif.x + dif.y * dif.y);
			if (dist < pointSize / 2 && v.z / v.w < rekVal)
			{
				rekInd = i;
				rekVal = v.z / v.w;
			}
		}
		auto& vpoints = Catalogue<VPointTransform>::Instance.GetAll();
		for (int i = 0; i < vpoints.size(); i++)
		{
			auto pos = vpoints[i]->Position();
			XMFLOAT4 v = { pos.x, pos.y, pos.z, 1 };

			XMStoreFloat4(&v,
				XMVector4Transform(
					XMVector4Transform(
						XMLoadFloat4(&v),
						XMLoadFloat4x4(&m_view)
					),
					XMLoadFloat4x4(&m_proj)
				)
			);
			ImVec2 dif = { norm.x - v.x / v.w, (norm.y - v.y / v.w) * size.y / size.x };
			float dist = sqrtf(dif.x * dif.x + dif.y * dif.y);
			if (dist < pointSize / 2 && v.z / v.w < rekVal)
			{
				rekInd = i + points.size();
				rekVal = v.z / v.w;
			}
		}
		if (rekInd >= 0)
		{
			auto& owner = rekInd < points.size() ?
				points[rekInd]->owner :
				vpoints[rekInd - points.size()]->owner;
			owner.Select(!owner.Selected());
		}
	}
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
		if (!Entity::selection.empty())
			for (int i = Entity::selection.size() - 1; i >= 0; i--)
				Entity::selection[i]->Select(false);
		else
			for (int i = 0; i < mainFolder->Get().size(); i++)
				mainFolder->Get()[i]->Select(true);


	if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		if (!Entity::selection.empty())
		{
			for (int i = Entity::selection.size() - 1; i >= 0; i--)
			{
				if (i >= Entity::selection.size())
					i = Entity::selection.size() - 1;
				for (int j = 0; j < mainFolder->Get().size(); j++)
					if (mainFolder->Get()[j] == Entity::selection[i])
					{
						Entity::selection[i]->Delete();
						break;
					}
			}
		}
	//TODO: wondow to choose where the merged point appears
	if (ImGui::IsKeyPressed(ImGuiKey_M, false))
		if (!Entity::selection.empty())
			for (int i = Entity::selection.size() - 2; i >= 0; i--)
				Entity::selection[i]->Merge(Entity::selection.back());
	Entity::FinalizeDeletions();
}


void CadApplication::SetColor(Entity* object, COLORING c)
{
	ID3D11Buffer* colorbuffer[] = { m_cbColor.get() };
	XMFLOAT4 color;
	if (c == COLORING::REGULAR)
	{
		if (object->Selected())
			if (Entity::selection.back() == object)
				color = ColorPalette::highlightOrange;
			else
				color = ColorPalette::selectYellow;
		else
			color = ColorPalette::white;
	}
	else if (c == COLORING::VIRTUAL_POINT)
	{
		if (object->Selected())
			color = ColorPalette::vEditMagenta;
		else
			color = ColorPalette::virtualPurple;
	}
	m_device.SetBuffer(m_cbColor.get(), &color);
	m_device.context()->PSSetConstantBuffers(0, 1, colorbuffer);
}

void CadApplication::SetModelMatrix(Transform* object)
{
	ID3D11Buffer* cbs[] = { m_cbworld.get(), m_cbview.get(), m_cbproj.get() };
	SceneTransform::instance.localOrigin = object ? object->localPosition : XMFLOAT3();
	MyMat modelMat = object ? object->Get() : MyMat::Identity();
	m_device.SetBuffer(m_cbworld.get(), &modelMat);

	m_device.context()->VSSetConstantBuffers(0, 1, &cbs[0]);
}

void CadApplication::OnResize()
{
	SIZE wndSize = m_window.getClientSize();
	XMStoreFloat4x4(&m_world, MyMat::Identity());
	auto p = MyMat::Perspective(
		fov,
		static_cast<float>(wndSize.cx) / wndSize.cy,
		n, f);
	XMStoreFloat4x4(&m_proj, p);
	XMStoreFloat4x4(&m_invproj, XMMatrixInverse(nullptr, p));

	auto client = m_window.getClientSize();
	float ratio = client.cx * 1.0f / client.cy;
	m_gizmoBuffer.x = ratio;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);
}

void CadApplication::Render() {
	const float clearColor[] = { 0.0f, 0.0f, 0.0f };


	m_device.context()->ClearRenderTargetView(
		m_backBuffer.get(), clearColor);




	m_device.context()->VSSetShader(
		m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(
		m_pixelShader.get(), nullptr, 0);
	m_device.context()->GSSetShader(
		nullptr, nullptr, 0);

	ID3D11Buffer* cbs[] = { m_cbworld.get(), m_cbview.get(), m_cbproj.get() };

	XMStoreFloat4x4(&m_view, Camera::mainCamera->Transform());
	XMStoreFloat4x4(&m_invview, Camera::mainCamera->ReverseTransform());
	m_device.SetBuffer(m_cbview.get(), &m_view);
	m_device.context()->VSSetConstantBuffers(1, 1, &cbs[1]);

	m_device.SetBuffer(m_cbproj.get(), &m_proj);
	m_device.context()->VSSetConstantBuffers(2, 1, &cbs[2]);
	DrawGrid();

	m_device.context()->GSSetConstantBuffers(1, 1, &cbs[1]);
	m_device.context()->GSSetConstantBuffers(2, 1, &cbs[2]);

	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	auto mrs = Catalogue<MeshRenderer>::Instance.GetAll();
	for (int i = 0; i < mrs.size(); i++)
	{
		const Mesh* mesh = mrs[i]->mesh;
		Transform* t = &mrs[i]->transform;
		Entity* owner = &mrs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(t);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}

	m_device.context()->PSSetShader(
		m_wirePixelShader.get(), nullptr, 0);
	m_device.context()->DSSetShader(
		m_bezierDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_bezierHullShader.get(), nullptr, 0);
	m_device.context()->HSSetConstantBuffers(1, 1, &cbs[1]);
	m_device.context()->HSSetConstantBuffers(2, 1, &cbs[2]);
	m_device.context()->DSSetConstantBuffers(2, 1, &cbs[2]);
	m_device.context()->VSSetShader(
		m_bezierVertexShader.get(), nullptr, 0);
	auto bcs = Catalogue<BezierCurve>::Instance.GetAll();
	for (int i = 0; i < bcs.size(); i++)
	{
		const Mesh* mesh = bcs[i]->GetMesh();
		Entity* owner = &bcs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}

	/*m_device.context()->PSSetShader(
		m_surfacePixelShader.get(), nullptr, 0);*/
	m_device.context()->PSSetShader(
		m_wirePixelShader.get(), nullptr, 0);
	m_device.context()->DSSetShader(
		m_bicubicDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_bicubicHullShader.get(), nullptr, 0);
	m_device.context()->RSSetState(m_rasterizerNoCull.get());

	auto bbcs = Catalogue<BicubicSegment>::Instance.GetAll();
	for (int i = 0; i < bbcs.size(); i++)
	{
		const Mesh* mesh = bbcs[i]->GetMesh();
		Entity* owner = &bbcs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		XMINT4 mode = { bbcs[i]->deBoorMode ? 1 : 0, (surfDetail+bbcs[i]->surface->surfDetailOffset)-1,0,0};
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		auto* pmode = m_cbSurfMode.get();
		m_device.context()->DSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->HSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);

		mode.z = 1;
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		m_device.context()->DSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->HSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}
	m_device.context()->DSSetShader(
		m_gregDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_gregHullShader.get(), nullptr, 0);
	auto gps = Catalogue<GregoryPatch>::Instance.GetAll();
	for (int i = 0; i < gps.size(); i++)
	{
		const Mesh* mesh = gps[i]->GetMesh();
		Entity* owner = &gps[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		XMINT4 mode = { 0, (surfDetail + gps[i]->surfDetailOffset) -1 ,0,0};
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		auto* pmode = m_cbSurfMode.get();
		m_device.context()->DSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->HSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);

		mode.z = 1;
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		m_device.context()->DSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->HSSetConstantBuffers(3, 1, &pmode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}

	m_device.context()->DSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->HSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->VSSetShader(
		m_vertexShader.get(), nullptr, 0);

	m_device.context()->RSSetState(nullptr);

	for (int i = 0; i < gps.size(); i++)
	{
		if (!gps[i]->drawPolygon)
			continue;
		const Mesh* mesh = gps[i]->GetWireMesh();
		Entity* owner = &gps[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}
	for (int i = 0; i < bbcs.size(); i++)
	{
		if (!bbcs[i]->drawPolygon)
			continue;
		const Mesh* mesh = bbcs[i]->GetWireMesh();
		Entity* owner = &bbcs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}

	for (int i = 0; i < bcs.size(); i++)
	{
		if (!bcs[i]->drawPolygon)
			continue;
		const Mesh* mesh = bcs[i]->GetMesh();
		Entity* owner = &bcs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		int verts = mesh->SetBuffers();
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		m_device.context()->DrawIndexed(verts, 0, 0);
	}
	auto scs = Catalogue<SplineGenerator>::Instance.GetAll();
	for (int i = 0; i < scs.size(); i++)
	{
		if (!scs[i]->drawPolygon)
			continue;
		const Mesh* mesh = scs[i]->GetPolygon();
		Entity* owner = &scs[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		int verts = mesh->SetBuffers();
		m_device.context()->DrawIndexed(verts, 0, 0);
	}

	m_device.context()->GSSetShader(
		m_pointGeometryShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(
		m_pixelShader.get(), nullptr, 0);
	ID3D11Buffer* gizmobuffer[] = { m_cbgizmos.get() };
	m_gizmoBuffer.y = pointSize / m_window.getClientSize().cx * m_window.m_defaultWindowWidth;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);
	m_device.context()->GSSetConstantBuffers(0, 1, gizmobuffer);

	auto prs = Catalogue<PointRenderer>::Instance.GetAll();
	for (int i = 0; i < prs.size(); i++)
	{
		Transform* t = &prs[i]->transform;
		Entity* owner = &prs[i]->owner;
		if (!owner->enabled)
			continue;
		SetColor(owner,
			dynamic_cast<VPointTransform*>(t) ?
			COLORING::VIRTUAL_POINT :
			COLORING::REGULAR);

		SetModelMatrix(t);
		m_device.context()->DrawIndexed(point->SetBuffers(), 0, 0);
	}



	DrawAxes(SceneCursor::instance.GetWorld(), cursorSize);

	if (!Entity::GetSelected<Transform>().empty())
		DrawAxes(SceneTransform::instance.selectionCenter +
			(SceneTransform::instance.type == SceneTransform::Type::center ?
				SceneTransform::instance.localPosition :
				XMFLOAT3{})
			, groupCenterSize);

}

void CadApplication::DrawAxes(DirectX::XMFLOAT3 location, float size)
{
	ID3D11DepthStencilState* p;
	m_device.context()->OMGetDepthStencilState(&p, 0);
	m_device.context()->OMSetDepthStencilState(m_depthStateOver.get(), 0);

	ID3D11Buffer* cbs[] = { m_cbworld.get() };
	m_device.context()->GSSetShader(
		m_cursorGeometryShader.get(), nullptr, 0);
	ID3D11Buffer* gizmobuffer[] = { m_cbgizmos.get() };
	m_gizmoBuffer.y = size / m_window.getClientSize().cx * m_window.m_defaultWindowWidth;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);
	m_device.context()->GSSetConstantBuffers(0, 1, gizmobuffer);

	auto mesh = point.get();
	auto mat = MyMTrans(location);


	ID3D11Buffer* colorbuffer[] = { m_cbColor.get() };
	XMFLOAT4X4 colors{
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 1,
		1, 1, 1, 1
	};
	m_device.SetBuffer(m_cbColor.get(), &colors);
	m_device.context()->PSSetConstantBuffers(0, 1, colorbuffer);

	m_device.SetBuffer(m_cbworld.get(), &mat);
	m_device.context()->VSSetConstantBuffers(0, 1, &cbs[0]);

	m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	m_device.context()->OMSetDepthStencilState(p, 0);
}

void CadApplication::DrawGrid()
{
	float v = Camera::mainCamera->GetZoom() + fabs(Camera::mainCamera->GetOffset().z);
	v = log10f(v);
	const float l10 = log2f(10);
	float v1 = exp2f(l10 * floorf(v)) / 4;
	float v2 = exp2f(l10 * ceilf(v)) / 4;
	float mod = 1 - (v - floorf(v));
	DrawSingleGrid(v1, mod);
	DrawSingleGrid(v2, 1);
}

void CadApplication::DrawSingleGrid(float scale, float visibility)
{
	ID3D11DepthStencilState* p;
	m_device.context()->OMGetDepthStencilState(&p, 0);
	m_device.context()->OMSetDepthStencilState(m_depthStateOver.get(), 0);

	ID3D11Buffer* colorbuffer[] = { m_cbColor.get() };
	XMFLOAT4 color1 = ColorPalette::Darken(
		ColorPalette::gray, 1 - visibility
	);
	m_device.SetBuffer(m_cbColor.get(), &color1);
	m_device.context()->PSSetConstantBuffers(0, 1, colorbuffer);

	ID3D11Buffer* cbs[] = { m_cbworld.get() };
	XMFLOAT3 offset = Camera::mainCamera->GetOffset();
	{
		auto v = -XMLoadFloat3(&offset);
		v = XMVectorSetZ(v, 0);
		v = XMVectorScale(v, 1 / scale);
		v = XMVectorFloor(v);
		v = XMVectorScale(v, scale);
		XMStoreFloat3(&offset, v);
	}
	MyMat gridModel = MyMScale({ scale,scale,1.0f }) * MyMTrans(offset);
	m_device.SetBuffer(m_cbworld.get(), &gridModel);
	m_device.context()->VSSetConstantBuffers(0, 1, &cbs[0]);

	m_device.context()->DrawIndexed(grid->SetBuffers(), 0, 0);

	m_device.context()->OMSetDepthStencilState(p, 0);
}
