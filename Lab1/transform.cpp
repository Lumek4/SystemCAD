#include "transform.h"
#include"entity.hpp"
#include"vecmath.h"
#include"all_components.h"
#include"sceneCursor.h"
using namespace DirectX;


Transform::Transform()
{
	AnchorOnSelection = [this](Entity* e) {
		parent = e->Selected() ? &SceneTransform::instance : nullptr;
	};
}

DirectX::XMFLOAT3 Transform::Position()
{
	if (parent)
	{
		XMFLOAT4 res = { localPosition.x, localPosition.y, localPosition.z, 1 };
		XMStoreFloat4(&res,
			XMVector4Transform(XMLoadFloat4(&res), parent->Get())
			);
		return { res.x, res.y, res.z };
	}
	else
		return localPosition;
}

void Transform::SetPosition(DirectX::XMFLOAT3 value)
{
	if (parent)
	{
		XMFLOAT4 v = { value.x, value.y, value.z, 1 };
		XMStoreFloat4(&v,
			XMVector4Transform(XMLoadFloat4(&v), parent->GetInverse())
		);
		localPosition = { v.x, v.y, v.z };
	}
	else
		localPosition = value;
}

MyMat Transform::Get() const
{
	auto mat = MyMScale(scale) * MyMRot(rotation) * MyMTrans(localPosition);
	if (parent == nullptr)
		return mat;
	else
		return mat * parent->Get();
}

MyMat Transform::GetInverse() const
{
	auto mat = MyMTrans(localPosition).Invert() * MyMRot(rotation).Invert() * MyMScale(scale).Invert();
	if (parent == nullptr)
		return mat;
	else
		return parent->GetInverse() * mat;
}

void Transform::Translate(DirectX::XMFLOAT3 vector)
{
	localPosition = localPosition + vector;
	onModified.Notify(this);
}
void Transform::Rotate(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 axis, float angle)
{
	auto rel = localPosition - point;
	auto newRotation = Quaternion::Get(axis, angle);
	rel = Quaternion::RotateByQuaternion(rel, newRotation);


	XMStoreFloat4(&rotation,
		XMQuaternionMultiply(
			XMLoadFloat4(&newRotation),
			XMLoadFloat4(&rotation)
		)
	);
	localPosition = point + rel;
	onModified.Notify(this);
}
void Transform::Scale(DirectX::XMFLOAT3 point, float factor)
{
	auto rel = localPosition - point;
	rel = rel * factor;
	scale = scale * factor;
	localPosition = point + rel;
	onModified.Notify(this);
}


PointTransform::PointTransform(Entity& owner)
	:Component(ComponentConstructorArgs(PointTransform))
{
	owner.onSelect += AnchorOnSelection;
}
MyMat PointTransform::Get() const
{
	if (parent == nullptr)
		return MyMTrans(localPosition);
	else
		return MyMTrans(localPosition) * parent->Get();
}
MyMat PointTransform::GetInverse() const
{
	if (parent == nullptr)
		return MyMTrans(localPosition).Invert();
	else
		return parent->GetInverse() * MyMTrans(localPosition).Invert();
}
ModelTransform::ModelTransform(Entity& owner)
	:Component(ComponentConstructorArgs(ModelTransform))
{
	owner.onSelect += AnchorOnSelection;
}


SceneTransform SceneTransform::instance{};
const SceneTransform SceneTransform::identity{};

void SceneTransform::Reset()
{
	if (IsReset())
		return;
	localPosition = {};
	scale = { 1, 1, 1 };
	rotation = { 0,0,0,1 };
	selectionCenter = {};
	translation = {};
	rotationAngle = 0;
	axis = { 0,0,1 };
	scaleFactor = 1;
	onModified.Notify(this);
}
bool SceneTransform::IsReset()
{
	int v = 0;
	v |= memcmp(&localPosition, &identity.localPosition, sizeof(localPosition));
	v |= memcmp(&scale, &identity.scale, sizeof(scale));
	v |= memcmp(&rotation, &identity.rotation, sizeof(rotation));
	v |= memcmp(&type, &identity.type, sizeof(type));
	return !v;
}
void SceneTransform::Apply()
{
	auto selectedTransforms = Entity::GetSelected<Transform>();
	for (int i = 0; i < selectedTransforms.size(); i++)
	{
		auto* transform = selectedTransforms[i];
		XMFLOAT3 offset;
		switch (type)
		{
		case SceneTransform::Type::global:
			offset = {};
			break;
		case SceneTransform::Type::highlight:
			offset = selectedTransforms[i]->localPosition;
			break;
		case SceneTransform::Type::center:
			offset = selectionCenter;
			break;
		case SceneTransform::Type::cursor:
			offset = SceneCursor::instance.GetWorld();
			break;
		case SceneTransform::Type::local:
			offset = transform->localPosition;
			break;
		}
		onModified.enabled = false;
		transform->Scale(offset, scaleFactor);
		transform->Rotate(offset, axis, rotationAngle);
		onModified.enabled = true;
		transform->Translate(translation);
	}
	SceneTransform::instance.Reset();
}
MyMat SceneTransform::Get() const
{
	if (local)
		return
		MyMTrans(-localOrigin) *
		MyMScale(scale) *
		MyMRot(rotation) *
		MyMTrans(localPosition + localOrigin);
	else
		return
		MyMTrans(-origin) *
		MyMScale(scale) *
		MyMRot(rotation) *
		MyMTrans(localPosition + origin);
}

MyMat SceneTransform::GetInverse() const
{
	if (local)
		return
		MyMTrans(localPosition + localOrigin).Invert() *
		MyMRot(rotation).Invert() *
		MyMScale(scale).Invert() *
		MyMTrans(-localOrigin).Invert();
	else
		return
		MyMTrans(localPosition + origin).Invert() *
		MyMRot(rotation).Invert() *
		MyMScale(scale).Invert() *
		MyMTrans(-origin).Invert();
}


VPointTransform::VPointTransform(Entity& owner)
	:Component(ComponentConstructorArgs(VPointTransform)),
	index(-1)
{
	owner.hideInList = true;
	owner.onSelect += AnchorOnSelection;
	AutoDeselect = [this]() {
		this->owner.Unregister(Entity::preAnySelect, AutoDeselect);
		this->owner.Select(false);
	};
}

MyMat VPointTransform::Get() const
{
	if (parent == nullptr)
		return MyMTrans(localPosition);
	else
		return MyMTrans(localPosition) * parent->Get();
}

MyMat VPointTransform::GetInverse() const
{
	if (parent == nullptr)
		return MyMTrans(localPosition).Invert();
	else
		return parent->GetInverse() * MyMTrans(localPosition).Invert();
}