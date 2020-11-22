#pragma once
#include <vector>
#include "dxrendering.h"

class DXVertexBuffer
{
public:
	DXVertexBuffer(DXRendering& dxr, const std::vector<float>& vertices)
		:
		stride(sizeof(float))
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(float) * vertices.size());
		bd.StructureByteStride = sizeof(float);
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();
		dxr.pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);
	}
	void Bind(DXRendering& dxr) noexcept
	{
		dxr.pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, 0u);
	}
protected:
	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
};