#pragma once
#include <DirectXMath.h>

namespace DX = DirectX;

static class DXME
{
public:
	// Probably fucked pointers here
	static DX::XMFLOAT3 AddFloat3(DX::XMFLOAT3 a, DX::XMFLOAT3 b)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);
		DX::XMVECTOR c_V = DX::XMVectorAdd(a_V, b_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, c_V);
		return retval;
	}
	static DX::XMFLOAT3 SubtractFloat3(DX::XMFLOAT3 a, DX::XMFLOAT3 b)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);
		DX::XMVECTOR c_V = DX::XMVectorSubtract(a_V, b_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, c_V);
		return retval;
	}
	static DX::XMFLOAT3 CrossFloat3(DX::XMFLOAT3 a, DX::XMFLOAT3 b)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);
		DX::XMVECTOR c_V = DX::XMVector3Cross(a_V, b_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, c_V);
		return retval;
	}
	static DX::XMFLOAT3 NormalizeFloat3(DX::XMFLOAT3 a)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMVector3Normalize(a_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, b_V);
		return retval;
	}
};