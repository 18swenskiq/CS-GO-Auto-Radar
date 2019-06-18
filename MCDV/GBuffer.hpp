#pragma once
#include <iostream>

#include <glad\glad.h>
#include <GLFW\glfw3.h>


class GBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gMapInfo;

	unsigned int gMask;

	int width;
	int height;

public:
	// 14 byte/px
	// 14 megabyte @ 1024x1024
	GBuffer(int window_width, int window_height) {
		this->width = window_width;
		this->height = window_height;
		glGenFramebuffers(1, &this->gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

		// Position buffer float16 (48bpp)
		glGenTextures(1, &this->gPosition);
		glBindTexture(GL_TEXTURE_2D, this->gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window_width, window_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gPosition, 0);

		// normal buffer float16 (48bpp)
		glGenTextures(1, &this->gNormal);
		glBindTexture(GL_TEXTURE_2D, this->gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window_width, window_height, 0, GL_RGB, GL_FLOAT, NULL);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->gNormal, 0);

		// Map info buffer uint16 (16bpp)
		glGenTextures(1, &this->gMapInfo);
		glBindTexture(GL_TEXTURE_2D, this->gMapInfo);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, window_width, window_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->gMapInfo, 0);

		// Announce attachments
		unsigned int attachments[3] = { 
			GL_COLOR_ATTACHMENT0, 
			GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2
		};

		glDrawBuffers(3, attachments);

		// Create and test render buffer
		glGenRenderbuffers(1, &this->rBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	/*
	void BindRTtoTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gBuffer);
		glActiveTexture(GL_TEXTURE0);
	}*/

	void BindPositionBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gPosition);
		glActiveTexture(GL_TEXTURE0);
	}

	void BindNormalBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gNormal);
		glActiveTexture(GL_TEXTURE0);
	}

	void BindInfoBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gMapInfo);
		glActiveTexture(GL_TEXTURE0);
	}

	void Bind() {
		glViewport(0, 0, this->width, this->height);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer ); //Set as active draw target
	}

	static void Unbind() {
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer

	}

	~GBuffer() {
		glDeleteFramebuffers(1, &this->gBuffer);
	}
};

/* Simple mask buffer... */
class MBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gMask;

	int width;
	int height;

public:
	// 14 byte/px
	// 14 megabyte @ 1024x1024
	MBuffer(int window_width, int window_height) {
		this->width = window_width;
		this->height = window_height;
		glGenFramebuffers(1, &this->gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

		// Map info buffer uint16 (16bpp)
		glGenTextures(1, &this->gMask);
		glBindTexture(GL_TEXTURE_2D, this->gMask);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, window_width, window_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gMask, 0);

		// Announce attachments
		unsigned int attachments[3] = {
			GL_COLOR_ATTACHMENT0
		};

		glDrawBuffers(1, attachments);

		// Create and test render buffer
		glGenRenderbuffers(1, &this->rBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	void BindMaskBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gMask);
		glActiveTexture(GL_TEXTURE0);
	}

	void Bind() {
		glViewport(0, 0, this->width, this->height);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer); //Set as active draw target
	}

	static void Unbind() {
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~MBuffer() {
		glDeleteFramebuffers(1, &this->gBuffer);
	}
};

/* Basic frame buffer for compositing */
class FBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gColor;

	int width;
	int height;

public:
	// 14 byte/px
	// 14 megabyte @ 1024x1024
	FBuffer(int window_width, int window_height) {
		this->width = window_width;
		this->height = window_height;
		glGenFramebuffers(1, &this->gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

		// Position buffer float16 (48bpp)
		glGenTextures(1, &this->gColor);
		glBindTexture(GL_TEXTURE_2D, this->gColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gColor, 0);

		// Announce attachments
		unsigned int attachments[1] = {
			GL_COLOR_ATTACHMENT0
		};

		glDrawBuffers(1, attachments);

		// Create and test render buffer
		glGenRenderbuffers(1, &this->rBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	/*
	void BindRTtoTexSlot(int slot = 0) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, this->gBuffer);
	glActiveTexture(GL_TEXTURE0);
	}*/

	void BindRTToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gColor);
		glActiveTexture(GL_TEXTURE0);
	}

	void Bind() {
		glViewport(0, 0, this->width, this->height);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer); //Set as active draw target
	}

	static void Unbind() {
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~FBuffer() {
		glDeleteFramebuffers(1, &this->gBuffer);
	}
};