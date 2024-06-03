#include "vPointCollection.h"
#include"catalogue.hpp"
#include"entity.hpp"
#include"all_components.h"
#include"entityPresets.h"

void VPointCollection::ForwardModifiedFunction(void* arg, Transform* t)
{
	auto _this = (VPointCollection*)arg;
	int lastModifiedIndex = -1;
	for (int i = 0; i < _this->points.size(); i++)
		if (_this->points[i]->GetComponent<Transform>() == t ||
			dynamic_cast<SceneTransform*>(t) && _this->points[i] == _this->selection)
		{
			lastModifiedIndex = i;
		}
	_this->onCollectionModified.Notify({ lastModifiedIndex });
};
void VPointCollection::ForwardSelectionFunction(void* arg, Entity* e)
{
	auto _this = (VPointCollection*)arg;
	if (e->Selected())
	{
		for (int i = Entity::selection.size() - 2; i >= 0; i--)
			if (Entity::selection[i] != &_this->owner)
			{
				Entity::selection[i]->Select(false);
			}
		e->Register(Entity::preAnySelect, e->GetComponent<VPointTransform>()->AutoDeselect);
	}
	else
	{

		SceneTransform::instance.Reset();
		int lastModifiedIndex = e->GetComponent<VPointTransform>()->index;
		_this->onCollectionModified.Notify({ lastModifiedIndex });
	}
	_this->selection = e->Selected() ? e : nullptr;
	_this->onCollectionModified.Notify({ -1 });
}
void VPointCollection::PreSelectionSceneTransformPurgeFunction(void* arg, Entity* e)
{
	auto _this = (VPointCollection*)arg;
	if (!e->Selected())
	{
		SceneTransform::instance.Reset();
		int lastModifiedIndex = e->GetComponent<VPointTransform>()->index;
		_this->onCollectionModified.Notify({ lastModifiedIndex });
	}
}
void VPointCollection::DeletePointsFunction(void* arg, Entity*)
{
	auto _this = (VPointCollection*)arg;
	for (int i = 0; i < _this->points.size(); i++)
	{
		_this->points[i]->Delete();
		_this->points[i] = nullptr;
	}
}
VPointCollection::VPointCollection(Entity& owner)
	:Component(ComponentConstructorArgs(VPointCollection)),
	ForwardModified(this, ForwardModifiedFunction),
	ForwardSelection(this, ForwardSelectionFunction),
	PreSelectionSceneTransformPurge(this, PreSelectionSceneTransformPurgeFunction),
	DeletePoints(this, DeletePointsFunction)
{
	owner.Register(SceneTransform::instance.onModified, ForwardModified);
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
