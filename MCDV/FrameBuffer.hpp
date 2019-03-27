#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLFWUtil.hpp"

class FrameBuffer {
public:
	unsigned int fbo;
	unsigned int rbo;
	unsigned int texColorBuffer;

	unsigned int width;
	unsigned int height;

	FrameBuffer(int width = 1024, int height = 1024, bool depthtest = true) {
		this->width = width;
		this->height = height;

		//unsigned int framebuffer;
		glGenFramebuffers(1, &this->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);

		// generate texture
		//unsigned int texColorBuffer;
		glGenTextures(1, &this->texColorBuffer);
		glBindTexture(GL_TEXTURE_2D, this->texColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// attach it to currently bound framebuffer object
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texColorBuffer, 0);

		glGenRenderbuffers(1, &this->rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		FrameBuffer::Unbind();
	}

	void BindRTtoTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->texColorBuffer);
		glActiveTexture(GL_TEXTURE0);
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo); //Set as active draw target
	}

	static void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~FrameBuffer() {
		glDeleteFramebuffers(1, &this->fbo);
	}
};