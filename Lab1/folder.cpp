#include "folder.h"
#include"catalogue.hpp"

Folder::Folder(Entity& owner)
	:Component(ComponentConstructorArgs(Folder)),
	EntityCollection(*(Component*)this)
{
}

bool Folder::Contains(Entity* v) const
{
	for (int i = 0; i < entities.size(); i++)
		if (entities[i] == v)
			return true;
	return false;
}
