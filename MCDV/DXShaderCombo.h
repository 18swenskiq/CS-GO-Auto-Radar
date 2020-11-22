#pragma once
#include <wrl.h>
#include <d3d11.h>

class DXShaderCombo
{
public:
	DXShaderCombo(Microsoft::WRL::ComPtr<ID3D11VertexShader> in_vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> in_ps);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> in_vs);
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> in_ps);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
};

DXShaderCombo::DXShaderCombo(Microsoft::WRL::ComPtr<ID3D11VertexShader> in_vs, Microsoft::WRL::ComPtr<ID3D11PixelShader> in_ps)
{
	vs = in_vs;
	ps = in_ps;
}


Microsoft::WRL::ComPtr<ID3D11VertexShader> DXShaderCombo::GetVertexShader()
{
	return vs;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> DXShaderCombo::GetPixelShader()
{
	return ps;
}

void DXShaderCombo::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> in_vs)
{
	vs = in_vs;
}

void DXShaderCombo::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> in_ps)
{
	ps = in_ps;
}

