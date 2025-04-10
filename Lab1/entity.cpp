#include "entity.hpp"
#include"transform.h"
#include"all_components.h"
#include"vecmath.h"



Entity* Entity::New()
{
	return objects.emplace_back(new Entity()).get();
}


void Entity::Delete()
{
	if (currentlyDeleting)
		return;
	currentlyDeleting = true;
	Select(false);
	for (int i = 0; i < registered.size(); i++)
	{
		void* e = registered[i]._Myfirst._Val;
		void* r = registered[i]._Get_rest()._Myfirst._Val;

		registered[i]._Get_rest()._Get_rest()._Myfirst._Val(e,r);
	}
	for (int i = containingCollections.size() - 1; i >= 0; i--)
	{
		/*if (!containingCollections[i]->IsMutable())
			containingCollections[i]->component.owner.Delete();
		else*/
			containingCollections[i]->Remove(this);
	}
	preDelete.Notify(this);
	//assert(0 && "Deleting a non-existent entity.");

}
void Entity::FinalizeDeletions()
{
	bool finished = false;
	while (!finished)
	{
		auto end = objects.end();
		auto it = objects.begin();
		for (; it < end; it++)
			if ((*it).get()->currentlyDeleting)
			{
				objects.erase(it);
				break;
			}
		if (it == end)
			finished = true;
	}
}
bool Entity::Merge(Entity* other)
{
	for (int i = 0; i < containingCollections.size(); i++)
	{
		auto* c = (PointCollection*)containingCollections[i];
		if (c)
			c->TransferRelations(this, other);
	}
	Delete();
	return true;
}
Event<void> Entity::preAnySelect{};
void Entity::Select(bool selected)
{
	if (selected != this->selected)
	{
		preSelect.Notify(this);
		preAnySelect.Notify();
		this->selected = selected;
		if (selected)
		{
			selection.push_back(this);
		}
		else
		{
			for (auto it = selection.begin(); it != selection.end(); it++)
				if (*it == this)
				{
					selection.erase(it);
					break;
				}
		}
		onSelect.Notify(this);
	}
}

std::string_view Entity::GetName()
{
	return std::string_view(name);
}


void Entity::SetName(std::string_view name)
{
	this->name = name;
}
std::vector<Entity*> Entity::selection{};
std::vector<std::unique_ptr<Entity>> Entity::objects{};
