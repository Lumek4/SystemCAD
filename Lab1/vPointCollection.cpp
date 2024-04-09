#include "vPointCollection.h"
#include"catalogue.hpp"
#include"entity.hpp"
#include"all_components.h"
#include"entityPresets.h"

VPointCollection::VPointCollection(Entity& owner)
	:Component(ComponentConstructorArgs(VPointCollection))
{
	ForwardModified = [this](Transform* t) {
		int lastModifiedIndex = -1;
		for (int i = 0; i < points.size(); i++)
			if (points[i]->GetComponent<Transform>() == t ||
				dynamic_cast<SceneTransform*>(t) && points[i] == selection)
			{
				lastModifiedIndex = i;
			}
		onCollectionModified.Notify({lastModifiedIndex});
	};
	owner.Register(SceneTransform::instance.onModified, ForwardModified);

	ForwardSelection = [this](Entity* e) {
		if (e->Selected())
		{
			for (int i = Entity::selection.size() - 2; i >= 0; i--)
				if (Entity::selection[i] != &this->owner)
				{
					Entity::selection[i]->Select(false);
				}
			e->Register(Entity::preAnySelect, e->GetComponent<VPointTransform>()->AutoDeselect);
		}
		else
		{

			SceneTransform::instance.Reset();
			int lastModifiedIndex = e->GetComponent<VPointTransform>()->index;
			onCollectionModified.Notify({lastModifiedIndex});
		}
		selection = e->Selected() ? e : nullptr;
		onCollectionModified.Notify({-1});
	};
	PreSelectionSceneTransformPurge = [this](Entity* e) {
		if (!e->Selected())
		{
			SceneTransform::instance.Reset();
			int lastModifiedIndex = e->GetComponent<VPointTransform>()->index;
			onCollectionModified.Notify({lastModifiedIndex});
		}
	};


	DeletePoints = [this](Entity*)
	{
		for (int i = 0; i < points.size(); i++)
		{
			points[i]->Delete();
			points[i] = nullptr;
		}
	};
	owner.preDelete += DeletePoints;
}

void VPointCollection::Resize(int count)
{
	if (count < points.size())
	{
		while (count < points.size())
		{
			auto* p = points.back();
			points.pop_back();
			p->Delete();
		}
	}
	else
		while (count > points.size())
		{
			auto& p = points.emplace_back(Entity::New());
			auto* vPoint = p->AddComponent<VPointTransform>();
			vPoint->index = points.size() - 1;
			vPoint->onModified += ForwardModified;
			p->preSelect += PreSelectionSceneTransformPurge;
			p->onSelect += ForwardSelection;
			p->AddComponent<PointRenderer>();
			p->enabled = showVirtualPoints;
		}
	onCollectionModified.Notify({-1});
}

DirectX::XMFLOAT3 VPointCollection::GetPoint(int i)
{
	if (i >= 0 && i < points.size())
		return points[i]->GetComponent<Transform>()->Position();
	else
		return { NAN, NAN, NAN };
}

int VPointCollection::GetCount()
{
	return points.size();
}

void VPointCollection::ShowVirtualPoints(bool v)
{
	if (v == showVirtualPoints)
		return;
	showVirtualPoints = v;
	for (int i = 0; i < points.size(); i++)
	{
		if (v==false)
			points[i]->Select(false);
		points[i]->enabled = v;
	}
}
