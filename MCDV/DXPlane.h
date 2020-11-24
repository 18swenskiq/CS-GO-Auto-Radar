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
		float det = DXME::DotFloat3(DXME::CrossFloat3(p1.normal, p2.normal), p3.normal);
		float epsilon = 1e-5f; // Value for floating point error

		if (det < epsilon && det > -epsilon) { return false; };

		auto part1 = DXME::SubtractFloat3(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), (DXME::MultiplyFloat3(DXME::CrossFloat3(p2.normal, p3.normal), p1.offset)));
		auto part2 = DXME::MultiplyFloat3(DXME::CrossFloat3(p3.normal, p1.normal), p2.offset);
		auto part3 = DXME::MultiplyFloat3(DXME::CrossFloat3(p1.normal, p2.normal), p3.offset);
		auto part4 = DXME::SubtractFloat3(part1, part2);
		auto part5 = DXME::SubtractFloat3(part4, part3);
		auto retval = DXME::DivideFloat3(part5, det);

		*p = retval;

		return true;
	}
	static bool FinalThreePlaneIntersection(DXPlane p1, DXPlane p2, DXPlane p3, DirectX::XMFLOAT3* p)
	{
		float det = DXME::DotFloat3(DXME::CrossFloat3(p1.normal, p2.normal), p3.normal);
		float epsilon = 1e-5f;

		if (det < epsilon && det > -epsilon) { return false; };

		// this makes me want to die
		auto a = -p1.offset;
		auto b = DXME::CrossFloat3(p2.normal, p3.normal);
		auto c = p2.offset;
		auto d = DXME::CrossFloat3(p3.normal, p1.normal);
		auto e = p3.offset;
		auto f = DXME::CrossFloat3(p1.normal, p2.normal);
		auto f5 = DXME::CrossFloat3(p2.normal, p3.normal);
		auto g = DXME::DotFloat3(p1.normal, f5);

		auto s11 = DXME::MultiplyFloat3(b, a);
		auto s12 = DXME::MultiplyFloat3(d, c);
		auto s13 = DXME::MultiplyFloat3(f, e);

		auto s21 = DXME::SubtractFloat3(s11, s12);
		auto s22 = DXME::SubtractFloat3(s21, s13);
		auto s23 = DXME::SubtractFloat3(DX::XMFLOAT3(1.0f, 1.0f, 1.0f), s22);

		auto s31 = DXME::DivideFloat3(s23, g);

		*p = s31;


	}


	static float EvalPointPolarity(DXPlane plane, DirectX::XMFLOAT3 p1)
	{
		return DXME::DotFloat3(p1, plane.normal) - DXME::DotFloat3(DXME::MultiplyFloat3(plane.normal, plane.offset), plane.normal);
	}

	static void InPlaceOrderCoplanarClockWise(DXPlane plane, std::vector<DirectX::XMFLOAT3>* Points)
	{
		if (Points->size() == 0) return;

		// Find center point (average distribution of points)
		DirectX::XMFLOAT3 center(0, 0, 0);
		for (int i = 0; i < Points->size(); i++)
		{
			center = DXME::AddFloat3(center, (*Points)[i]);
		}

		center = DXME::DivideFloat3(center, Points->size());

		DirectX::XMFLOAT3 ref = DXME::SubtractFloat3((*Points)[0], center);

		std::vector<DirectX::XMFLOAT4> angledVecs;

		for (int i = 0; i < Points->size(); i++)
		{
			DirectX::XMFLOAT3 diff = DXME::SubtractFloat3((*Points)[i], center);
			float ang = atan2(DXME::GetLength(DXME::CrossFloat3(diff, ref)), DXME::DotFloat3(diff, ref));

			float sign = DXME::DotFloat3(DXME::CrossFloat3(diff, ref), plane.normal) < 0 ? -1.0f : 1.0f;
			ang *= sign;

			angledVecs.push_back(DirectX::XMFLOAT4((*Points)[i].x, (*Points)[i].y, (*Points)[i].z, ang));
		}

		while (true) {
			bool modified = false;

			for (int i = 0; i < Points->size() - 1; i++)
			{
				int s0 = i; int s1 = i + 1;

				DirectX::XMFLOAT4 a = angledVecs[s0]; DirectX::XMFLOAT4 b = angledVecs[s1];

				if (a.w > b.w) {
					angledVecs[s0] = b; angledVecs[s1] = a;
					modified = true;
				}
			}
			if (!modified) break;
		}

		for (int i = 0; i < Points->size(); i++)
		{
			(*Points)[i] = DirectX::XMFLOAT3(angledVecs[i].x, angledVecs[i].y, angledVecs[i].z);
		}
	}

};