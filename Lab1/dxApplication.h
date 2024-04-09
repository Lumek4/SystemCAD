#pragma once
#include"windowApplication.h"
#include"dxDevice.h"
class DxApplication : public mini::WindowApplication {
public:
	explicit DxApplication(HINSTANCE hInstance);
	~DxApplication();
protected:
	virtual void GUI() {}
	virtual void Update() {}
	virtual void Render() {}
	int MainLoop() override;
	bool ProcessMessage(mini::WindowMessage& msg) override;

protected:
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	Viewport m_viewport;
	DxDevice m_device;

	virtual void OnResize() { }
private:

	bool m_resizing = false;
	bool m_minimized = false;
	void ResizeWindow(SIZE);
	void ResetRenderTarget();
};