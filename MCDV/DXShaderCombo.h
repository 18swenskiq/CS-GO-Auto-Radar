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
	void SetUnsignedInfo(DXRendering& dx, unsigned int value);
	void SetMatrixModel(DXRendering& dxr, DX::XMMATRIX value);
	void SetVec2Origin(DXRendering& dxr, DX::XMFLOAT2 value);
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

// This will have to be adjusted in the shader's HLSL
void DXShaderCombo::SetUnsignedInfo(DXRendering& dxr, unsigned int value)
{
	struct InfoConstantBuffer
	{
		unsigned int in_int;
	};

	InfoConstantBuffer m_ConstantBufferData;

	m_ConstantBufferData.in_int = value;

	CD3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(InfoConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	ID3D11Buffer* g_pConstantBuffer = NULL;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &m_ConstantBufferData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	dxr.pDevice->CreateBuffer(&cbDesc, &InitData, &g_pConstantBuffer);

	dxr.pContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}

void DXShaderCombo::SetMatrixModel(DXRendering& dxr, DX::XMMATRIX value)
{
	struct ModelConstantBuffer
	{
		DX::XMMATRIX in_m;
	};

	ModelConstantBuffer m_ConstantBufferData;

	m_ConstantBufferData.in_m = value;

	CD3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(ModelConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	ID3D11Buffer* g_pConstantBuffer = NULL;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &m_ConstantBufferData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	dxr.pDevice->CreateBuffer(&cbDesc, &InitData, &g_pConstantBuffer);

	dxr.pContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}

void DXShaderCombo::SetVec2Origin(DXRendering& dxr, DX::XMFLOAT2 value)
{
	struct OriginConstantBuffer
	{
		DX::XMFLOAT2 in_o;
	};

	OriginConstantBuffer m_ConstantBufferData;

	m_ConstantBufferData.in_o = value;

	CD3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(OriginConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	ID3D11Buffer* g_pConstantBuffer = NULL;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &m_ConstantBufferData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	dxr.pDevice->CreateBuffer(&cbDesc, &InitData, &g_pConstantBuffer);

	dxr.pContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}


