#pragma once
#include "interpolation.h"

namespace deprecated {
	/* 3 Float vector */
	struct vec3 {
		float x;
		float y;
		float z;

		vec3()
			:x(0), y(0), z(0) {}

		vec3(float x, float y, float z)
			:x(x), y(y), z(z) {}

		vec3 operator+(const vec3& o) const {
			return vec3(x + o.x, y + o.y, z + o.z);
		}

		vec3 operator*(const float& o) const {
			return vec3(x * o, y * o, z * o);
		}

		vec3 operator*(const vec3& o) const {
			return vec3(x * o.x, y * o.y, z * o.z);
		}

		vec3 operator/(const vec3& o) const {
			return vec3(x / o.x, y / o.y, z / o.z);
		}

		vec3 operator/(const float& o) const {
			return vec3(x / o, y / o, z / o);
		}

		static vec3 lerp(vec3 a, vec3 b, float f) {
			return vec3(lerpf(a.x, b.x, f),
				lerpf(a.y, b.y, f),
				lerpf(a.z, b.z, f));
		}

		static float dot(vec3 a, vec3 b) {
			float product = 0.0f;
			product += a.x * b.x;
			product += a.y * b.y;
			product += a.z * b.z;
			return product;
		}
	};

	/* 2 Float vector */
	struct vec2 {
		float x;
		float y;

		vec2()
			:x(0), y(0) {}

		vec2(float x, float y)
			:x(x), y(y) {}

		vec2 operator+(const vec2& o) const {
			return vec2(x + o.x, y + o.y);
		}

		vec2 operator*(const float& o) const {
			return vec2(x * o, y * o);
		}

		vec2 operator*(const vec2& o) const {
			return vec2(x * o.x, y * o.y);
		}

		vec2 operator/(const vec3& o) const {
			return vec2(x / o.x, y / o.y);
		}

		vec2 operator/(const float& o) const {
			return vec2(x / o, y / o);
		}

		static vec2 lerp(vec3 a, vec3 b, float f) {
			return vec2(lerpf(a.x, b.x, f),
				lerpf(a.y, b.y, f));
		}

		static float dot(vec2 a, vec2 b) {
			float product = 0.0f;
			product += a.x * b.x;
			product += a.y * b.y;
			return product;
		}
	};
}