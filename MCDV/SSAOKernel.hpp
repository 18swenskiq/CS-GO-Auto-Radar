#pragma once
#include <vector>
#include <glm\glm.hpp>
#include <random>
#include "interpolation.h"
#include "Texture.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
std::default_random_engine generator;

std::vector<glm::vec3> get_ssao_samples(int numSamples = 64) {
	std::vector<glm::vec3> kernel;
	for (int i = 0; i < numSamples; ++i) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / (float)numSamples;
		scale = lerpf(0.1f, 1.0f, scale * scale);
		sample *= scale;
		kernel.push_back(sample);
	}
	return kernel;
}

class ssao_rotations_texture : public Texture {
public:
	ssao_rotations_texture(int x = 4, int y = 4) {
		std::vector<glm::vec3> noise;

		for (int i = 0; i < 65536; i++) {
			glm::vec3 s(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f
			);
			noise.push_back(s);
		}

		glGenTextures(1, &this->texture_id);
		glBindTexture(GL_TEXTURE_2D, this->texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, &noise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
};