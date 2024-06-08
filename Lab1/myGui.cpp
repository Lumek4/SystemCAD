#include "myGui.h"
#include"imgui/imgui.h"
#include"vecmath.h"
#include"sceneCursor.h"
#include"colorPaletteh.h"
#include<DirectXMath.h>
#include"entityPresets.h"
using namespace DirectX;
using namespace ColorPalette;

char h[20] = "##";
bool showTransforms = false, showDragDropTargets = true;
bool dragging = false;



void SetHash(void* hash, int size)
{
	memset(h + 2, 0, sizeof(h) - 2);
	char* c = (char*)hash;
	char a = 0;
	int i = 0;
	for (;i < size && i < sizeof(h) - 4; i++)
		if (c[i] == 0)
			h[i + 2] = ++a;
		else
			h[i + 2] = c[i];
	h[i] = a;
}

void MyGui::EntityBaseWidget(Entity* ep, bool& remove, bool& allowDragDrop, bool& shiftSelected)
{
	bool styled = true;
	XMFLOAT4 color;
	if (!Entity::selection.empty() && ep == Entity::selection.back())
	{
		color = highlightOrange;
	}
	else if (ep->Selected())
	{
		color = selectYellow;
	}
	else
		styled = false;

	if (dragging)
		color = Darken(color, 0.4);
	if (styled)
	{
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
			color);
		ImGui::PushStyleColor(ImGuiCol_Header,
			Darken(color, 0.2));
	}
	ImGui::BeginGroup();

	SetHash(&ep, sizeof(&ep));

	if (ImGui::Button((std::string("X") + h).c_str(), {20,20}))
		remove = true;
	ImGui::SameLine(0, 5);
	ImGui::Checkbox((h + std::string("enable")).c_str(),
		&ep->enabled);

	ImGui::SameLine();

	if (!ImGui::GetIO().MouseDown[1])
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
		bool selected = ep->Selected();
		
		if (ImGui::Selectable(h, selected, ImGuiSelectableFlags_None, { 0, 19 }))
		{
			ep->Select(!selected);
			if (ImGui::GetIO().KeyShift)
				shiftSelected = !selected;
		}
		if (ImGui::IsItemHovered() && selected)
			allowDragDrop = true;
		ImGui::SameLine();
		//ImGui::SetCursorPosY(ImGui::GetCursorPosY());
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	}
	else
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - 64);
	auto name = ep->GetName();
	char buf[256] = {};
	memcpy_s(buf, 256, name.data(), name.length());
	if (ImGui::InputText((h + std::string("name")).c_str(),
		buf, 256))
	{
		ep->SetName(std::string_view(buf));
	}
	ImGui::PopItemWidth();

	if (showTransforms)
	{
		ImGui::Indent();
		auto* transform = ep->GetComponent<Transform>();
		if (transform)
			MyGui::TransformWidget(transform);
		ImGui::Unindent();
	}
	auto* list = ep->GetComponent<EntityCollection>();
	if (list)
	{
		ImGui::Indent();
		MyGui::EntitySecondaryListWidget(list->Get(), list);
		ImGui::Unindent();
	}
	ImGui::EndGroup();
	if (styled)
		ImGui::PopStyleColor(2);
}
void MyGui::EntitySecondaryWidget(Entity* ep, EntityCollection* parentp)
{
	bool styled = true;
	XMFLOAT4 color;
	if (!Entity::selection.empty() && ep == Entity::selection.back())
	{
		color = highlightOrange;
	}
	else if (ep->Selected())
	{
		color = selectYellow;
	}
	else
		styled = false;

	if (dragging)
		color = Darken(color, 0.4);
	if (styled)
	{
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
			color);
		ImGui::PushStyleColor(ImGuiCol_Header,
			Darken(color, 0.2));
	}
	ImGui::BeginGroup();
	struct { void *a, *b; } tmp;
	tmp.a = ep, tmp.b = parentp;
	SetHash(&tmp, sizeof(tmp));

	if (parentp->IsMutable())
	{
		if (ImGui::Button((std::string("X") + h).c_str(), { 20,20 }))
			parentp->Remove(ep);
		ImGui::SameLine();
	}
	ImGui::PushStyleColor(ImGuiCol_Text, Darken(white, 0.5f));

	if (!ImGui::GetIO().MouseDown[1])
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3);
		bool selected = ep->Selected();

		if (ImGui::Selectable(h, selected, ImGuiSelectableFlags_None, { 0, 19 }))
		{
			ep->Select(!selected);
		}
		ImGui::SameLine();
		//ImGui::SetCursorPosY(ImGui::GetCursorPosY());
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	}
	else
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2);

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetCursorPosX() - 24);
	auto name = ep->GetName();
	char buf[256] = {};
	memcpy_s(buf, 256, name.data(), name.length());
	ImGui::InputText((h + std::string("name")).c_str(),
		buf, 256, ImGuiInputTextFlags_ReadOnly);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::PopStyleColor(styled?3:1);
}
void MoveSelection(std::vector<Entity*>& list, const std::vector<Entity*>& selection, int item)
{
	auto moved = std::vector<Entity*>();
	for (int i = 0; i < item - moved.size(); i++)
	{
		list[i] = list[i + moved.size()];
		//list[i + moved.size()] = nullptr;
		for (int j = 0; j < selection.size(); j++)
			if (list[i] == selection[j])
			{
				moved.push_back(list[i]);
				//list[i] = nullptr;
				i--;
				break;
			}
	}
	for (int i = item - moved.size(); i < item; i++)
	{
		list[i] = moved[i - item + moved.size()];
		// moved[i - item + moved.size()] = nullptr;
	}
	moved.clear();
	for (int i = list.size() - 1; i >= item + moved.size(); i--)
	{
		list[i] = list[i - moved.size()];
		// list[i- moved.size()] = nullptr;
		for (int j = 0; j < selection.size(); j++)
			if (list[i] == selection[j])
			{
				moved.push_back(list[i]);
				// list[i] = nullptr;
				i++;
				break;
			}
	}
	for (int i = item + moved.size() - 1; i >= item; i--)
	{
		list[i] = moved[item + moved.size() - 1 - i];
		//moved[item + moved.size() - 1 - i] = nullptr;
	}
}
void MyGui::EntityListWidget(std::vector<Entity*>& list)
{
	bool dragDropAllowed = false;

	ImGui::BeginGroup();

	ImVec2 startingPosition = ImGui::GetCursorPos();
	ImVec2 dropTargetRect = { ImGui::GetWindowWidth(),25 };
	ImVec2 lastDropTargetRect = dropTargetRect;
	startingPosition.y -= dropTargetRect.y;

	int lastItem = -1;
	for (int item = 0; item <= list.size(); item++)
	{
		if (item < list.size() && list[item]->hideInList)
			continue;
		auto endPosition = ImGui::GetCursorPos();
		if (dragging)
		{
			ImGui::SetCursorPos({ startingPosition.x, startingPosition.y + 10 });
			ImGui::Dummy(dropTargetRect);
			ImGui::SetCursorPos(endPosition);
		}
		if (dragging && ImGui::BeginDragDropTarget())
		{
			auto itemPointCollection = lastItem < 0 ? nullptr :
				list[lastItem]->GetComponent<PointCollection>();

			auto payload = ImGui::AcceptDragDropPayload("Selection",
				showDragDropTargets ? 0 : ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			ImGui::Dummy({ ImGui::GetWindowWidth(), 10 });
			if (payload)
			{
				if (itemPointCollection && itemPointCollection->isMutable)
					itemPointCollection->AddSelection(Entity::selection);
				else
					MoveSelection(list, Entity::selection, item);

				dragging = false;
				ImGui::EndDragDropTarget();
				break;
			}

			ImGui::EndDragDropTarget();
		}

		if (item == list.size())
		{
			ImGui::SetCursorPos(endPosition);
			break;
		}
		bool remove = false, shiftSelected = false;
		startingPosition = ImGui::GetCursorPos();
		MyGui::EntityBaseWidget(list[item], remove, dragDropAllowed, shiftSelected);
		if (remove)
		{
			list[item]->Delete();
			break;
		}
		if (shiftSelected)
		{
			auto s = Entity::selection.size();
			if (s > 1)
			{
				list[item]->Select(false);
				auto last = Entity::selection[s - 2];
				int from = 0, to = 0;
				for (int i = 0; i < list.size(); i++)
					if (last == list[i])
						from = i;
					else if (list[item] == list[i])
						to = i;
				if (from > to)
					for (int i = from; i >= to; i--)
						list[i]->Select(true);
				else
					for (int i = from; i <= to; i++)
						list[i]->Select(true);
			}
		}
		lastItem = item;
		lastDropTargetRect = dropTargetRect;
		dropTargetRect.y = ImGui::GetCursorPosY() - startingPosition.y + 5;
	}
	ImGui::EndGroup();
	if ((dragging || dragDropAllowed) && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		ImGui::SetDragDropPayload("Selection", &Entity::selection, sizeof(std::vector<Entity*>*), ImGuiCond_Once);
		int size = Entity::selection.size();
		ImGui::Text("%d %s...", size, size == 1 ? "Entity" : "Entities");
		ImGui::EndDragDropSource();
		dragging = true;
	}
	else
		dragging = false;
}
void MyGui::EntitySecondaryListWidget(std::vector<Entity*>& list, EntityCollection* parent)
{

	ImGui::BeginGroup();
	if(ImGui::CollapsingHeader((h + std::string("SecondaryList")).c_str()))
		for (int item = 0; item < list.size(); item++)
		{
			MyGui::EntitySecondaryWidget(list[item], parent);
		}
	ImGui::EndGroup();
}

bool MyGui::TransformWidget(Transform* p)
{
	bool changed = false;
	if (ImGui::CollapsingHeader((std::string("Transform") + h).c_str()))
	{
		auto* ptransform = dynamic_cast<PointTransform*>(p);
		if (ptransform)
		{
			changed = ImGui::DragFloat3((std::string("Position") + h).c_str(),
				&ptransform->localPosition.x, 0.01f);
		}
		auto* transform = dynamic_cast<ModelTransform*>(p);
		if (transform)
		{
			changed = ImGui::DragFloat3((std::string("Position") + h).c_str(),
				&transform->localPosition.x, 0.01f);
			changed |= ImGui::DragFloat4((std::string("Rotation") + h).c_str(),
				&transform->rotation.x, 0.01f);
			XMStoreFloat4(&transform->rotation,
				XMVector4Normalize(XMLoadFloat4(&transform->rotation)));
			changed |= ImGui::DragFloat3((std::string("Scale") + h).c_str(),
				&transform->scale.x, 0.01f);
		}
	}
	if (changed)
		p->onModified.Notify(p);
	return changed;
}

bool MyGui::UnitFloat3(const char* name, float* v)
{
	float old[3];
	for (int i = 0; i < 3; i++)
		old[i] = v[i];
	if (ImGui::DragFloat3(name,
		v, 0.01f, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		int ind = -1;

		for (int i = 0; i < 3; i++)
			if (old[i] != v[i])
				ind = i;

		if (ind != -1)
		{
			float len = 0;
			for (int i = 0; i < 3; i++)
				if (i != ind)
					len += v[i] * v[i];
			if (len == 0)
				v[ind] = 1;
			else
			{
				len /= 1 - v[ind] * v[ind];
				for (int i = 0; i < 3; i++)
					if (i != ind)
						v[i] /= len;
			}
		}
		XMStoreFloat3((XMFLOAT3*)v,
			XMVector3Normalize(XMLoadFloat3((XMFLOAT3*)v)));
		return true;
	}
	return false;
}

bool MyGui::TorusMeshWidget(std::unique_ptr<Mesh>& m, TorusMeshData& data)
{
	if (ImGui::SliderInt2(" Model detail", (int*)&data.division, 3, 500, "%d Segments", ImGuiSliderFlags_AlwaysClamp) |
		ImGui::SliderFloat(" Major radius", &data.radii.x, data.radii.y, 20, "%.2f", ImGuiSliderFlags_AlwaysClamp) |
		ImGui::SliderFloat(" Minor radius", &data.radii.y, 0.1f, data.radii.x, "%.2f", ImGuiSliderFlags_AlwaysClamp))
	{
		auto newTorus = Mesh::Torus(data.division.x, data.division.y, data.radii.x, data.radii.y);
		auto renderers = Catalogue<MeshRenderer>::Instance.GetAll();
		for (int i = 0; i < renderers.size(); i++)
			if (renderers[i]->mesh == m.get())
				renderers[i]->mesh = newTorus.get();
		m = std::move(newTorus);
		return true;
	}
	return false;
}

bool MyGui::SceneCursorWidget(SceneCursor* sc)
{
	auto size = ImGui::GetIO().DisplaySize;

	ImGui::PushItemWidth(ImGui::CalcItemWidth() / 2 - 4);

	bool xchanged = ImGui::SliderFloat("##scx",
		&sc->screen.x, 1, size.x, "%.0f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();

	bool ychanged = ImGui::SliderFloat("Screen##scy",
		&sc->screen.y, 1, size.y, "%.0f", ImGuiSliderFlags_AlwaysClamp);

	if (isnan(sc->screen.x) && ychanged)
		sc->screen.x = size.x / 2;
	if (isnan(sc->screen.y) && xchanged)
		sc->screen.y = size.y / 2;

	ImGui::PopItemWidth();
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse)
	{
		auto cur = ImGui::GetMousePos();
		sc->screen.x = cur.x; sc->screen.y = size.y - cur.y;
		xchanged = ychanged = true;
	}
	bool wchanged = ImGui::DragFloat3("World", &sc->world.x, 0.01f);
	ImGui::SameLine();
	if (ImGui::Button("0"))
	{
		sc->world = {};
		wchanged = true;
	}
	sc->sChanged = xchanged || ychanged;
	return sc->sChanged || wchanged;
}
bool MyGui::SceneTransformWidget(SceneTransform* st)
{
	auto selectedTransforms = Entity::GetSelected<Transform>();

	//st->translation = st->localPosition;
	//XMFLOAT3 rotationAxis{ st->rotation.x, st->rotation.y, st->rotation.z };

	//auto ax = XMLoadFloat3(&rotationAxis);
	//float axlen = XMVectorGetX(XMVector3Length(ax));

	//st->rotationAngle = 2 * atan2f(axlen, st->rotation.w);
	//st->rotationAngle += floorf(st->rotationAngle/XM_2PI)*XM_2PI;
	//if (axlen >= FLT_EPSILON)
	//{
	//	XMStoreFloat3(&rotationAxis, ax / axlen);
	//	st->axis = rotationAxis;
	//}

	//st->scaleFactor = st->scale.x;

	
	ImGui::Begin("Transformation");
	bool t_changed = ImGui::DragFloat3("Translation", &st->translation.x, 0.01f);
	bool r_changed = MyGui::UnitFloat3("Rotation axis", &st->axis.x);

	r_changed |= ImGui::DragFloat("Angle",
		&st->rotationAngle, 0.01f);
	bool s_changed = ImGui::DragFloat("Scale",
		&st->scaleFactor, 0.01f);

	
	const char* transformOptions =
		"Common center\0"\
		"Cursor\0"\
		"Local origin\0"\
		"Highlighted object\0"\
		"Global origin\0";
	bool type_changed = ImGui::Combo("##mode", (int*)&st->type, transformOptions);

	st->selectionCenter = {};
	for (int i = 0; i < selectedTransforms.size(); i++)
	{
		st->localOrigin = selectedTransforms[i]->localPosition;
		auto pos = st->type == SceneTransform::Type::center ?
			selectedTransforms[i]->localPosition :
			selectedTransforms[i]->Position();
		st->selectionCenter = st->selectionCenter + pos;
	}
	if (!selectedTransforms.empty())
	{
		st->selectionCenter = st->selectionCenter * (1.0f / selectedTransforms.size());

		st->local = false;
		if(s_changed)
			st->scale = { st->scaleFactor, st->scaleFactor, st->scaleFactor };
		if (r_changed)
			st->rotation = Quaternion::Get(st->axis, st->rotationAngle);
		if (t_changed)
			st->localPosition = st->translation;
		switch (st->type)
		{
		case SceneTransform::Type::global: // no changes
			st->origin = { 0,0,0 };
			break;
		case SceneTransform::Type::highlight:
			st->origin = selectedTransforms.back()->localPosition;
			break;
		case SceneTransform::Type::center: // average position
			st->origin = st->selectionCenter;
			break;
		case SceneTransform::Type::cursor: // sceneCursor
			st->origin = SceneCursor::instance.GetWorld();
			break;
		case SceneTransform::Type::local: // special
			st->local = true;
			break;
		}
		if (s_changed || r_changed || t_changed || type_changed)
			st->onModified.Notify(st);
	}

	ImGui::SameLine();
	if (ImGui::Button("Apply"))
	{
		st->Apply();
		

		ImGui::End(); // Transformation
		return true;
	}


	ImGui::End(); // Transformation
	return s_changed || r_changed || t_changed || type_changed;
}

void MyGui::GuiOptions()
{
	ImGui::Checkbox("Show transforms", &showTransforms);
	ImGui::Checkbox("Highlight drag and drop targets", &showDragDropTargets);
}
void MyGui::ShowBicubicSurfacePopup(BicubicSurfaceParams& data)
{
	ImGui::OpenPopup("Create a Bicubic Surface", ImGuiPopupFlags_::ImGuiPopupFlags_MouseButtonLeft);
}
void MyGui::BicubicSurfacePopup(BicubicSurfaceParams& data, bool& modified, bool& create)
{
	if (!ImGui::BeginPopup("Create a Bicubic Surface"))
		return;
	ImGui::Text("Create a Bicubic Surface");

	static bool cylinder = false;
	modified = ImGui::Checkbox("Cylindrical", &cylinder);
	data.wrapMode = cylinder ? SURFACE_WRAP_U : SURFACE_WRAP_NONE;
	ImGui::SameLine();
	modified |= ImGui::Checkbox("C2 Continuity", &data.deBoor);

	modified |= ImGui::SliderInt2(" Dimensions", (int*)&data.division, 1, 20, "%d Segments", ImGuiSliderFlags_AlwaysClamp) |
		ImGui::SliderFloat(cylinder?" Radius":" Width", &data.dimensions.x, 0.1f, 40, "%.2f", ImGuiSliderFlags_AlwaysClamp) |
		ImGui::SliderFloat(cylinder?" Height":" Height", &data.dimensions.y, 0.1f, 40, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	if (data.deBoor && cylinder  && data.division.x <3)
		data.division.x = 3;

	if ((create = ImGui::Button("Create")))
	{
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}

void MyGui::SameLineIfFits(float width)
{
	ImGui::SameLine();
	float pos = ImGui::GetCursorPosX() + width;
	float w = ImGui::GetWindowWidth();
	if (pos < w)
		;
	else
		ImGui::Dummy(ImVec2{0,0});
}