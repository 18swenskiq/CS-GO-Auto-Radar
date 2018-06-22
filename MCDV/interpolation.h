#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//Linear interpolation between two floats, given time
static float lerpf(float a, float b, float f) {
	return (a * (1.0f - f)) + (b * f);
}

static glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float f) {
	return glm::vec3(lerpf(a.x, b.x, f),
		lerpf(a.y, b.y, f),
		lerpf(a.z, b.z, f));
}
