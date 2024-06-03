#pragma once
#include"component.h"
#include"transform.h"
#include"event.hpp"
#include<memory>
#include"entity.hpp"
#include"pointSource.h"
//class SplineGenerator;

class VPointCollection : public Component, public PointSource
{
public:
	VPointCollection(Entity& owner);

	const char* Name() const override { return "VPointCollection"; }

	std::vector<Entity*> points;
	Entity* selection = nullptr;
	//int lastModifiedIndex = -1;

	void Resize(int count);
	DirectX::XMFLOAT3 GetPoint(int i) override;
	int GetCount() override;
	Event<NeedRedrawEventData> onCollectionModified;
	Event<NeedRedrawEventData>& NeedRedrawEvent() override
	{ return onCollectionModified; }

	void ShowVirtualPoints(bool v);
	bool ShowingVirtualPoints() { return showVirtualPoints; }
private:
	bool modified = true;
	bool showVirtualPoints = true;

	//bool selected;
	static Event<Entity*>::Reaction::Function DeletePointsFunction;
	Event<Entity*>::Reaction DeletePoints;
	static Event<Transform*>::Reaction::Function ForwardModifiedFunction;
	Event<Transform*>::Reaction ForwardModified;
	static Event<Entity*>::Reaction::Function ForwardSelectionFunction;
	Event<Entity*>::Reaction ForwardSelection;
	static Event<Entity*>::Reaction::Function PreSelectionSceneTransformPurgeFunction;
	Event<Entity*>::Reaction PreSelectionSceneTransformPurge;
	std::vector<std::unique_ptr<Event<Entity*>::Reaction>> ElementDeletion;
};