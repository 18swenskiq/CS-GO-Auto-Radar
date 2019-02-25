#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLFWUtil.hpp"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


class FrameBuffer {
public:
	unsigned int fbo;
	unsigned int rbo;
	unsigned int texture;

	FrameBuffer(bool depthtest = true) {
		glGenFramebuffers(1, &this->fbo); //Generate frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {

			glGenTextures(1, &this->texture);
			glBindTexture(GL_TEXTURE_2D, this->texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			//attach
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture, 0);

			if (depthtest)
			{
				glGenRenderbuffers(1, &this->rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
				glBindRenderbuffer(GL_RENDERBUFFER, 0);

				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);
			}

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "Framebuffer failed to generate" << std::endl;
		}

		FrameBuffer::Unbind();
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	}

	void Save() {
		void* data = malloc(3 * 1024 * 1024);
		glReadPixels(0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, data);

		if (data != 0)
			stbi_write_png("test.png", 1024, 1024, 3, data, 1024 * 3);
		else
			std::cout << "Something went wrong making render" << std::endl;
	}

	static void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~FrameBuffer() {
		glDeleteFramebuffers(1, &this->fbo);
	}
};