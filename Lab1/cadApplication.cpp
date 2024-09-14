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
#include"intersect.h"

using namespace mini;
using namespace DirectX;

CadApplication::CadApplication(HINSTANCE hInstance)
	:DxApplication(hInstance)
{


	

	const auto vsBytes = DxDevice::LoadByteCode(L"VS_FullTransform.cso");
	const auto vsUvBytes = DxDevice::LoadByteCode(L"VS_WithUV.cso");
		{
		const auto psBytes = DxDevice::LoadByteCode(L"PS_Models.cso");
		const auto wirePSBytes = DxDevice::LoadByteCode(L"PS_Shapes.cso");
		const auto pointgsBytes = DxDevice::LoadByteCode(L"pointgs.cso");
		const auto cursorgsBytes = DxDevice::LoadByteCode(L"curgs.cso");
		m_vs_transform = m_device.CreateVertexShader(vsBytes);
		m_vs_uvs = m_device.CreateVertexShader(vsUvBytes);
		m_ps_model = m_device.CreatePixelShader(psBytes);
		m_ps_shape = m_device.CreatePixelShader(wirePSBytes);
		m_pointGeometryShader = m_device.CreateGeometryShader(pointgsBytes);
		m_cursorGeometryShader = m_device.CreateGeometryShader(cursorgsBytes);
	}
	{

		const auto bezierVSBytes = DxDevice::LoadByteCode(L"VS_NoTransform.cso");
		const auto bezierHSBytes = DxDevice::LoadByteCode(L"bezierHS.cso");
		const auto bezierDSBytes = DxDevice::LoadByteCode(L"bezierDS.cso");
		m_vs_noTransform = m_device.CreateVertexShader(bezierVSBytes);
		m_bezierHullShader = m_device.CreateHullShader(bezierHSBytes);
		m_bezierDomainShader = m_device.CreateDomainShader(bezierDSBytes);
	}
	{

		const auto bicubicHSBytes = DxDevice::LoadByteCode(L"bicubicHS.cso");
		const auto bicubicDSBytes = DxDevice::LoadByteCode(L"bicubicDS.cso");
		const auto bicubicGridGSBytes = DxDevice::LoadByteCode(L"bicubicGridGS.cso");
		m_bicubicHullShader = m_device.CreateHullShader(bicubicHSBytes);
		m_bicubicDomainShader = m_device.CreateDomainShader(bicubicDSBytes);
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

	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_layout.push_back(m_device.CreateInputLayout(elements, vsBytes));
		VertexPosition::layout = m_layout[m_layout.size() - 1].get();
	}
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_layout.push_back(m_device.CreateInputLayout(elements, vsUvBytes));
		VertexPositionUV::layout = m_layout[m_layout.size() - 1].get();
	}


	torus = Mesh::Torus(10, 10, 2, 1);
	point = Mesh::Point();
	grid = Mesh::Grid(100);


	m_cbworld = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbview = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbproj = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();

	m_cbgizmos = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbTint = m_device.CreateConstantBuffer<DirectX::XMFLOAT4>();
	m_cbColor = m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>();
	m_cbSurfMode = m_device.CreateConstantBuffer<DirectX::XMINT4, 3>();

	BlendStateDescription bsdesc{};
	m_blendAdd = m_device.CreateBlendState(bsdesc);

	Camera::mainCamera = std::make_unique<Camera>(m_device);
	Camera::mainCamera->Rotate({ 0, 2 * XM_PI / 3 });
	//shapes.push_back(ImplicitShape::Ellipsoid(radii));

	mainFolder = Entity::New()->AddComponent<Folder>();
	mainFolder->Add(EntityPresets::Point({}));
	OnResize();
}

CadApplication::~CadApplication()
{
	Entity::Clear();
}

void CadApplication::GUI()
{
	auto viewportDock = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
	

	ImGui::Begin("Options");
	MyGui::GuiOptions();
	{
		static std::array<char, 256> buf{ "../testint.json" };
		bool sv = false, ld = false;
		MyGui::SaveLoadWidget(buf, sv, ld);
		try
		{
			if (sv)
				save(*this, buf.data());
			if (ld)
				load(*this, buf.data());

		}
		catch (std::exception e)
		{
			printf("%s", e.what());
		}
	}
	ImGui::SliderInt("Surface Detail", &surfDetail, 1, 64);
	ImGui::SliderFloat("Show UV", &surfaceUVColoring, 0.0f, 1.0f,
		"%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Checkbox("Anaglyph", &anaglyph);
	ImGui::SliderFloat("Eye distance", &eyeDistance, 0.001f, 10.0f);
	ImGui::SliderFloat("Plane distance", &planeDistance, 1.0f, 100.0f);
	ImGui::ColorEdit3("Left", &leftTint.x);
	ImGui::ColorEdit3("Right", &rightTint.x);
	ImGui::End(); // Options

	{
		auto trss = Entity::GetSelected<TorusGenerator>();
		if (!trss.empty())
		{
			ImGui::Begin("Torus model data");
			MyGui::TorusMeshWidget(trss.back());
			ImGui::End(); // Torus model data
		}
	}

	ImGui::Begin("Entities");
	MyGui::EntityListWidget(mainFolder->Get());
	ImGui::End(); // Entities

	ImGui::Begin("Toolbox");
	ImVec2 buttonDims(70, 40);
	if (ImGui::Button("+Torus", buttonDims))
		mainFolder->Add(EntityPresets::Torus(
			SceneCursor::instance.GetWorld()
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
	MyGui::SameLineIfFits(buttonDims.x);
	{
		if (ImGui::Button("Intersect", buttonDims))
			MyGui::ShowIntersectPopup();
		static float detail = 0.2f; bool create = false;
		static int texResolution = 128;
		MyGui::IntersectPopup(detail, texResolution, create);

		if(create)
		{
			auto bsg = Entity::GetSelected<BicubicSegment>();
			auto bsf = Entity::GetSelected<BicubicSurface>();
			auto tgn = Entity::GetSelected<TorusGenerator>();
			
			std::vector<std::vector<DirectX::XMFLOAT3>> pts;
			std::vector<std::vector<DirectX::XMFLOAT2>> uva;
			std::vector<std::vector<DirectX::XMFLOAT2>> uvb;
			std::vector<Entity*> a;
			std::vector<Entity*> b;

			multiIntersect(bsg, bsg, detail, pts, uva, uvb, a, b);
			multiIntersect(bsf, bsf, detail, pts, uva, uvb, a, b);
			multiIntersect(tgn, tgn, detail, pts, uva, uvb, a, b);

			multiIntersect(bsg, bsf, detail, pts, uva, uvb, a, b);
			multiIntersect(bsg, tgn, detail, pts, uva, uvb, a, b);
			multiIntersect(bsf, tgn, detail, pts, uva, uvb, a, b);

			for (int i = 0; i < pts.size(); i++)
			{
				mainFolder->Add(EntityPresets::IntersCurve(
					a[i], b[i],
					uva[i], uvb[i], pts[i], texResolution));
			}
		}
	}
	ImGui::End(); // Toolbox

	ImGui::Begin("3D Cursor");
	MyGui::SceneCursorWidget(&SceneCursor::instance);
	SceneCursor::instance.Correct(m_view, m_proj, m_invview, m_invproj);
	ImGui::End(); // 3D Cursor
	{
		auto selectedBeziers = Entity::GetSelected<BezierCurve>();
		if (selectedBeziers.size() == 1)
		{
			ImGui::Begin("Bezier Curve");
			ImGui::Checkbox("Draw Bezier Polygon", &selectedBeziers.back()->drawPolygon);
			ImGui::End();
		}
	}
	{
		auto selectedSplines = Entity::GetSelected<SplineGenerator>();
		if (selectedSplines.size() == 1)
		{
			ImGui::Begin("Spline Curve");
			ImGui::Checkbox("Draw DeBoor Polygon", &selectedSplines.back()->drawPolygon);
			ImGui::End();
		}
	}
	{
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
	{
		auto selectedIntersections = Entity::GetSelected<IntersectionCurve>();
		if (!selectedIntersections.empty())
		{
			auto& inters = selectedIntersections.back();
			ImGui::Begin("Intersection");
			MyGui::TextureWidget(inters->texvA.get(), inters->trimListA);
			MyGui::TextureWidget(inters->texvB.get(), inters->trimListB);
			if (ImGui::Button("Make interpolated curve"))
			{
				std::vector<Entity*> line{};
				for (int i = 0; i < inters->pointsWorld.size(); i++)
				{
					line.push_back(EntityPresets::Point(inters->pointsWorld[i]));
					line.back()->enabled = false;
				}
				/*for (int i = 0; i < inters->pointsWorld.size()&&i<1; i++)
				{
					line.push_back(EntityPresets::Point(inters->pointsWorld[i]));
					line.back()->enabled = false;
				}*/
				mainFolder->Add(EntityPresets::InterpCurve(line));
				mainFolder->AddSelection(line);
			}
			ImGui::End(); //Intersection
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
	if (io.MouseDown[2] && !io.WantCaptureMouse)
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
	{
		static bool translating = false;
		static bool scaling = false;
		static bool rotating = false;
		if (ImGui::IsKeyPressed(ImGuiKey_Q, false))
		{
			translating = true;
			scaling = false;
			rotating = false;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_W, false))
		{
			translating = false;
			scaling = true;
			rotating = false;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_E, false))
		{
			translating = false;
			scaling = false;
			rotating = true;
		}
		auto selectedTransforms = Entity::GetSelected<Transform>();
		if (selectedTransforms.empty() ||
			ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			translating = scaling = rotating = false;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) && io.KeyShift)
		{
			translating = scaling = rotating = false;
			SceneTransform::instance.Apply();
		}
		if(translating || scaling || rotating)
		{
			auto mou = ImGui::GetMousePos();
			ImGui::SetNextWindowPos({ mou.x - 15, mou.y - 15 });
			ImGui::SetNextWindowSize({ 15, 15 });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 8,8 });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4,1 });
			ImGui::PushStyleColor(ImGuiCol_Text, ColorPalette::highlightOrange);
			ImGui::Begin("MouseTransform", nullptr, ImGuiWindowFlags_NoDecoration);
			if (translating)
				ImGui::Text("T");
			if (scaling)
				ImGui::Text("S");
			if (rotating)
				ImGui::Text("R");
			ImGui::End();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);
		}
		static bool mouseTransforming = false;
		static bool mouseTransformingStart = false;
		if (mouseTransforming)
		{
			auto& st = SceneTransform::instance;
			if (translating)
			{
				static XMFLOAT3 pos;
				if (mouseTransformingStart)
					pos = st.translation;
				
				float c = 2*tan(fov*0.5f) / io.DisplaySize.y;
				auto delta = ImGui::GetMouseDragDelta();
				auto v = XMVector4Transform(
					{
						 delta.x*c,
						-delta.y*c,
						0, 0 },
					Camera::mainCamera->ReverseTransform());
				XMStoreFloat3(&st.translation,
					XMLoadFloat3(&pos)
					+ v*Camera::mainCamera->GetZoom());
				st.localPosition = st.translation;
			}
			if (scaling)
			{
				static float sca;
				if (mouseTransformingStart)
					sca = st.scaleFactor;

				auto delta = ImGui::GetMouseDragDelta();
				XMStoreFloat3(&st.scale, sca *
					XMVector2Dot({
						io.MousePos.x - io.DisplaySize.x / 2,
						io.MousePos.y - io.DisplaySize.y / 2
						},{
						io.MousePos.x - io.DisplaySize.x / 2 - delta.x,
						io.MousePos.y - io.DisplaySize.y / 2 - delta.y})
						/
					XMVector2LengthSq({
						io.MousePos.x - io.DisplaySize.x / 2 - delta.x,
						io.MousePos.y - io.DisplaySize.y / 2 - delta.y
						})
						);

				st.scaleFactor = st.scale.x;
			}
			if (rotating)
			{
				static float ang;
				if (mouseTransformingStart)
					ang = st.rotationAngle;



				auto delta = ImGui::GetMouseDragDelta();

				auto ax = XMVector4Transform(
					{0,0,1,0}, Camera::mainCamera->ReverseTransform());

				XMStoreFloat3(&st.axis, ax);
				st.rotationAngle = ang + atan2f(
					io.MousePos.y - io.DisplaySize.y / 2,
					io.MousePos.x - io.DisplaySize.x / 2) -
					atan2f(
						io.MousePos.y - io.DisplaySize.y / 2 - delta.y,
						io.MousePos.x - io.DisplaySize.x / 2 - delta.x
					);

				st.rotation = Quaternion::Get(st.axis, st.rotationAngle);
			}
				
			SceneTransform::instance.onModified.Notify(&SceneTransform::instance);
		}
		mouseTransformingStart = mouseTransforming ^ ImGui::IsMouseDragging(0);
		mouseTransforming = (translating || scaling || rotating) && !io.WantCaptureMouse &&
			ImGui::IsMouseDragging(0);
	}


	static bool boxSelect = false;
	if (!boxSelect &&
		ImGui::IsMouseReleased(1) && !io.KeyShift && !io.WantCaptureMouse)
	{
		XMVECTOR mouse = {
			2 * io.MousePos.x / io.DisplaySize.x - 1,
			-2 * io.MousePos.y / io.DisplaySize.y + 1
		};
		XMMATRIX viewproj = XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj);
		float size = pointSize / 2 / io.DisplaySize.x * m_window.m_defaultWindowWidth;
		float aspect = io.DisplaySize.y / io.DisplaySize.x;

		int rekInd = -1;
		float rekVal = INFINITY;
		auto& points = Catalogue<PointTransform>::Instance.GetAll();
		auto& vpoints = Catalogue<VPointTransform>::Instance.GetAll();
		for (int i = 0; i < points.size()+vpoints.size(); i++)
		{
			XMFLOAT3 pos;
			if (i < points.size())
			{
				if (!points[i]->owner.enabled)
					continue;
				pos = points[i]->Position();
			}
			else
			{
				if (!vpoints[i - points.size()]->owner.enabled)
					continue;
				pos = vpoints[i - points.size()]->Position();
			}

			float depth = vecmath::screenRay({ pos.x,pos.y,pos.z,1 }, mouse,
				viewproj, size, aspect);
			
			if (!isnan(depth) && depth < rekVal)
			{
				rekInd = i;
				rekVal = depth;
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
	if (boxSelect)
	{
		auto del = ImGui::GetMouseDragDelta(1);
		auto mou = ImGui::GetMousePos();
		ImVec2 mouseMin = { fminf(mou.x - del.x, mou.x),fminf(mou.y - del.y, mou.y) };
		ImVec2 mouseMax = { fmaxf(mou.x - del.x, mou.x),fmaxf(mou.y - del.y, mou.y) };

		ImGui::SetNextWindowPos(mouseMin);
		ImGui::SetNextWindowSize({ mouseMax.x - mouseMin.x, mouseMax.y - mouseMin.y });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 1,1 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4,1 });
		ImGui::PushStyleColor(ImGuiCol_Border, ColorPalette::highlightOrange);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ColorPalette::highlightOrange);
		ImGui::PushStyleColor(ImGuiCol_Text, ColorPalette::highlightOrange);
		ImGui::SetNextWindowBgAlpha(0.1f);
		ImGui::Begin("BoxSelect", nullptr, ImGuiWindowFlags_NoDecoration);
		bool addToSelection = !io.KeyCtrl && io.KeyShift;
		if (addToSelection)
			ImGui::Text("+");
		bool replaceSelection = !io.KeyCtrl && !io.KeyShift;
		if (replaceSelection)
			ImGui::Text("=");
		bool removeFromSelecion = io.KeyCtrl && !io.KeyShift;
		if (removeFromSelecion)
			ImGui::Text("-");
		bool xorSelection = io.KeyCtrl && io.KeyShift;
		if (xorSelection)
			ImGui::Text("*");
		ImGui::End();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(2);
		if (ImGui::IsMouseReleased(1))
		{
			if(replaceSelection)
				for (int i = Entity::selection.size() - 1; i >= 0; i--)
					Entity::selection[i]->Select(false);


			XMVECTOR mouseLo = {
				2 * mouseMin.x / io.DisplaySize.x - 1,
				-2 * mouseMax.y / io.DisplaySize.y + 1
			};
			XMVECTOR mouseHi = {
				2 * mouseMax.x / io.DisplaySize.x - 1,
				-2 * mouseMin.y / io.DisplaySize.y + 1
			};
			XMMATRIX viewproj = XMLoadFloat4x4(&m_view) * XMLoadFloat4x4(&m_proj);
			//float aspect = io.DisplaySize.y / io.DisplaySize.x;

			auto& points = Catalogue<PointTransform>::Instance.GetAll();
			for (int i = 0; i < points.size(); i++)
			{
				if (!points[i]->owner.enabled)
					continue;
				XMFLOAT3 pos = points[i]->Position();
				if (vecmath::screenBox({ pos.x,pos.y,pos.z,1 },
					mouseLo, mouseHi, viewproj))
					if(removeFromSelecion)
						points[i]->owner.Select(false);
					else if (xorSelection)
						points[i]->owner.Select(!points[i]->owner.Selected());
					else
						points[i]->owner.Select(true);
			}
			auto& vpoints = Catalogue<VPointTransform>::Instance.GetAll();
			for (int i = 0; i < vpoints.size(); i++)
			{
				if (!vpoints[i]->owner.enabled)
					continue;
				XMFLOAT3 pos = vpoints[i]->Position();
				if (vecmath::screenBox({ pos.x,pos.y,pos.z,1 },
					mouseLo, mouseHi, viewproj))
					if (removeFromSelecion)
						points[i]->owner.Select(false);
					else if (xorSelection)
						points[i]->owner.Select(!points[i]->owner.Selected());
					else
						points[i]->owner.Select(true);
			}
		}
	}
	boxSelect = ImGui::IsMouseDragging(1);


	if (ImGui::IsKeyPressed(ImGuiKey_A, false))
		if (!Entity::selection.empty())
			for (int i = Entity::selection.size() - 1; i >= 0; i--)
				Entity::selection[i]->Select(false);
		else
			for (int i = 0; i < mainFolder->Get().size(); i++)
				mainFolder->Get()[i]->Select(true);


	if (ImGui::IsKeyPressed(ImGuiKey_H, false))
		if (!Entity::selection.empty())
		{
			auto& s = Entity::selection;
			bool anyEnabled = false;
			for (int i = 0; i < s.size(); i++)
				anyEnabled |= s[i]->enabled;

			if(anyEnabled)
				for (int i = 0; i < s.size(); i++)
					s[i]->enabled = false;
			else
				for (int i = 0; i < s.size(); i++)
					s[i]->enabled = true;

		}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		if (!Entity::selection.empty())
		{
			Entity::Selection s = Entity::selection;
			for (int i = 0; i < s.size(); i++)
				s[i]->Delete();
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
	color.w *= 1-surfaceUVColoring;
	m_device.SetBuffer(m_cbColor.get(), &color);
}

void CadApplication::SetModelMatrix(Transform* object)
{
	SceneTransform::instance.localOrigin = object ? object->localPosition : XMFLOAT3();
	MyMat modelMat = object ? object->Get() : MyMat::Identity();
	m_device.SetBuffer(m_cbworld.get(), &modelMat);
}

void CadApplication::OnResize()
{
	auto wndSize = m_window.getClientSize();
	float ratio = static_cast<float>(wndSize.cx) / wndSize.cy;
	m_gizmoBuffer.x = ratio;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);
}
void CadApplication::Render()
{
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_device.context()->ClearRenderTargetView(
		m_backBuffer.get(), clearColor);
	SIZE wndSize = m_window.getClientSize();

	if (anaglyph)
	{
		m_device.context()->OMSetBlendState(m_blendAdd.get(), nullptr, -1);
		MyMat p = MyMat::Perspective(
			fov,
			static_cast<float>(wndSize.cx) / wndSize.cy,
			n, f, -eyeDistance/2, planeDistance);
		XMStoreFloat4x4(&m_proj, p);
		XMStoreFloat4x4(&m_invproj, XMMatrixInverse(nullptr, p));
		m_device.SetBuffer(m_cbTint.get(), &leftTint);
		Camera::mainCamera->Move({ -eyeDistance / 2 / Camera::mainCamera->GetZoom(), 0 });
		Draw();


		p = MyMat::Perspective(
			fov,
			static_cast<float>(wndSize.cx) / wndSize.cy,
			n, f, eyeDistance/2, planeDistance);
		XMStoreFloat4x4(&m_proj, p);
		XMStoreFloat4x4(&m_invproj, XMMatrixInverse(nullptr, p));
		m_device.SetBuffer(m_cbTint.get(), &rightTint);
		Camera::mainCamera->Move({ eyeDistance / Camera::mainCamera->GetZoom(), 0 });
		Draw();


		p = MyMat::Perspective(
			fov,
			static_cast<float>(wndSize.cx) / wndSize.cy,
			n, f);
		XMStoreFloat4x4(&m_proj, p);
		XMStoreFloat4x4(&m_invproj, XMMatrixInverse(nullptr, p));
		Camera::mainCamera->Move({ -eyeDistance / 2/ Camera::mainCamera->GetZoom(), 0});
		m_device.context()->OMSetBlendState(nullptr, nullptr, -1);
	}
	else
	{
		auto p = MyMat::Perspective(
			fov,
			static_cast<float>(wndSize.cx) / wndSize.cy,
			n, f);
		XMStoreFloat4x4(&m_proj, p);
		XMStoreFloat4x4(&m_invproj, XMMatrixInverse(nullptr, p));
		XMFLOAT4 tint = { 1,1,1,1 };
		m_device.SetBuffer(m_cbTint.get(), &tint);
		Draw();
	}
}

void CadApplication::Draw()
{
	m_device.context()->VSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->GSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->HSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->DSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->PSSetShader(
		nullptr, nullptr, 0);

	ID3D11Buffer* cbs[] = { m_cbworld.get(), m_cbview.get(), m_cbproj.get() };
	auto* pmode = m_cbSurfMode.get();
	XMINT4 clearMode[3] = { {}, {0,0,1,1}, {} };
	ID3D11Buffer* gizmobuffer[] = { m_cbgizmos.get() };
	ID3D11Buffer* colorbuffer[] = { m_cbColor.get(), m_cbTint.get()};

	m_device.context()->VSSetConstantBuffers(0, 3, &cbs[0]);

	m_device.context()->GSSetConstantBuffers(0, 1, gizmobuffer);
	m_device.context()->GSSetConstantBuffers(1, 2, &cbs[1]);

	m_device.context()->HSSetConstantBuffers(1, 2, &cbs[1]);
	m_device.context()->HSSetConstantBuffers(3, 1, &pmode);

	m_device.context()->DSSetConstantBuffers(1, 2, &cbs[1]);
	m_device.context()->DSSetConstantBuffers(3, 1, &pmode);

	m_device.context()->PSSetConstantBuffers(0, 2, colorbuffer);
	m_device.context()->PSSetConstantBuffers(3, 1, &pmode);


	XMStoreFloat4x4(&m_view, Camera::mainCamera->Transform());
	XMStoreFloat4x4(&m_invview, Camera::mainCamera->ReverseTransform());
	m_device.SetBuffer(m_cbview.get(), &m_view);
	m_device.SetBuffer(m_cbproj.get(), &m_proj);

	m_device.context()->VSSetShader(
		m_vs_transform.get(), nullptr, 0);
	m_device.context()->PSSetShader(
		m_ps_model.get(), nullptr, 0);
	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	DrawGrid();



	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//auto& mrs = Catalogue<MeshRenderer>::Instance.GetAll();
	//for (int i = 0; i < mrs.size(); i++)
	//{
	//	const Mesh* mesh = mrs[i]->mesh;
	//	if (mesh == nullptr)
	//		mesh = torus.get();
	//	Transform* t = &mrs[i]->transform;
	//	Entity* owner = &mrs[i]->owner;
	//	if (!owner->enabled || !mesh)
	//		continue;
	//	SetColor(owner);
	//	SetModelMatrix(t);
	//	m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	//}
	m_device.context()->VSSetShader(
		m_vs_uvs.get(), nullptr, 0);
	m_device.context()->PSSetShader(
		m_ps_shape.get(), nullptr, 0);
	auto cuts = Entity::GetSelected<IntersectionCurve>();
	auto& tgs = Catalogue<TorusGenerator>::Instance.GetAll();
	for (int i = 0; i < tgs.size(); i++)
	{
		const Mesh* mesh = tgs[i]->GetMesh();
		Entity* owner = &tgs[i]->owner;
		Transform* t = owner->GetComponent<Transform>();
		if (!owner->enabled || !mesh)
			continue;

		ID3D11ShaderResourceView* trim = nullptr;
		std::vector<bool>* trimAreas = nullptr;
		for (int j = 0; j < cuts.size(); j++)
		{
			if (owner == cuts[j]->a)
			{
				trim = cuts[j]->texvA.get();
				trimAreas = &cuts[j]->trimListA;
			}
			else if (owner == cuts[j]->b)
			{
				trim = cuts[j]->texvB.get();
				trimAreas = &cuts[j]->trimListB;
			}
			else
				continue;
			break;
		}
		m_device.context()->PSSetShaderResources(0, 1, &trim);

		XMINT4 mode[3] = { {0,0,0,trim ? 1 : 0 },
			{0,0,1,1}, {} };

		if (trimAreas)
			for (int j = 0; j < 32 && j < trimAreas->size(); j++)
				mode[2].x |= (1 << j) & (trimAreas[0][j] ? -1 : 0);

		m_device.SetBuffer(m_cbSurfMode.get(), mode, sizeof(XMINT4) * 3);

		SetColor(owner);
		SetModelMatrix(t);


		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}
	m_device.SetBuffer(m_cbSurfMode.get(), clearMode, sizeof(XMINT4) * 3);
	

	m_device.context()->DSSetShader(
		m_bezierDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_bezierHullShader.get(), nullptr, 0);
	m_device.context()->VSSetShader(
		m_vs_noTransform.get(), nullptr, 0);
	auto& bcs = Catalogue<BezierCurve>::Instance.GetAll();
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
		m_ps_shape.get(), nullptr, 0);
	m_device.context()->DSSetShader(
		m_bicubicDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_bicubicHullShader.get(), nullptr, 0);
	m_device.context()->RSSetState(m_rasterizerNoCull.get());

	auto& bbcs = Catalogue<BicubicSegment>::Instance.GetAll();
	for (int i = 0; i < bbcs.size(); i++)
	{
		const Mesh* mesh = bbcs[i]->GetMesh();
		Entity* owner = &bbcs[i]->owner;
		Entity* surfOwner = &bbcs[i]->surface->owner;
		if (!owner->enabled || !surfOwner->enabled || !mesh)
			continue;

		ID3D11ShaderResourceView* trim = nullptr;
		XMINT4 coords{ 0,0,1,1 };
		{
			coords.z = bbcs[i]->surface->division.x;
			coords.w = bbcs[i]->surface->division.y;
			auto& segs = bbcs[i]->surface->GetSegments();
			for (int y = 0; y < coords.w; y++)
				for (int x = 0; x < coords.z; x++)
					if (segs[x + y * coords.z] == bbcs[i])
					{
						coords.x = x; coords.y = y;
						y = coords.w; break;
					}
		}
		std::vector<bool>* trimAreas = nullptr;
		for (int j = 0; j < cuts.size(); j++)
		{
			if (owner == cuts[j]->a || surfOwner == cuts[j]->a)
			{
				trim = cuts[j]->texvA.get();
				trimAreas = &cuts[j]->trimListA;
			}
			else if (owner == cuts[j]->b || surfOwner == cuts[j]->b)
			{
				trim = cuts[j]->texvB.get();
				trimAreas = &cuts[j]->trimListB;
			}
			else
				continue;
			break;
		}
		m_device.context()->PSSetShaderResources(0, 1, &trim);
		SetColor(owner);
		SetModelMatrix(nullptr);
		XMINT4 mode[3] = { {bbcs[i]->deBoorMode ? 1 : 0,
			(surfDetail + bbcs[i]->surface->surfDetailOffset),
			0,trim ? 1 : 0 },
			coords,
			{}
		};
		if (trimAreas)
			for (int j = 0; j < 32 && j < trimAreas->size(); j++)
				mode[2].x |= (1 << j) & (trimAreas[0][j] ? -1 : 0);
		m_device.SetBuffer(m_cbSurfMode.get(), mode, sizeof(XMINT4) * 3);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);

		mode[0].z = 1;
		m_device.SetBuffer(m_cbSurfMode.get(), mode, sizeof(XMINT4) * 3);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}
	m_device.SetBuffer(m_cbSurfMode.get(), clearMode, sizeof(XMINT4) * 3);

	m_device.context()->DSSetShader(
		m_gregDomainShader.get(), nullptr, 0);
	m_device.context()->HSSetShader(
		m_gregHullShader.get(), nullptr, 0);
	auto& gps = Catalogue<GregoryPatch>::Instance.GetAll();
	for (int i = 0; i < gps.size(); i++)
	{
		const Mesh* mesh = gps[i]->GetMesh();
		Entity* owner = &gps[i]->owner;
		if (!owner->enabled || !mesh)
			continue;
		SetColor(owner);
		SetModelMatrix(nullptr);
		XMINT4 mode = { 0, (surfDetail + gps[i]->surfDetailOffset) ,0,0 };
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);

		mode.z = 1;
		m_device.SetBuffer(m_cbSurfMode.get(), &mode);
		m_device.context()->DrawIndexed(mesh->SetBuffers(), 0, 0);
	}

	m_device.context()->DSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->HSSetShader(
		nullptr, nullptr, 0);
	m_device.context()->VSSetShader(
		m_vs_transform.get(), nullptr, 0);

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
	auto& scs = Catalogue<SplineGenerator>::Instance.GetAll();
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
		m_ps_model.get(), nullptr, 0);

	m_gizmoBuffer.y = pointSize / m_window.getClientSize().cx * m_window.m_defaultWindowWidth;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);

	auto& prs = Catalogue<PointRenderer>::Instance.GetAll();
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
	m_gizmoBuffer.y = size / m_window.getClientSize().cx * m_window.m_defaultWindowWidth;
	m_device.SetBuffer(m_cbgizmos.get(), &m_gizmoBuffer);

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
