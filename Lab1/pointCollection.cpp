#include "pointCollection.h"
#include"catalogue.hpp"
#include"entity.hpp"
#include"all_components.h"
#include<algorithm>

void PointCollection::ForwardModifiedFunction(void* arg, Transform* t)
{
	auto _this = (PointCollection*)arg;
	auto& l = _this->Get();
	for (int i = 0; i < l.size(); i++)
	{
		auto* et = l[i]->GetComponent<Transform>();
		if (et == t || (et != nullptr && et->parent == t))
			_this->onCollectionModified.Notify({ i });
	}
}
void PointCollection::ForwardSelectionFunction(void* arg, Entity* e)
{
	auto _this = (PointCollection*)arg;
	auto& l = _this->Get();
	auto it = std::find(l.begin(), l.end(), e);
	int i = (it == l.end()) ? -1 : (it - l.begin());
	_this->onCollectionModified.Notify({ i });
}
void PointCollection::CleanupFunction(void* arg, Entity* e)
{
	((PointCollection*)arg)->Clear();
}
PointCollection::PointCollection(Entity& owner)
	:Component(ComponentConstructorArgs(PointCollection)),
	EntityCollection(*(Component*)this),
	ForwardModified(this, ForwardModifiedFunction),
	ForwardSelection(this, ForwardSelectionFunction),
	Cleanup(this, CleanupFunction)
{
	owner.Register(SceneTransform::instance.onModified, ForwardModified);
	//SceneTransform::instance.onModified += ForwardModified;

	owner.preDelete += Cleanup;
	/*Unregister = [this](Entity&) {
		SceneTransform::instance.onModified -= ForwardModified;
		for (int i = 0; i < points.size(); i++)
		{
			points[i]->onSelect -= ForwardSelection;
			points[i]->preDelete -= *ElementDeletion[i].get();
			points[i]->GetComponent<Transform>()->onModified -= ForwardModified;
		}
	};
	owner.preDelete += Unregister;*/
}

bool PointCollection::Remove(Entity* v)
{
	if (EntityCollection::Remove(v))
	{
		owner.Unregister(v->onSelect, ForwardSelection);
		//v->onSelect -= ForwardSelection;
		owner.Unregister(v->GetComponent<PointTransform>()->onModified, ForwardModified);
		//v->GetComponent<PointTransform>()->onModified -= ForwardModified;

		onCollectionModified.Notify({ -1 });
		return true;
	}
	return false;
}

bool PointCollection::Contains(Entity* v) const
{
	for (int i = 0; i < entities.size(); i++)
		if (entities[i] == v)
			return true;
	return false;
}

bool PointCollection::Add(Entity* v)
{
	auto* t = v->GetComponent<PointTransform>();
	if (t && EntityCollection::Add(v))
	{
		owner.Register(v->onSelect, ForwardSelection);
		//v->onSelect += ForwardSelection;
		owner.Register(t->onModified, ForwardModified);
		//t->onModified += ForwardModified;
		onCollectionModified.Notify({-1});
		return true;
	}
	return false;
}




DirectX::XMFLOAT3 PointCollection::GetPoint(int i)
{
	if (i >= 0 && i < entities.size())
		return entities[i]->GetComponent<Transform>()->Position();
	else
		return {NAN, NAN, NAN};
}

int PointCollection::GetCount()
{
	return entities.size();
}