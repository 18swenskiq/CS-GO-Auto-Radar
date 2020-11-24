#pragma once
#include "buildmode.h"

#ifdef DXBUILD
#include <DirectXMath.h>
#endif

#ifdef GLBUILD
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#endif

#include <string>

//Linear interpolation between two floats, given time
static float lerpf(float a, float b, float f) {
	return (a * (1.0f - f)) + (b * f);
}

template<typename T>
static T lerpT(T a, T b, float f) {
	return (T)((float)a * (1.0f - f)) + ((float)b * f);
}

namespace util {
	static float roundf(float a, float r) {
		#ifdef DXBUILD
		return round(a / r) * r;
		#endif
		#ifdef GLBUILD
		return glm::round(a / r) * r;
		#endif
	}
}

#ifdef DXBUILD
static DirectX::XMFLOAT3 lerp(DirectX::XMFLOAT3 a, DirectX::XMFLOAT3 b, float f)
{
	return DirectX::XMFLOAT3(lerpf(a.x, b.x, f), lerpf(a.y, b.y, f), lerpf(a.z, b.z, f));
}
#endif

#ifdef GLBUILD
static glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float f) {
	return glm::vec3(lerpf(a.x, b.x, f),
		lerpf(a.y, b.y, f),
		lerpf(a.z, b.z, f));
}
#endif

inline float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

#ifdef DXBUILD
DirectX::XMFLOAT4 parseVec4(std::string src)
{
	DirectX::XMFLOAT4 out = DirectX::XMFLOAT4(0, 0, 0, 1);
	std::vector<std::string> strings;
	strings = split(src);

	if (strings.size() >= 1) out.x = (float)::atoi(strings[0].c_str()) / 255.0f;
	if (strings.size() >= 2) out.y = (float)::atoi(strings[1].c_str()) / 255.0f;
	if (strings.size() >= 3) out.z = (float)::atoi(strings[2].c_str()) / 255.0f;
	if (strings.size() >= 4) out.w = (float)::atoi(strings[3].c_str()) / 255.0f;

	return out;
}
#endif

#ifdef GLBUILD
glm::vec4 parseVec4(std::string src) {
	glm::vec4 out = glm::vec4(0, 0, 0, 1);
	std::vector<std::string> strings;
	strings = split(src);

	if (strings.size() >= 1) out.r = (float)::atoi(strings[0].c_str()) / 255.0f;
	if (strings.size() >= 2) out.g = (float)::atoi(strings[1].c_str()) / 255.0f;
	if (strings.size() >= 3) out.b = (float)::atoi(strings[2].c_str()) / 255.0f;
	if (strings.size() >= 4) out.a = (float)::atoi(strings[3].c_str()) / 255.0f;

	return out;
}
#endif