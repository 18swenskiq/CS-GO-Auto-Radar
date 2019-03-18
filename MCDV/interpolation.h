#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//Linear interpolation between two floats, given time
static float lerpf(float a, float b, float f) {
	return (a * (1.0f - f)) + (b * f);
}

template<typename T>
static T lerpT(T a, T b, float f) {
	return (T)((float)a * (1.0f - f)) + ((float)b * f);
}

static glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float f) {
	return glm::vec3(lerpf(a.x, b.x, f),
		lerpf(a.y, b.y, f),
		lerpf(a.z, b.z, f));
}

inline float remap(float value, float low1, float high1, float low2, float high2)
{
	return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}
