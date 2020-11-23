#pragma once
#include <iostream>
#include <DirectXMath.h>
#include "DXMathExtensions.h"
#include <vector>
#include <math.h>

class DXPlane {
public:
	DirectX::XMFLOAT3 normal;
	float offset;
	
	int textureID = 0;
	// Plane defined by three points
	DXPlane(DirectX::XMFLOAT3 P, DirectX::XMFLOAT3 Q, DirectX::XMFLOAT3 R)
	{
		DirectX::XMFLOAT3 n = DXME::CrossFloat3(DXME::SubtractFloat3(P, Q), DXME::SubtractFloat3(P, R)); // Calculate normal

		// TODO: Figure out how this returns a float?
		float d = DXME::DotFloat3(DXME::NormalizeFloat3(n), P);

		// Set class attribs
		this->normal = DXME::NormalizeFloat3(n);
		this->offset = d;
	}

	// Direct constructor
	DXPlane(DirectX::XMFLOAT3 normal, float offset)
	{
		this->offset = offset;
		this->normal = normal;
	}

	// Standard constructor (generic floor plane)
	DXPlane()
	{
		this->offset = 0.0f;
		this->normal = DirectX::XMFLOAT3(0, 0, 1);
	}

	// Descructor
	~DXPlane() {};

	static bool ThreePlaneIntersection(DXPlane p1, DXPlane p2, DXPlane p3, DirectX::XMFLOAT3* p)
	{
		// TODO: how tf am I supposed to get a float from this
		float det = DXME::DotFloat3(DXME::CrossFloat3(p1.normal, p2.normal), p3.normal);
		float epsilon = 1e-5f; // Value for floating point error

		if (det < epsilon && det > -epsilon) { return false; };

		// TODO: Fill this in

		return true;
	}

	static float EvalPointPolarity(DXPlane plane, DirectX::XMFLOAT3 p1)
	{
		return DXME::DotFloat3(p1, plane.normal) - DXME::DotFloat3(DXME::MultiplyFloat3(plane.normal, plane.offset), plane.normal);
	}
};