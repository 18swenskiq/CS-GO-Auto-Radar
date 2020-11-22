#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <sstream>

class DXRendering
{
public:
	// TODO: add exception stuff
	DXRendering();
	DXRendering(const DXRendering&) = delete;
	DXRendering& operator=(const DXRendering&) = delete;
	~DXRendering() = default;
	void ClearBuffer(float red, float green, float blue) noexcept;
	void PrintVersion() noexcept;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(const std::wstring& path);
	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(const std::wstring& path);
private:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
};