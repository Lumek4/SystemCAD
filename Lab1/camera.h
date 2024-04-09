#pragma once
#include"dxDevice.h"
#include<DirectXMath.h>
#include"myMat.h"
#include"event.hpp"

class Camera
{
public:
	static std::unique_ptr<Camera> mainCamera;
	explicit Camera(DxDevice& device);
	MyMat Transform();
	MyMat ReverseTransform();
	MyMat BilboardTransform();
	
	void Rotate(DirectX::XMFLOAT2 delta);
	void Move(DirectX::XMFLOAT2 delta);
	DirectX::XMFLOAT3 GetOffset() { return translation; }
	void Zoom(float delta);
	inline float GetZoom() { return distance; }
	Event<Camera*> onMovement{ };

private:
	DirectX::XMFLOAT3 rotations{};
	DirectX::XMFLOAT3 translation{};
	float distance;


	DxDevice& device;
};

