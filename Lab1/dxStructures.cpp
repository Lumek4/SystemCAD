#include "dxStructures.h"

SwapChainDescription::SwapChainDescription(HWND wndHwnd, SIZE wndSize)
{
	ZeroMemory(this, sizeof(SwapChainDescription));
	BufferDesc.Width = wndSize.cx;
	BufferDesc.Height = wndSize.cy;
	BufferDesc.RefreshRate.Numerator = 120;
	BufferDesc.RefreshRate.Denominator = 1;
	BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //0
	//BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //0
	SampleDesc.Quality = 0;
	SampleDesc.Count = 1;
	BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	BufferCount = 1;
	OutputWindow = wndHwnd;
	Windowed = true;
	//SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //0
	//Flags = 0;
}

Viewport::Viewport(SIZE size)
{
	this->TopLeftX = 0.0f;
	this->TopLeftY = 0.0f;
	this->Width = static_cast<FLOAT>(size.cx);
	this->Height = static_cast<FLOAT>(size.cy);
	this->MinDepth = 0.0f;
	this->MaxDepth = 1.0f;
}

Texture2DDescription::Texture2DDescription(UINT width, UINT height)
{
	ZeroMemory(this, sizeof(Texture2DDescription));
	Width = width;
	Height = height;
	//MipLevels = 0;
	ArraySize = 1;
	Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SampleDesc.Count = 1;
	SampleDesc.Quality = 0;
	Usage = D3D11_USAGE_DEFAULT;
	BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//CPUAccessFlags = 0;
	//MiscFlags = 0;
}
Texture2DDescription Texture2DDescription::CPUTextureDescription(UINT width, UINT height)
{
	Texture2DDescription desc(width, height);
	desc.MipLevels = 1;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	return desc;
}
Texture2DDescription Texture2DDescription::DepthStencilDescription(UINT width, UINT height)
{
	Texture2DDescription desc(width, height);
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	return desc;
}

BufferDescription::BufferDescription(UINT bindFlags, size_t byteWidth)
{
	ZeroMemory(this, sizeof(BufferDescription));
	BindFlags = bindFlags;
	ByteWidth = static_cast<UINT>(byteWidth);
	Usage = D3D11_USAGE_DYNAMIC;
	CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//MiscFlags = 0;
	//StructureByteStride = 0;
}

BufferDescription BufferDescription::ConstantBufferDescription(size_t byteWidth)
{
	BufferDescription desc{D3D11_BIND_CONSTANT_BUFFER, byteWidth };
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	return desc;
}

SamplerDescription::SamplerDescription()
{
	ZeroMemory(this, sizeof(SamplerDescription));
}

SamplerDescription SamplerDescription::PointSamplerDescription()
{
	SamplerDescription desc;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	return desc;
}

DepthStencilDescription::DepthStencilDescription()
{
	DepthEnable = true;
	DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthFunc = D3D11_COMPARISON_LESS;
	StencilEnable = false;
	StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	FrontFace.StencilFunc = BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	FrontFace.StencilDepthFailOp = BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	FrontFace.StencilPassOp = BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	FrontFace.StencilFailOp = BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
}

RasterizerStateDescription::RasterizerStateDescription()
{
	ZeroMemory(this, sizeof(RasterizerStateDescription));
	FillMode = D3D11_FILL_SOLID;
	CullMode = D3D11_CULL_NONE;
}
