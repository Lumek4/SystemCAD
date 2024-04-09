#include"dxApplication.h"
#include<windowsx.h>
#include"imgui/imgui_impl_dx11.h"
#include"imgui/imgui_impl_win32.h"
#include"all_components.h"
#include"vecmath.h"
#include"myGui.h"
#include"colorPaletteh.h"
#include"entityPresets.h"

using namespace mini;
using namespace DirectX;
DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance), m_device(m_window), m_viewport(m_window.getClientSize())
{
	ID3D11Texture2D* temp = nullptr;
	m_device.swapChain()->GetBuffer(0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&temp));
	const dx_ptr<ID3D11Texture2D> backTexture{ temp };
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	SIZE wndSize = m_window.getClientSize();
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);
	ResetRenderTarget();
}

DxApplication::~DxApplication()
{
}


int DxApplication::MainLoop() {
	MSG msg{};
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if(!m_minimized) {
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			GUI();
			Update();
			Render();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			m_device.swapChain()->Present(1, 0);
		}
		else {
			Sleep(1000 / 60);
		}
	} while (msg.message != WM_QUIT);
	return msg.wParam;
}

bool DxApplication::ProcessMessage(mini::WindowMessage& msg)
{

	switch (msg.message)
	{
	case WM_ENTERSIZEMOVE:
		m_resizing = true;
		return true;
	case WM_SIZE:
		if (msg.wParam == SIZE_MINIMIZED)
		{
			if (!m_minimized)
			{
				m_minimized = true;
			}
		}
		else if (m_minimized)
		{
			m_minimized = false;
		}

		if (msg.wParam == SIZE_MINIMIZED || m_resizing)
			return true;
		[[fallthrough]];// else continue to WM_EXITSIZEMOVE
	case WM_EXITSIZEMOVE:
	{
		m_resizing = false;
		RECT r;
		GetClientRect(m_window.getHandle(), &r);
		SIZE size
		{
			size.cx = r.right - r.left,
			size.cy = r.bottom - r.top
		};
		if (size.cx != (LONG)m_viewport.Width || size.cy != (LONG)m_viewport.Height)
			ResizeWindow(size);

		return true;
	}
	case WM_PAINT:
	{
		if (!m_resizing)
			return false;
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		GUI();
		Update();
		Render();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		m_device.swapChain()->Present(1, 0);
		return true;
	}
	default:
		return false;
	}
	return false;
}

void DxApplication::ResizeWindow(SIZE s)
{
	if (!m_device.context() || !m_device.swapChain())
		return;
	m_device.context()->OMSetRenderTargets(0, nullptr, nullptr);

	m_backBuffer = nullptr;

	m_device.ResizeBuffers();


	ID3D11Texture2D* temp = nullptr;
	m_device.swapChain()->GetBuffer(0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&temp));
	const dx_ptr<ID3D11Texture2D> backTexture{ temp };
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	SIZE wndSize = m_window.getClientSize();
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);

	m_viewport.Height = s.cy;
	m_viewport.Width = s.cx;

	OnResize();
	ResetRenderTarget();
}

void DxApplication::ResetRenderTarget()
{
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());
	m_device.context()->RSSetViewports(1, &m_viewport);
}