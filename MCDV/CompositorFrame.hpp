#pragma once
#include <stdio.h>
#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "loguru.hpp"

#include "Shader.hpp"
#include "Mesh.hpp"
#include "vdf.hpp"
#include <vector>

#include <filesystem>

#include "stb_image.h"

#define EXTRATYPE_STRING 8989124

/* OpenGL compositor frame */

namespace TARCF {
	// Full screen quad for drawing to screen
	Mesh* s_mesh_quad;
	Shader* s_debug_shader;

	// Collection of shaders for nodes
	namespace SHADERLIB {
		Shader* passthrough;

		std::map<std::string, Shader*> node_shaders;
	}

	// Collection of textures
	namespace TEXTURELIB {

	}

	class BaseNode;

	class Node;
	class NodeInstance;
	std::map<std::string, BaseNode*> NODELIB = {};

	// Property struct
	struct prop {
		GLenum type; // type of data
		unsigned int dsize; // size of datatype
		void* value = NULL;	// Arbitrary data
		
		// Sets this properties value
		void setValue(void* src) {
			if(type != EXTRATYPE_STRING) memcpy(value, src, dsize); // copy value in
			else {
				dsize = strlen((char*)src) + 1; // recalculate size
				free( value );   // delete old string
				malloc( dsize ); // alloc new memory
				memcpy(value, src, dsize); // copy in new value
			}
		}

		// Default constructor
		prop() {}

		// Constructor
		prop(GLenum eDataType, void* src):
			type(eDataType)
		{
			switch (eDataType) {
			case GL_FLOAT: dsize = sizeof(float); break;
			case GL_FLOAT_VEC2: dsize = sizeof(float) * 2; break;
			case GL_FLOAT_VEC3: dsize = sizeof(float) * 3; break;
			case GL_FLOAT_VEC4: dsize = sizeof(float) * 4; break;
			case EXTRATYPE_STRING: dsize = strlen((char*)src)+1; break;
			default: LOG_F(ERROR, "UNSUPPORTED UNIFORM TYPE"); throw std::exception("UNSUPPORTED UNIFORM TYPE"); return;
			}

			LOG_F(2, "	Storage size: %u", dsize);

			value = calloc( dsize, 1 );			// alloc new storage
			if(src) memcpy(value, src, dsize);	// copy in initial value
			if (eDataType == EXTRATYPE_STRING) LOG_F(3, "strv: %s", value);
		}

		~prop() {
			LOG_F(2, "dealloc()");
			//free( value ); // Clean memory created from constructor
		}
	};

	// Node class. Base class for every type of node
	class BaseNode {
	public:
		// Operator shader name
		Shader* m_operator_shader;

		// Property definitions for this node, contains default values.
		std::map<std::string, prop> m_prop_definitions;

		// Constructor
		BaseNode(Shader* sOpShader) :
			m_operator_shader(sOpShader)
		{
			//LOG_F(2, "Creating node from shader ( %s )", sOpShader->symbolicName.c_str());

			int count;
			int size;
			GLenum type;
			const int buf_size = 32;
			char buf_name[buf_size];
			int name_length;

			// Extract uniforms from shader
			glGetProgramiv(sOpShader->programID, GL_ACTIVE_UNIFORMS, &count);

			// Get all uniforms
			for (int i = 0; i < count; i++) {
				glGetActiveUniform(sOpShader->programID, i, buf_size, &name_length, &size, &type, buf_name);
				LOG_F(2, "    Property ( %s ), type( %u )", buf_name, type);
				m_prop_definitions.insert({ std::string(buf_name), prop(type, NULL) }); // write to definitions
			}
		}

		// Compute a node's outputs
		virtual void compute(NodeInstance* node);

		// Destructor
		~BaseNode() {}

		// Virtual functions
		// Generates required texture memory for a node
		// Default implmentation: Setup single RGBA texture on channel 0.
		virtual void v_gen_tex_memory(NodeInstance* instance);

		// Clears node
		virtual void clear(const NodeInstance* node);

		// Draws what this node is currently storing to screen
		virtual void debug_fs(const NodeInstance* instance, int channel = 0);
	};

	// Standard node.
	class Node: public BaseNode {
	public:
		Node(Shader* sOpShader):
			BaseNode(sOpShader)
		{
			
		}
	};

	// Bidirection connection struct
	struct Connection {
		Node* ptrNode; // Connected node
		unsigned int uConID; // target connection ID
	};

#define MAX_CHANNELS 4

	// This is an instance of a node which holds property values.
	class NodeInstance {
	public:
		std::vector<Connection> m_con_outputs[MAX_CHANNELS];	// Output connections: this['o_pos][<i>] -> ptrNode[uConID]
		std::vector<Connection> m_con_inputs;					// Inputs: ptrNode[uConID] -> this['i_pos]

		// Internal texture storage
		unsigned int m_gl_texture_ids[MAX_CHANNELS] = { 0, 0, 0, 0 };
		unsigned int m_gl_texture_w;
		unsigned int m_gl_texture_h;
		unsigned int m_gl_framebuffer;

		// Associated master node
		std::string m_nodeid;

		// Local instance properties
		std::map<std::string, prop> m_properties;

		// Check for framebuffer completeness
		inline bool check_buffer() {
			return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		}

		// Constructor creates framebuffer and texture memory, sets up properties
		NodeInstance(const unsigned int& iWidth, const unsigned int& iHeight, const std::string& sNodeId) :
			m_gl_texture_w(iWidth), m_gl_texture_h(iHeight), m_nodeid(sNodeId)
		{
			// Generate framebuffer
			glGenFramebuffers(1, &this->m_gl_framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, this->m_gl_framebuffer);

			// Copy properties
			for(auto&& p: NODELIB[m_nodeid]->m_prop_definitions){
				this->m_properties.insert({ p.first, prop(p.second) });
			}

			// Generate texture memory for this node
			NODELIB[m_nodeid]->v_gen_tex_memory(this);
			if (!this->check_buffer()) LOG_F(ERROR, "(NODE) Framebuffer did not complete");
		}

		// Destructor deallocates texture memory and framebuffer
		~NodeInstance() {
			LOG_F(2, "Deallocating node storage ( type:%s )", m_nodeid);

			// Delete texture storage.
			for (auto&& uTex : m_gl_texture_ids)
				if (uTex) glDeleteTextures(1, &uTex);

			glDeleteFramebuffers(1, &this->m_gl_framebuffer);
		}

		// Call respective compute function
		inline void compute() { NODELIB[m_nodeid]->compute(this); }

		// Call respective debug_fs function
		inline void debug_fs() { NODELIB[m_nodeid]->debug_fs(this); }

		// Set a property
		void setProperty(const std::string& propname, void* ptr) {
			if (m_properties.count(propname)) m_properties[propname].setValue(ptr);
		}
	};

	// Loads a texture from a file as a node
	class TextureNode: public BaseNode {
	public:
		// Constructor sets string property to path
		TextureNode(const std::string& sSource):
			BaseNode(SHADERLIB::passthrough)
		{
			m_prop_definitions.insert({ "source", prop(EXTRATYPE_STRING, (void*)sSource.c_str()) });
		}

		void compute(NodeInstance* node) override {
			if (node->m_gl_texture_ids[0]) glDeleteTextures(1, &node->m_gl_texture_ids[0]); // delete original texture
			
			// Load texture via gen-tex
			v_gen_tex_memory(node);
		}

		// Override texture mem generation so we can load from disk instead.
		void v_gen_tex_memory(NodeInstance* instance) override {
			glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffer); // bind framebuffer
			glGenTextures(1, &instance->m_gl_texture_ids[0]);
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);

			// Load image via stb
			int width, height, nrChannels;
			stbi_set_flip_vertically_on_load(true);

			LOG_F(3, "Opening image: %s", (char*)instance->m_properties["source"].value);

			unsigned char* data = stbi_load((char*)instance->m_properties["source"].value, &width, &height, &nrChannels, 4);

			if (data) {
				instance->m_gl_texture_w = width;
				instance->m_gl_texture_h = height;
			} 

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			
			free( data );

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);

			unsigned int attachments[1] = {
				GL_COLOR_ATTACHMENT0
			};

			glDrawBuffers(1, attachments);
		}
	};

	void BaseNode::compute(NodeInstance* node) {
		glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
		glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffer);
		this->m_operator_shader->use();
		s_mesh_quad->Draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void BaseNode::v_gen_tex_memory(NodeInstance* instance) {
		glGenTextures(1, &instance->m_gl_texture_ids[0]);
		glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);

		unsigned int attachments[1] = {
			GL_COLOR_ATTACHMENT0
		};

		glDrawBuffers(1, attachments);
	}

	void BaseNode::clear(const NodeInstance* node) {
		glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
		glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffer);
		glClearColor(0.0, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void BaseNode::debug_fs(const NodeInstance* instance, int channel) {
		s_debug_shader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[channel]);
		s_mesh_quad->Draw();
	}

	// Init system
	void init() {
		s_mesh_quad = new Mesh({
			-1.0, -1.0, 0.0, 0.0, // bottom left
			1.0, -1.0, 1.0, 0.0, // bottom right
			1.0, 1.0, 1.0, 1.0, // top right

			-1.0, -1.0, 0.0, 0.0, // bottom left
			1.0, 1.0, 1.0, 1.0, // top right
			-1.0, 1.0, 0.0, 1.0  // top left
			}, MeshMode::POS_XY_TEXOORD_UV);

		s_debug_shader = new Shader("shaders/engine/quadbase.vs", "shaders/engine/node.preview.fs", "shader.node.preview");

		SHADERLIB::passthrough = new Shader("shaders/engine/quadbase.vs", "shaders/engine/tarcfnode/passthrough.fs", "tarcfn.passthrough");

		// Generative nodes (static)
		NODELIB.insert({ "texture", new TextureNode("textures/modulate.png") });

		// Load generic transformative nodes
		for(const auto& entry: std::filesystem::directory_iterator("tarcfnode")){
			std::ifstream ifs(entry.path().c_str());
			if (!ifs) throw std::exception("Node info read error");

			std::string file_str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			kv::FileData file_kv(file_str);

			kv::DataBlock* block_info = file_kv.headNode->GetFirstByName("info");
			kv::DataBlock* block_shader = file_kv.headNode->GetFirstByName("shader");

			if (!block_info) { LOG_F(ERROR, "No info block in node: %s", entry.path().filename().c_str()); continue; };
			if (!block_shader) { LOG_F(ERROR, "No shader block in node: %s", entry.path().filename().c_str()); continue; };

			// Create node & shader
			NODELIB.insert(
				{
				kv::tryGetStringValue(block_info->Values, "name", "none"),
				new Node(
					new Shader(
						kv::tryGetStringValue(block_info->Values, "vertex", "shaders/engine/quadbase.vs"),
						kv::tryGetStringValue(block_info->Values, "fragment", "shaders/engine/tarcfnode/passthrough.fs"),
						"tarcfn::" + kv::tryGetStringValue(block_info->Values, "name", "none") 
					) 
				)
				}
			);
		}
	}
}