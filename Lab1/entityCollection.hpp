#pragma once

#include"entity.hpp"
#include"event.hpp"

class EntityCollection
{
public:
	EntityCollection(Component& component)
		:component(component){}
	virtual bool Contains(Entity* v) const = 0;
	Event<int> onRemove;
	virtual bool Remove(Entity* v);
	Event<void> onAdd;
	virtual bool Add(Entity* v);
	virtual void AddSelection(const std::vector<Entity*>& v)
	{
		for (int i = 0; i < v.size(); i++)
		{
			Add(v[i]);
		}
	}
	Event<DirectX::XMINT2> onTransferRelations;
	virtual void TransferRelations(Entity* from, Entity* to);

	const std::vector<Entity*>& Get() const { return entities; }
	std::vector<Entity*>& Get() { return entities; }
	virtual bool IsMutable() = 0;

	virtual void Clear()
	{
		auto& l = Get();
		for (int i = l.size() - 1; i >= 0; i--)
			for (auto it = l[i]->containingCollections.begin(); it < l[i]->containingCollections.end(); it++)
				if (*it == this)
				{
					l[i]->containingCollections.erase(it);
					break;
				}
		l.clear();
	}

	Component& component;
	std::vector<Entity*> entities;
};