#pragma once
#include"torusGenerator.h"
#include"entity.hpp"
#include"mesh.h"
#include"sceneCursor.h"
#include"bicubicSurface.h"
#include <span>
#include<d3d11.h>

struct BicubicSurfaceParams;
namespace MyGui
{
	bool UnitFloat3(const char* name, float* v);
	 
	//int PushEntityStyle(Entity* ep);
	void SameLineIfFits(float width);
	void EntityBaseWidget(Entity* ep, bool& remove, bool& allowDragDrop, bool& shiftSelected);
	void EntitySecondaryWidget(Entity* ep, EntityCollection* parentp);
	void EntityListWidget(std::vector<Entity*>& list);
	void EntitySecondaryListWidget(std::vector<Entity*>& list, EntityCollection* parent);
	bool TransformWidget(Transform* p);
	bool SceneCursorWidget(SceneCursor* sc);
	bool SceneTransformWidget(SceneTransform* st);
	void GuiOptions();
	bool TorusMeshWidget(TorusGenerator* torus);
	bool SaveLoadWidget(std::span<char> buffer, bool& save, bool& load);
	void TextureWidget(ID3D11ShaderResourceView* tex, std::vector<bool>& areas);

	void ShowBicubicSurfacePopup(BicubicSurfaceParams& data);
	void BicubicSurfacePopup(BicubicSurfaceParams& data, bool& modified, bool& create);

	void ShowIntersectPopup();
	void IntersectPopup(float& detail, int& texResolution, bool& create);
}