#pragma once
#include"all_components.h"
#include"entity.hpp"
#include"mesh.h"
#include"sceneCursor.h"
#include"bicubicSurface.h"

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
	struct TorusMeshData
	{
		DirectX::XMINT2 division{ 10,10 };
		DirectX::XMFLOAT2 radii{ 2, 1 };
	};
	bool TorusMeshWidget(std::unique_ptr<Mesh>& m, TorusMeshData& data);

	void ShowBicubicSurfacePopup(BicubicSurfaceParams& data);
	void BicubicSurfacePopup(BicubicSurfaceParams& data, bool& modified, bool& create);
}