#include "camera.h"
#include"window.h"
using namespace DirectX;
using namespace mini;

Camera::Camera(DxDevice& device)
	:device(device), distance(10)
{
}
std::unique_ptr<Camera> Camera::mainCamera = nullptr;

MyMat Camera::BilboardTransform()
{
	return MyMRot({ 0, -rotations.y, 0 }) *
		MyMRot({ 0, 0, -rotations.z });
}

void Camera::Rotate(DirectX::XMFLOAT2 delta)
{
	rotations = { 0,
		max(min(rotations.y - delta.y, 0), -XM_PI),
		rotations.z - delta.x };
	onMovement.Notify(this);
}

void Camera::Move(DirectX::XMFLOAT2 delta)
{
	XMFLOAT4 vec4{delta.x, delta.y, 0, 0};
	auto vec = XMVector4Transform(XMLoadFloat4(&vec4), ReverseTransform())*distance;


	XMStoreFloat4(&vec4, vec);
	translation =
		DirectX::XMFLOAT3{
			translation.x - vec4.x,
			translation.y - vec4.y,
			translation.z - vec4.z
	};
	onMovement.Notify(this);
}

void Camera::Zoom(float delta)
{
	distance *= ((delta < 0) ? 1 / (1 - delta) : 1 + delta);
	onMovement.Notify(this);
}

MyMat Camera::Transform()
{
	return MyMTrans(translation) *
		MyMRot({ 0, 0, rotations.z }) *
		MyMRot({ 0, rotations.y, 0 }) *
		MyMTrans({ 0,0,distance });
}
MyMat Camera::ReverseTransform()
{
	return MyMTrans({ 0,0,-distance }) *
		MyMRot({ 0, -rotations.y, 0 }) *
		MyMRot({ 0, 0, -rotations.z }) *
		MyMTrans({-translation.x, -translation.y, -translation.z});
}
