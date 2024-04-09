#pragma once
#include"component.h"
#include<DirectXMath.h>
#include"myMat.h"
#include"event.hpp"

class Entity;
class Transform
{
public:
	Transform();
	Transform* parent = nullptr;
	DirectX::XMFLOAT3 localPosition{};
	DirectX::XMFLOAT3 scale{ 1,1,1 };
	DirectX::XMFLOAT4 rotation{ 0,0,0,1 };

	Event<Transform*> onModified{ };

	DirectX::XMFLOAT3 Position();
	void SetPosition(DirectX::XMFLOAT3 value);

	virtual MyMat Get() const;
	virtual MyMat GetInverse() const;

	void Translate(DirectX::XMFLOAT3 vector);
	void Rotate(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 axis, float angle);
	void Scale(DirectX::XMFLOAT3 point, float factor);
protected:
	Event<Entity*>::Reaction AnchorOnSelection;
};

class ModelTransform : public Component, public Transform
{
public:
	ModelTransform(Entity& owner);
	const char* Name() const override { return "ModelTransform"; }
};

class PointTransform : public Component, public Transform
{
public:
	PointTransform(Entity& owner);
	const char* Name() const override { return "PointTransform"; }

	MyMat Get() const override;
	MyMat GetInverse() const override;
};
class VPointCollection;
class VPointTransform : public Component, public Transform
{
public:
	VPointTransform(Entity& owner);
	const char* Name() const override { return "VPointTransform"; }

	MyMat Get() const override;
	MyMat GetInverse() const override;
	Event<void>::Reaction AutoDeselect;
	int index;
};

class SceneTransform : public Transform
{
	SceneTransform() = default;
	bool IsReset();
public:
	static SceneTransform instance;
	static const SceneTransform identity;
	void Reset();
	void Apply();
	MyMat Get() const override;
	MyMat GetInverse() const override;
	bool local;
	DirectX::XMFLOAT3 localOrigin{};
	DirectX::XMFLOAT3 origin{};

	DirectX::XMFLOAT3 selectionCenter{};
	DirectX::XMFLOAT3 translation{};
	float rotationAngle = 0, scaleFactor = 1;
	DirectX::XMFLOAT3 axis{0,0,1};
	enum class Type
	{
		center, cursor, local, highlight, global
	} type;
};

