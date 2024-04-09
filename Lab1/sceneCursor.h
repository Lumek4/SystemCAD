#pragma once
#include<DirectXMath.h>

class SceneCursor;
namespace MyGui
{
	extern bool SceneCursorWidget(SceneCursor* sc);
}
class SceneCursor
{
	friend bool MyGui::SceneCursorWidget(SceneCursor* sc);
	SceneCursor() = default;
	DirectX::XMFLOAT2 screen{};
	DirectX::XMFLOAT3 world{};
	bool sChanged = false;
public:
	static SceneCursor instance;
	const DirectX::XMFLOAT4X4 *view, *proj, *invview, *invproj;
public:
	void Correct(
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& proj,
		const DirectX::XMFLOAT4X4& invview,
		const DirectX::XMFLOAT4X4& invproj
	);
	const DirectX::XMFLOAT2& GetScreen() { return screen; }
	const DirectX::XMFLOAT3& GetWorld() { return world; }
};

