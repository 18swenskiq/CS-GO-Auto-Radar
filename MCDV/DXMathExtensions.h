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
	static DX::XMFLOAT3 MultiplyFloat3(DX::XMFLOAT3 a, DX::XMFLOAT3 b)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);
		DX::XMVECTOR c_V = DX::XMVectorMultiply(a_V, b_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, c_V);
		return retval;
	}
	static DX::XMFLOAT3 MultiplyFloat3(DX::XMFLOAT3 a, float b)
	{
		DX::XMFLOAT3 retval;

		retval.x = a.x * b;
		retval.y = a.y * b;
		retval.z = a.z * b;
		return retval;
	}
	static DX::XMFLOAT3 DivideFloat3(DX::XMFLOAT3 a, float b)
	{
		DX::XMFLOAT3 retval;

		retval.x = a.x / b;
		retval.y = a.y / b;
		retval.z = a.z / b;
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
	static float DotFloat3(DX::XMFLOAT3 a, DX::XMFLOAT3 b)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);
		DX::XMVECTOR c_V = DX::XMVector3Dot(a_V, b_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, c_V);
		return retval.x;
	}
	static DX::XMFLOAT3 NormalizeFloat3(DX::XMFLOAT3 a)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMVector3Normalize(a_V);

		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, b_V);
		return retval;
	}
	static DX::XMFLOAT3 LerpFloat(DX::XMFLOAT3 a, DX::XMFLOAT3 b, float c)
	{
		DX::XMVECTOR a_V = DX::XMLoadFloat3(&a);
		DX::XMVECTOR b_V = DX::XMLoadFloat3(&b);

		DX::XMVECTOR d_V = DX::XMVectorLerp(a_V, b_V, c);
		DX::XMFLOAT3 retval;
		DX::XMStoreFloat3(&retval, d_V);
		return retval;
	}
	static DX::XMFLOAT3 GetNormal(DX::XMFLOAT3 a, DX::XMFLOAT3 b, DX::XMFLOAT3 c)
	{
		DX::XMFLOAT3 crossresult = CrossFloat3(SubtractFloat3(a, c), SubtractFloat3(b, c));
		return NormalizeFloat3(crossresult);


	}
};