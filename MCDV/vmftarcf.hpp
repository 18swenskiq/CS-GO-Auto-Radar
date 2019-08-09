#pragma once

// Nodes for VMF files

#include "vmf.hpp"
#include "CompositorFrame.hpp"
#include "GBuffer.hpp"

namespace TARCF {
	Shader* s_shader_gbuf_debug;

	// Fast guassian blur implementation
	class RenderBrushesGBuffer: public BaseNode {
	public:
		RenderBrushesGBuffer() :
			BaseNode(GBuffer::s_gbufferwriteShader)
		{
			m_prop_definitions.insert({ "vmf", prop::prop_explicit<vmf*>(EXTRATYPE_RAWPTR, 0, -1) });
			m_prop_definitions.insert({ "layers", prop::prop_explicit<unsigned int>(GL_INT, TAR_CHANNEL_NONRESERVE, -1) });
			m_prop_definitions.insert({ "matrix.view", prop::prop_explicit<glm::mat4>(GL_FLOAT_MAT4, glm::mat4(1.0), -1) });
			m_prop_definitions.insert({ "matrix.proj", prop::prop_explicit<glm::mat4>(GL_FLOAT_MAT4, glm::mat4(1.0), -1) });

			m_output_definitions.push_back(Pin("GBuffer", 0));
			m_output_definitions.push_back(Pin("NBuffer", 1));
			m_output_definitions.push_back(Pin("OBuffer", 2));
		}

		void compute(NodeInstance* node) override {
			glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
			glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			TARChannel::setChannels(node->m_properties["layers"].getValue<unsigned int>());
			LOG_F(INFO, "Tarchannel: %u", node->m_properties["layers"].getValue<unsigned int>());
			GBuffer::s_gbufferwriteShader->use();

			// Set matrices
			GBuffer::s_gbufferwriteShader->setMatrix("view", node->m_properties["matrix.view"].getValue<glm::mat4>());
			GBuffer::s_gbufferwriteShader->setMatrix("projection", node->m_properties["matrix.proj"].getValue<glm::mat4>());

			// Draw vmf
			node->m_properties["vmf"].getValue<vmf*>()->DrawWorld(GBuffer::s_gbufferwriteShader, glm::mat4(1.0f), [](solid* ptrSolid, entity* ptrEnt) {
				if (ptrSolid) {
					glm::vec3 orig = (ptrSolid->m_bounds.MAX + ptrSolid->m_bounds.MIN) * 0.5f;
					GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(orig.x, orig.y, orig.z));
				}
				if (ptrEnt)
					GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(0, 0, 0));
			});

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void v_gen_buffers(NodeInstance* instance) override {
			// Front and back buffer
			glGenFramebuffers(1, &instance->m_gl_framebuffers[0]);
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]);
		}

		void v_gen_tex_memory(NodeInstance* instance) override {
			glGenTextures(4, &instance->m_gl_texture_ids[0]);

			// Position buffer float16 (48bpp)
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);

			// normal buffer float16 (48bpp)
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[1]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, instance->m_gl_texture_ids[1], 0);

			// Brush/model origin whatever
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[2]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, instance->m_gl_texture_ids[2], 0);

			// Depth texture
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[3]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, instance->m_gl_texture_ids[3], 0);

			unsigned int attachments[3] = {
				GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT1,
				GL_COLOR_ATTACHMENT2
			};

			glDrawBuffers(3, attachments);
		}

		void debug_fs(const NodeInstance* instance, int channel) override {
			s_shader_gbuf_debug->use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
			s_mesh_quad->Draw();
		}
	};

	void VMF_NODES_INIT() {
		// Create debug shader
		s_shader_gbuf_debug = new Shader("shaders/engine/quadbase.vs", "shaders/engine/node.gbuffer.preview.fs", "shader.node.gbuffer.preview");
		
		// Append to node lib
		NODELIB.insert({ "vmf.gbuffer", new RenderBrushesGBuffer() });
	}
}