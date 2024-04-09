#pragma once
#include"component.h"
#include"entityCollection.hpp"

class Folder : public Component, public EntityCollection
{

public:
	Folder(Entity& owner);
	const char* Name() const override { return "Folder"; }

#pragma region EntityCollectionMembers
	bool Contains(Entity* v) const override;
	bool IsMutable() override { return isMutable; }
#pragma endregion
	bool isMutable = true;
};