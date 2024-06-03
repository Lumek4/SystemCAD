#pragma once
#include"component.h"
#include"pointSource.h"
#include"transform.h"
#include"event.hpp"
#include"entityCollection.hpp"
#include<memory>

class PointCollection : public Component, public EntityCollection, public PointSource
{
public:
	PointCollection(Entity& owner);
#pragma region EntityCollectionMembers
	bool Contains(Entity* v) const override;
	bool Remove(Entity* v) override;
	bool Add(Entity* v) override;
	bool IsMutable() override { return isMutable; }
	bool isMutable = true;
#pragma endregion

	const char* Name() const override { return "PointCollection"; }


	DirectX::XMFLOAT3 GetPoint(int i) override;
	int GetCount() override;
	Event<NeedRedrawEventData>& NeedRedrawEvent() override
	{return onCollectionModified;}
	
	Event<NeedRedrawEventData> onCollectionModified;
private:
	static Event<Transform*>::Reaction::Function ForwardModifiedFunction;
	Event<Transform*>::Reaction ForwardModified;
	static Event<Entity*>::Reaction::Function ForwardSelectionFunction;
	Event<Entity*>::Reaction ForwardSelection;
	static Event<Entity*>::Reaction::Function CleanupFunction;
	Event<Entity*>::Reaction Cleanup;
};