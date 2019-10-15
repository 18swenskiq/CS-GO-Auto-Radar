#pragma once
#include <iostream>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "loguru.hpp"

#include "Mesh.hpp"
#include "Shader.hpp"

// Buffer for drawing Gbuffer
class GBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gOrigin;

	int width;
	int height;

	
	inline static Shader* s_previewShader;
public:
	inline static Mesh* s_previewMesh;
	inline static Shader* s_gbufferwriteShader;
	inline static Shader* s_gbufferwrite_cleanShader;

	// 14 byte/px
	// 14 megabyte @ 1024x1024
	GBuffer(const int& window_width, const int& window_height): width(window_width), height(window_height) {
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

		// Brush/model origin whatever
		glGenTextures(1, &this->gOrigin);
		glBindTexture(GL_TEXTURE_2D, this->gOrigin);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, window_width, window_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->gOrigin, 0);


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
			LOG_F(ERROR, "(GBuffer) Framebuffer did not complete");
	}

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

	void BindOriginBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gOrigin);
		glActiveTexture(GL_TEXTURE0);
	}

	// Bind this frame buffer as the draw target
	void Bind() {
		glViewport(0, 0, this->width, this->height);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer ); //Set as active draw target
	}

	// GL Clear functions
	static void clear() {
		glClearColor(0.00, 0.00, 0.00, 0.00);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Unbind frame buffer
	static void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~GBuffer() {
		glDeleteFramebuffers(1, &this->gBuffer);
	}

	// Initialize any OPENGL stuff
	static void INIT() {
		// Basic quad.
		GBuffer::s_previewMesh = new Mesh({
			0.0, 0.0, 0.0, 0.0, // bottom left
			1.0, 0.0, 1.0, 0.0, // bottom right
			1.0, 1.0, 1.0, 1.0, // top right

			0.0, 0.0, 0.0, 0.0, // bottom left
			1.0, 1.0, 1.0, 1.0, // top right
			0.0, 1.0, 0.0, 1.0  // top left
			}, MeshMode::POS_XY_TEXOORD_UV);
	}

	static void compile_shaders() {
		// Shader
		GBuffer::s_previewShader = new Shader("shaders/engine/screenbase.vs", "shaders/engine/gb.preview.fs", "shader.gbuffer.preview");
		GBuffer::s_gbufferwriteShader = new Shader("shaders/source/se.gbuffer.vs", "shaders/source/se.gbuffer.fs", "shader.gbuffer.write");
		GBuffer::s_gbufferwrite_cleanShader = new Shader("shaders/source/se.gbuffer.vs", "shaders/source/se.gbuffer.clean.fs", "shader.gbuffer.clean.write");
	}

	// Binds shader, mesh etc..
	static void _BindPreviewData() {
		GBuffer::s_previewMesh->_Bind();
		GBuffer::s_previewShader->use();
	}

	// Draws the preview mesh
	static void _DrawPreview(const glm::vec2& screenPos, const float& scale) {
		// Set model matrix
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(screenPos.x - 1.0, screenPos.y - 1.0, 0.0));
		model = glm::scale(model, glm::vec3(scale));
		GBuffer::s_previewShader->setMatrix("model", model);

		// Draw mesh
		GBuffer::s_previewMesh->_Draw();
	}

	// Draws a preview of this gBuffer on screen.
	void DrawPreview(glm::vec2 offset) {
		GBuffer::_BindPreviewData();
		glDisable(GL_DEPTH_TEST);

		// Draw previews
		GBuffer::s_previewShader->setFloat("NormalizeScale", 1.0f);
		this->BindNormalBufferToTexSlot(0);
		GBuffer::_DrawPreview(glm::vec2(0.0, 1.5) + offset, 0.5f);

		GBuffer::s_previewShader->setFloat("NormalizeScale", 0.001f);
		this->BindPositionBufferToTexSlot(0);
		GBuffer::_DrawPreview(glm::vec2(0.5, 1.5) + offset, 0.5f);

		this->BindOriginBufferToTexSlot(0);
		GBuffer::_DrawPreview(glm::vec2(1.0, 1.5) + offset, 0.5f);

		glEnable(GL_DEPTH_TEST);
	}
};

/* Simple mask buffer... */
class UIBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gUi;

public:
	int width;
	int height;

	// 14 byte/px
	// 14 megabyte @ 1024x1024
	UIBuffer(int window_width, int window_height) {
		this->width = window_width;
		this->height = window_height;
		glGenFramebuffers(1, &this->gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);

		// Map info buffer uint16 (16bpp)
		glGenTextures(1, &this->gUi);
		glBindTexture(GL_TEXTURE_2D, this->gUi);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, window_width, window_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gUi, 0);

		// Announce attachments
		unsigned int attachments[1] = {
			GL_COLOR_ATTACHMENT0
		};

		glDrawBuffers(1, attachments);

		// Create and test render buffer
		glGenRenderbuffers(1, &this->rBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, this->rBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, window_width, window_height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG_F(ERROR, "(GBuffer) Framebuffer did not complete");
	}

	void Bind() {
		glViewport(0, 0, this->width, this->height);
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer); //Set as active draw target
	}

	void BindBufBufferToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gUi);
		glActiveTexture(GL_TEXTURE0);
	}

	static void Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Revert to default framebuffer
	}

	~UIBuffer() {
		glDeleteFramebuffers(1, &this->gBuffer);
	}

	const inline static unsigned int clearc = 0;

	// GL Clear functions
	void clear() {
		glClearBufferuiv(GL_COLOR, 0, &UIBuffer::clearc);
		glClear( GL_DEPTH_BUFFER_BIT);
	}

	uint32_t pick_normalized_pixel(const double& mouse_x, const double& mouse_y, const int& window_w, const int& window_h) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer); //Set as active draw target
		float norm_x = (float)mouse_x / (float)window_w; // get normalized coords
		float norm_y = (float)mouse_y / (float)window_h;

		int px = this->width * norm_x;
		int py = this->height * norm_y;

		uint32_t id = 0;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glReadPixels(px, py, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &id);
		UIBuffer::Unbind();
		return id;
	}

	inline static Shader* s_previewShader;

	static void compile_shaders() {
		// Shader
		UIBuffer::s_previewShader = new Shader("shaders/engine/screenbase.vs", "shaders/engine/idpreview.fs", "shader.uibuffer.preview");
	}

	// Binds shader, mesh etc..
	static void _BindPreviewData() {
		GBuffer::s_previewMesh->_Bind();
		UIBuffer::s_previewShader->use();
	}

	// Draws the preview mesh
	static void _DrawPreview(const glm::vec2& screenPos, const float& scale) {
		// Set model matrix
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(screenPos.x, screenPos.y, 0.0));
		model = glm::scale(model, glm::vec3(scale));
		UIBuffer::s_previewShader->setMatrix("model", model);
		
		// Draw mesh
		GBuffer::s_previewMesh->_Draw();
	}
};

/* Basic frame buffer for compositing */
class FBuffer {
	unsigned int gBuffer;
	unsigned int rBuffer;

	unsigned int gColor;
	unsigned int hData;
	unsigned int aoBuffer;

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

		// Color buffer
		glGenTextures(1, &this->gColor);
		glBindTexture(GL_TEXTURE_2D, this->gColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gColor, 0);

		// Position buffer float16
		glGenTextures(1, &this->hData);
		glBindTexture(GL_TEXTURE_2D, this->hData);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->hData, 0);

		// Position buffer float16
		glGenTextures(1, &this->aoBuffer);
		glBindTexture(GL_TEXTURE_2D, this->aoBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, window_width, window_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->aoBuffer, 0);

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

	void BindRTToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->gColor);
		glActiveTexture(GL_TEXTURE0);
	}

	void BindHeightToTexSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->hData);
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

// Macro that will attach frame buffer, clear, set shader uniforms
#define GBUFFER_WRITE_START(buffer, viewm) buffer.Bind();\
GBuffer::clear();\
GBuffer::s_gbufferwriteShader->use();\
GBuffer::s_gbufferwriteShader->setMatrix("view", viewm);

// Macro that defines the end of a 'frame buffer write'
#define GBUFFER_WRITE_END GBuffer::Unbind();