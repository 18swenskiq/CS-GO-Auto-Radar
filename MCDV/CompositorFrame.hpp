#pragma once
#include <stdio.h>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <random>

#include "loguru.hpp"

#include "Shader.hpp"
#include "Mesh.hpp"
#include "vdf.hpp"
#include <vector>

#include <filesystem>

#include "stb_image.h"

#define EXTRATYPE_STRING 8989124
#define EXTRATYPE_RAWPTR 8989125

/* OpenGL compositor frame */

namespace TARCF {
	// Full screen quad for drawing to screen
	Mesh* s_mesh_quad;
	Shader* s_debug_shader;

	// Collection of shaders for nodes
	namespace SHADERLIB {
		std::map<std::string, Shader*> node_shaders;
	}

	// Collection of textures
	namespace TEXTURELIB {

	}

	class BaseNode;

	class Node;
	class NodeInstance;
	std::map<std::string, BaseNode*> NODELIB = {};

	// Gauruntee full vec4
	glm::vec4 parse_vec(std::string str) {
		glm::vec4 out = glm::vec4(0);

		str = sutil::removeChar(str, '(');
		str = sutil::removeChar(str, ')');

		std::vector<std::string> elems = split(str, ' ');

		for (int i = 0; i < glm::min(elems.size(), 4u); i++) {
			out[i] = ::atof(sutil::trim(elems[i]).c_str());
		}

		return out;
	}

	// Property struct
	struct prop {
		GLenum type; // type of data
		unsigned int dsize; // size of datatype
		void* value = NULL;	// Arbitrary data
		int uniformloc; // Location of the uniform in shader
		
		// Sets this properties value
		void setValue(void* src) {
			if(type != EXTRATYPE_STRING) memcpy(value, src, dsize); // copy value in
			else {
				dsize = strlen((char*)src) + 1; // recalculate size
				free( value );   // delete old string
				value = malloc( dsize ); // alloc new memory
				memcpy(value, src, dsize); // copy in new value
			}
		}

		// Explicit set
		template <typename T>
		void setValueEx(T val) { setValue(&val); }

		// Overrides for glm types (since they have value_ptr function)
		template <> void setValueEx(glm::vec2 val) { setValue(glm::value_ptr(val)); }
		template <> void setValueEx(glm::vec3 val) { setValue(glm::value_ptr(val)); }
		template <> void setValueEx(glm::vec4 val) { setValue(glm::value_ptr(val)); }
		template <> void setValueEx(glm::mat4 val) { setValue(glm::value_ptr(val)); }

		// Parse value from string (eg. from vdf)
		void setValueFromString(const std::string& strVal) {
			switch (type) {
			case GL_FLOAT: setValueEx<float>(::atof(strVal.c_str())); break;
			case GL_FLOAT_VEC2: setValueEx<glm::vec2>(parse_vec(strVal)); break;
			case GL_FLOAT_VEC3: setValueEx<glm::vec3>(parse_vec(strVal)); break;
			case GL_FLOAT_VEC4: setValueEx<glm::vec4>(parse_vec(strVal)); break;
			case GL_INT: setValueEx<int>(std::stoi(strVal.c_str())); break;
			}
		}

		// Explicit get
		template <typename T>
		inline T getValue() { return *((T*)value); }

		// Default constructor
		prop() {}

		static unsigned int dataSize(const GLenum& eDataType, void* ptrArr = 0) {
			switch (eDataType) {
			case GL_FLOAT: return sizeof(float); break;
			case GL_FLOAT_VEC2: return sizeof(float) * 2; break;
			case GL_FLOAT_VEC3: return sizeof(float) * 3; break;
			case GL_FLOAT_VEC4: return sizeof(float) * 4; break;
			case GL_INT: return sizeof(int); break;
			case EXTRATYPE_STRING: return strlen((char*)ptrArr) + 1; break;
			case GL_FLOAT_MAT4: return sizeof(glm::mat4); break;
			case EXTRATYPE_RAWPTR: return sizeof(void*); break;
			default: LOG_F(WARNING, "UNSUPPORTED UNIFORM TYPE: %u", eDataType); return 0;
			}

			return 0;
		}

		// Constructor
		prop(const GLenum& eDataType, void* src, const int& uniformLocation = 0):
			type(eDataType), uniformloc(uniformLocation)
		{
			dsize = dataSize(eDataType, src);
			value = calloc( dsize, 1 );			// alloc new storage
			if(src) memcpy(value, src, dsize);	// copy in initial value
		}

		// Explicit type constructor
		template <typename T>
		static prop prop_explicit(const GLenum& eDataType, T src, const int& uniformLocation = 0){
			prop p = prop(eDataType, &src, uniformLocation);
			return p;
		}

		template <>
		static prop prop_explicit(const GLenum& eDataType, glm::vec2 src, const int& uniformLocation) {
			prop p = prop(eDataType, glm::value_ptr(src));
			return p;
		}

		template <>
		static prop prop_explicit(const GLenum& eDataType, glm::vec3 src, const int& uniformLocation) {
			prop p = prop(eDataType, glm::value_ptr(src));
			return p;
		}

		template <>
		static prop prop_explicit(const GLenum& eDataType, glm::vec4 src, const int& uniformLocation) {
			prop p = prop(eDataType, glm::value_ptr(src));
			return p;
		}

		template <>
		static prop prop_explicit(const GLenum& eDataType, glm::mat4 src, const int& uniformLocation) {
			prop p = prop(eDataType, glm::value_ptr(src));
			return p;
		}



		~prop() {
			LOG_F(2, "dealloc()");
			free( value ); // Clean memory created from constructor
		}

		// Copy-swap assignment
		prop& operator=(prop copy) {
			std::swap(dsize, copy.dsize);
			std::swap(type, copy.type);
			std::swap(value, copy.value);
			return *this;
		}
		
		// Move constructor
		prop(prop&& other){
			dsize = other.dsize;
			type = other.type;
			value = other.value;
			
			other.value = NULL;
			other.type = 0;
			other.dsize = 0;
		}
		
		// Copy constructor
		prop(const prop& other){
			dsize = other.dsize;
			type = other.type;
			value = malloc(dsize);
			memcpy(value, other.value, dsize);
		}
	};

	// Defines an output for a node.
	struct Pin {
		std::string name; // symbolic name for this output
		int location = -1; // Location for this uniform/output

		Pin() {}
		Pin(const std::string& _name)
			: name(_name) {}
		Pin(const std::string& _name, const int& _location)
			: name(_name), location(_location) {}
	};

	// Node class. Base class for every type of node
	class BaseNode {
	public:
		// Operator shader name
		Shader* m_operator_shader;

		// Property definitions for this node, contains default values.
		std::map<std::string, prop> m_prop_definitions;

		// List of output definitions
		std::vector<Pin> m_input_definitions;
		std::vector<Pin> m_output_definitions;

		// Constructor
		BaseNode(Shader* sOpShader) :
			m_operator_shader(sOpShader){
		}

		// Some debug information about the node
		void showInfo() const {
			LOG_F(INFO, "Inputs: %u", this->m_input_definitions.size());
			for(auto&& input: this->m_input_definitions) LOG_F(INFO, "  %i: %s", input.location, input.name.c_str());

			LOG_F(INFO, "Outputs: %u", this->m_output_definitions.size());
			for(auto&& output: this->m_output_definitions) LOG_F(INFO, "  %i: %s", output.location, output.name.c_str());

			LOG_F(INFO, "Attributes: %u", this->m_prop_definitions.size());
			for (auto&& attrib: this->m_prop_definitions) LOG_F(INFO, "  %i: %s", attrib.second.uniformloc, attrib.first.c_str());
		}

		// Compute a node's outputs
		virtual void compute(NodeInstance* node);

		// Destructor
		~BaseNode() {}

		// Virtual functions
		// Generates required texture memory for a node
		// Default implmentation: Setup single RGBA texture on channel 0.
		virtual void v_gen_tex_memory(NodeInstance* instance);

		// Creates buffers
		virtual void v_gen_buffers(NodeInstance* instance);

		// Clears node
		virtual void clear(const NodeInstance* node);

		// Draws what this node is currently storing to screen
		virtual void debug_fs(const NodeInstance* instance, int channel = 0);
	};

	// Bidirection connection struct
	struct Connection {
		NodeInstance* ptrNode; // Connected node
		unsigned int uConID; // target connection ID

		Connection() {}

		Connection(NodeInstance* _ptrNode, const unsigned int& _uConID)
			: ptrNode(_ptrNode),
			uConID(_uConID) {}
	};

#define MAX_CHANNELS 16

	// This is an instance of a node which holds property values.
	class NodeInstance {
	public:
		// Keep track whether this node needs to be calculated
		bool m_isDirty = true;

		// Connections to this node
		std::vector<std::vector<Connection>> m_con_outputs;	// Output connections: this['o_pos][<i>] -> ptrNode[uConID]
		std::vector<Connection> m_con_inputs;								// Inputs: ptrNode[uConID] -> this['i_pos]

		// Internal texture storage
		unsigned int m_gl_texture_ids[MAX_CHANNELS] = { 0 };
		unsigned int m_gl_texture_w;
		unsigned int m_gl_texture_h;
		unsigned int m_gl_framebuffers[MAX_CHANNELS] = { 0 };

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
			NODELIB[m_nodeid]->v_gen_buffers(this);

			// Copy properties
			for(auto&& p: NODELIB[m_nodeid]->m_prop_definitions){
				this->m_properties.insert({ p.first, prop(p.second) });
			}

			// Copy in/outs as connections
			for(auto&& defIn: NODELIB[m_nodeid]->m_input_definitions){
				this->m_con_inputs.push_back(Connection(NULL, 0)); // initialize as connected to nothing
			}

			// Copy outputs as connections
			for (auto&& defOut: NODELIB[m_nodeid]->m_output_definitions) {
				this->m_con_outputs.push_back(std::vector<Connection>{}); // create empty output connections
			}

			// Generate texture memory for this node
			NODELIB[m_nodeid]->v_gen_tex_memory(this);
			if (!this->check_buffer()) LOG_F(ERROR, "(NODE) Framebuffer did not complete");

		}

		// Destructor deallocates texture memory and framebuffer
		~NodeInstance() {
			LOG_F(2, "Deallocating node storage ( type:%s )", m_nodeid.c_str());

			// Delete texture storage.
			for (auto&& uTex : m_gl_texture_ids)
				if (uTex) glDeleteTextures(1, &uTex);

			unsigned int id = 0;
			for(auto&& buf: m_gl_framebuffers) if(buf) glDeleteFramebuffers(1, &this->m_gl_framebuffers[id++]);
			
		}

		// Consecutively mark dirt after some kind of an update
		void markChainDirt() {
			this->m_isDirty = true;
			for (auto&& outputTree : this->m_con_outputs)
				for (auto&& output : outputTree)
					if (output.ptrNode) output.ptrNode->m_isDirty = true;
		}

		// Call respective compute function
		void compute() { 
			if (!this->m_isDirty) return;

			// Compute any dependent input nodes if they are dirty
			for(auto&& input: this->m_con_inputs)
				if (input.ptrNode) if (input.ptrNode->m_isDirty) input.ptrNode->compute();

			// Pull inputs
			unsigned int inputnum = 0;
			for(auto&& input: this->m_con_inputs){
				if (input.ptrNode) {
					glActiveTexture(GL_TEXTURE0 + inputnum++);
					glBindTexture(GL_TEXTURE_2D, input.ptrNode->m_gl_texture_ids[input.uConID]);
				}
			}

			glActiveTexture(GL_TEXTURE0);
			//LOG_F(INFO, "Computing node type: %s", this->m_nodeid.c_str());

			// Compute this node
			NODELIB[this->m_nodeid]->compute(this);

			// Mark this as done
			this->m_isDirty = false;
			
		}

		// Call respective debug_fs function
		inline void debug_fs() { NODELIB[m_nodeid]->debug_fs(this); }

		// Set a property
		void setProperty(const std::string& propname, void* ptr) {
			if (m_properties.count(propname)) m_properties[propname].setValue(ptr);
		}

		// Explit set
		template <typename T>
		void setPropertyEx(const std::string& propname, T value) {
			setProperty(propname, &value);
		}

		// Connects two nodes together
		static void connect(NodeInstance* src, NodeInstance* dst, const unsigned int& conSrcID, const unsigned int& conDstID) {
			src->m_con_outputs[conSrcID].push_back( Connection(dst, conDstID) );
			dst->m_con_inputs[conDstID] = Connection(src, conSrcID);
		}
	};

	void BaseNode::compute(NodeInstance* node) {
		glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
		glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);
		this->m_operator_shader->use();
		LOG_F(INFO, "Shaderr: %s", this->m_operator_shader->symbolicName.c_str());
		s_mesh_quad->Draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void BaseNode::v_gen_buffers(NodeInstance* instance) {
		// Generate framebuffer
		glGenFramebuffers(1, &instance->m_gl_framebuffers[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]);
	}

	// Generic tex mem handler
	void BaseNode::v_gen_tex_memory(NodeInstance* instance) {
		unsigned int id = 0;
		unsigned int* attachments = new unsigned int[this->m_output_definitions.size()];
		for (auto&& texOut: this->m_output_definitions) {
			glGenTextures(1, &instance->m_gl_texture_ids[id]);
			glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[id]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+id, GL_TEXTURE_2D, instance->m_gl_texture_ids[id], 0);
			attachments[id] = GL_COLOR_ATTACHMENT0 + id;
			id++;
		}
		glDrawBuffers(this->m_output_definitions.size(), attachments);
		delete[] attachments;
	}

	void BaseNode::clear(const NodeInstance* node) {
		glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
		glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);
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

	namespace Atomic {
		// Node that loads texture from disk
		class TextureNode: public BaseNode {
		public:
			// Constructor sets string property to path
			TextureNode(const std::string& sSource):
				BaseNode(SHADERLIB::node_shaders["passthrough"])
			{
				m_prop_definitions.insert({ "source", prop(EXTRATYPE_STRING, (void*)sSource.c_str()) });
				m_output_definitions.push_back(Pin("output", 0));
			}

			void compute(NodeInstance* node) override {
				if (node->m_gl_texture_ids[0]) glDeleteTextures(1, &node->m_gl_texture_ids[0]); // delete original texture
			
				// Load texture via gen-tex
				v_gen_tex_memory(node);
			}

			// Override texture mem generation so we can load from disk instead.
			void v_gen_tex_memory(NodeInstance* instance) override {
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]); // bind framebuffer
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

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			
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

		// Node that measures distance to nearest 'landmass'
		class Distance: public BaseNode {
		public:
			Distance() :
				BaseNode(SHADERLIB::node_shaders["distance"])
			{
				m_prop_definitions.insert({ "maxdist", prop::prop_explicit<int>(GL_INT, 255, -1) });

				m_output_definitions.push_back(Pin("output", 0));
				m_input_definitions.push_back(Pin("input", 0));
			}

			void compute(NodeInstance* node) override {
				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
				// Bind shader
				NODELIB[node->m_nodeid]->m_operator_shader->use();
				s_mesh_quad->Draw();
				for (int i = 0; i < 255; i++) {
					glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[i%2]);
					if(i > 0) glBindTexture(GL_TEXTURE_2D, node->m_gl_texture_ids[(i + 1) % 2]);
					NODELIB[node->m_nodeid]->m_operator_shader->setFloat("iter", (255.0f - (float)i) * 0.00392156862f);
					s_mesh_quad->Draw();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			void v_gen_buffers(NodeInstance* instance) override {
				// Front and back buffer
				glGenFramebuffers(2, &instance->m_gl_framebuffers[0]);
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]);
			}

			void v_gen_tex_memory(NodeInstance* instance) override {
				// BACK BUFFER
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]); // bind framebuffer
				glGenTextures(1, &instance->m_gl_texture_ids[0]);
				glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);
				
				unsigned int attachments[1] = {
					GL_COLOR_ATTACHMENT0
				};

				glDrawBuffers(1, attachments);

				// FRONT BUFFER
				
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[1]);
				glGenTextures(1, &instance->m_gl_texture_ids[1]);
				glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[1]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_gl_texture_ids[1], 0);
				
				glDrawBuffers(1, attachments);
			}
		};

		// Fast guassian blur implementation
		class GuassBlur: public BaseNode {
		public:
			GuassBlur() :
				BaseNode(SHADERLIB::node_shaders["guass_multipass"])
			{
				m_prop_definitions.insert({ "iterations", prop::prop_explicit<int>(GL_INT, 8, -1) });
				m_prop_definitions.insert({ "radius", prop::prop_explicit<float>(GL_FLOAT, 10.0f, -1) });

				m_output_definitions.push_back(Pin("output", 0));
				m_input_definitions.push_back(Pin("input", 0));
			}

			void compute(NodeInstance* node) override {
				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);

				// copy image in (we need clamped frame buffers)
				glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[1]);
				SHADERLIB::node_shaders["passthrough"]->use(); 
				s_mesh_quad->Draw();

				// Switch to blur shader
				NODELIB[node->m_nodeid]->m_operator_shader->use();

				float rad = node->m_properties["radius"].getValue<float>();
				int iterations = node->m_properties["iterations"].getValue<int>() * 2;

				// Do blur pass
				for (int i = 0; i < iterations; i++) {
					float radius = (iterations - i - 1) * (1.0f / iterations) * rad;
					glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[i%2]);
					glBindTexture(GL_TEXTURE_2D, node->m_gl_texture_ids[(i + 1) % 2]);
					NODELIB[node->m_nodeid]->m_operator_shader->setVec2("direction", (i % 2 == 0)? glm::vec2(radius, 0): glm::vec2(0, radius));
					s_mesh_quad->Draw();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			void v_gen_buffers(NodeInstance* instance) override {
				// Front and back buffer
				glGenFramebuffers(2, &instance->m_gl_framebuffers[0]);
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]);
			}

			void v_gen_tex_memory(NodeInstance* instance) override {
				// BACK BUFFER
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]); // bind framebuffer
				glGenTextures(1, &instance->m_gl_texture_ids[0]);
				glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);
				
				unsigned int attachments[1] = {
					GL_COLOR_ATTACHMENT0
				};

				glDrawBuffers(1, attachments);

				// FRONT BUFFER
				
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[1]);
				glGenTextures(1, &instance->m_gl_texture_ids[1]);
				glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[1]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, instance->m_gl_texture_ids[1], 0);
				
				glDrawBuffers(1, attachments);
			}
		};

		// Generic Ambient Occlusion
		class AmbientOcclusion: public BaseNode {
			std::vector<glm::vec3> samples;
			std::vector<glm::vec2> offsets;
			unsigned int gl_noise_texture;
		public:
			AmbientOcclusion() :
				BaseNode(SHADERLIB::node_shaders["aopass"])
			{
				m_prop_definitions.insert({ "radius", prop::prop_explicit<float>(GL_FLOAT, 256.0f, -1) });
				m_prop_definitions.insert({ "iterations", prop::prop_explicit<int>(GL_INT, 64, -1) });
				m_prop_definitions.insert({ "bias", prop::prop_explicit<float>(GL_FLOAT, 1.0f, -1) });
				m_prop_definitions.insert({ "accum_divisor", prop::prop_explicit<float>(GL_FLOAT, 15.0f, -1) });

				// For reprojection
				m_prop_definitions.insert({ "matrix.proj", prop::prop_explicit<glm::mat4>(GL_FLOAT_MAT4, glm::mat4(1.0), -1) });
				m_prop_definitions.insert({ "matrix.view", prop::prop_explicit<glm::mat4>(GL_FLOAT_MAT4, glm::mat4(1.0), -1) });

				m_output_definitions.push_back(Pin("output", 0));

				// Inputs are from gbuffer
				m_input_definitions.push_back(Pin("gposition", 0));
				m_input_definitions.push_back(Pin("gnormal", 1));

				// Generate SSAO kernel
				std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
				std::default_random_engine generator;

				for (int i = 0; i < 64; ++i) {
					glm::vec3 sample(
						randomFloats(generator) * 2.0 - 1.0,
						randomFloats(generator) * 2.0 - 1.0,
						randomFloats(generator)
					);
					sample = glm::normalize(sample);
					sample *= randomFloats(generator);
					float scale = (float)i / (float)64;
					scale = lerpf(0.1f, 1.0f, glm::pow(scale, 2.15f)); // accelerate towards center
					sample *= scale;
					samples.push_back(sample);
				}

				// Create random offsets for many many sampling innit
				for (int i = 0; i < 128; i++) {
					offsets.push_back(glm::vec2(randomFloats(generator), randomFloats(generator)));
				}

				// Generate noise texture
				std::vector<glm::vec3> noise;

				for (int i = 0; i < 65536; i++) {
					glm::vec3 s(
						randomFloats(generator) * 2.0 - 1.0,
						randomFloats(generator) * 2.0 - 1.0,
						(randomFloats(generator) * 0.5) + 0.5
					);
					noise.push_back(s);
				}

				glGenTextures(1, &gl_noise_texture);
				glBindTexture(GL_TEXTURE_2D, gl_noise_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 256, 256, 0, GL_RGB, GL_FLOAT, &noise[0]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			void compute(NodeInstance* node) override {
				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
				// Bind shader
				m_operator_shader->use();
				m_operator_shader->setFloat("blendFac", 1.0f / (float)node->m_properties["iterations"].getValue<int>());
				m_operator_shader->setFloat("bias", node->m_properties["bias"].getValue<float>());
				m_operator_shader->setFloat("ssaoScale", node->m_properties["radius"].getValue<float>());

				m_operator_shader->setMatrix("projection", node->m_properties["matrix.proj"].getValue<glm::mat4>());
				m_operator_shader->setMatrix("view", node->m_properties["matrix.view"].getValue<glm::mat4>());
				m_operator_shader->setVec2("noiseScale", glm::vec2(node->m_gl_texture_w / 256.0f, node->m_gl_texture_h / 256.0f));
				m_operator_shader->setFloat("accum_divisor", node->m_properties["accum_divisor"].getValue<float>());

				// Gen samples if not existant & upload
				for (int i = 0; i < 64; i++) m_operator_shader->setVec3("samples[" + std::to_string(i) + "]", samples[i]);

				// Bind rotation texture
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, gl_noise_texture);
				m_operator_shader->setInt("ssaoRotations", 2);

				// Dragged inputs
				m_operator_shader->setInt("gbuffer_position", 0);
				m_operator_shader->setInt("gbuffer_normal", 1);

				glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);

				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBlendEquation(GL_FUNC_ADD);

				int iter = node->m_properties["iterations"].getValue<int>();
				for (int i = 0; i < glm::min(iter, 128); i++) {
					m_operator_shader->setVec2("noiseOffset", offsets[i]);
					s_mesh_quad->Draw();
				}

				glDisable(GL_BLEND);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			void v_gen_buffers(NodeInstance* instance) override {
				// Front and back buffer
				glGenFramebuffers(1, &instance->m_gl_framebuffers[0]);
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]);
			}

			void v_gen_tex_memory(NodeInstance* instance) override {
				// BACK BUFFER
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_gl_framebuffers[0]); // bind framebuffer
				glGenTextures(1, &instance->m_gl_texture_ids[0]);
				glBindTexture(GL_TEXTURE_2D, instance->m_gl_texture_ids[0]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, instance->m_gl_texture_w, instance->m_gl_texture_h, 0, GL_RGBA, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, instance->m_gl_texture_ids[0], 0);
				
				unsigned int attachments[1] = {
					GL_COLOR_ATTACHMENT0
				};

				glDrawBuffers(1, attachments);
			}

			// Clear mem from noise texture
			~AmbientOcclusion() {
				glDeleteTextures(1, &gl_noise_texture);
			}
		};

		// Simple blend node.
		class Blend: public BaseNode{
		public:
			enum BlendMode: signed int {
				BLEND_MIX,
				BLEND_ADD,
				BLEND_SUB,
				BLEND_MUL
			};

			Blend()
				: BaseNode(SHADERLIB::node_shaders["blend.mix"])
			{
				m_input_definitions.push_back(Pin("Base", 0));
				m_input_definitions.push_back(Pin("Layer", 1));
				m_input_definitions.push_back(Pin("Mask", 2));

				m_output_definitions.push_back(Pin("output", 0));

				m_prop_definitions.insert({ "mode", prop::prop_explicit<int>(GL_INT, BlendMode::BLEND_MIX, -1) });
				m_prop_definitions.insert({ "factor", prop::prop_explicit<float>(GL_FLOAT, 1.0f, -1) });
			}

			void compute(NodeInstance* node) override {
				Shader* ptrShader = m_operator_shader;
				switch (node->m_properties["mode"].getValue<int>()) {
				case BLEND_MIX: ptrShader = SHADERLIB::node_shaders["blend.mix"]; break;
				case BLEND_ADD: ptrShader = SHADERLIB::node_shaders["blend.add"]; break;
				case BLEND_SUB: ptrShader = SHADERLIB::node_shaders["blend.sub"]; break;
				case BLEND_MUL: ptrShader = SHADERLIB::node_shaders["blend.mul"]; break;
				default: break;
				}

				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
				// Bind shader
				ptrShader->use();
				ptrShader->setInt("MainTex", 0);
				ptrShader->setInt("MainTex1", 1);
				ptrShader->setInt("Mask", 2);
				ptrShader->setFloat("factor", node->m_properties["factor"].getValue<float>());

				glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);

				s_mesh_quad->Draw();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		};

		// Simple solid color
		class Color : public BaseNode {
		public:
			Color()
				: BaseNode(NULL)
			{
				m_output_definitions.push_back(Pin("output", 0));
				m_prop_definitions.insert({ "color", prop::prop_explicit<glm::vec4>(GL_FLOAT_VEC4,glm::vec4(1.0f), -1) });
			}

			void compute(NodeInstance* node) override {
				glm::vec4 col = node->m_properties["color"].getValue<glm::vec4>();
				
				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
				glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);

				glClearColor(col.r, col.g, col.b, col.a);
				glClear(GL_COLOR_BUFFER_BIT);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		};

		// Maps a gradient to channel
		class GradientMap: public BaseNode {
		public:
			GradientMap()
				: BaseNode(SHADERLIB::node_shaders["gradient"])
			{
				m_input_definitions.push_back(Pin("Layer", 0));
				m_output_definitions.push_back(Pin("output", 0));

				m_prop_definitions.insert({ "glGradientID", prop::prop_explicit<int>(GL_INT, 0, -1) });
				m_prop_definitions.insert({ "channelID", prop::prop_explicit<int>(GL_INT, 0, -1) });
				m_prop_definitions.insert({ "min", prop::prop_explicit<float>(GL_FLOAT, 0.0f, -1) });
				m_prop_definitions.insert({ "max", prop::prop_explicit<float>(GL_FLOAT, 1.0f, -1) });
			}

			void compute(NodeInstance* node) override {
				glViewport(0, 0, node->m_gl_texture_w, node->m_gl_texture_h);
				glBindFramebuffer(GL_FRAMEBUFFER, node->m_gl_framebuffers[0]);

				m_operator_shader->use();
				m_operator_shader->setInt("channelID", node->m_properties["channelID"].getValue<int>());
				m_operator_shader->setFloat("s_minimum", node->m_properties["min"].getValue<float>());
				m_operator_shader->setFloat("s_maximum", node->m_properties["max"].getValue<float>());

				m_operator_shader->setInt("MainTex", 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, node->m_properties["glGradientID"].getValue<int>());

				m_operator_shader->setInt("Gradient", 1);

				s_mesh_quad->Draw();

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		};

		// Base class that takes an input and outputs
		class StdTransformitive: public BaseNode {
		public:
			StdTransformitive(Shader* shader)
				: BaseNode(shader)
			{
				m_input_definitions.push_back(Pin("input", 0));
				m_output_definitions.push_back(Pin("output", 0));
			}
		};

		// Inverts input on RGB
		class Invert: public StdTransformitive {
		public:
			Invert()
				: StdTransformitive(SHADERLIB::node_shaders["invert"])
			{ }
		};

		// Just moves input->output
		class Passthrough: public StdTransformitive {
		public:
			Passthrough()
				: StdTransformitive(SHADERLIB::node_shaders["passthrough"])
			{ }
		};
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

		for (const auto& entry: std::filesystem::directory_iterator("shaders/engine/tarcfnode")) {
			SHADERLIB::node_shaders.insert({ split(entry.path().filename().string(), ".fs")[0],
				new Shader("shaders/engine/quadbase.vs", entry.path().string(), entry.path().filename().string()) });
		}

		// Generative nodes (static custom handle nodes)
		NODELIB.insert({ "texture", new Atomic::TextureNode("textures/modulate.png") });
		NODELIB.insert({ "distance", new Atomic::Distance() });
		NODELIB.insert({ "guassian", new Atomic::GuassBlur() });
		NODELIB.insert({ "aopass", new Atomic::AmbientOcclusion() });
		NODELIB.insert({ "invert", new Atomic::Invert() });
		NODELIB.insert({ "passthrough", new Atomic::Passthrough() });
		NODELIB.insert({ "blend", new Atomic::Blend() });
		NODELIB.insert({ "color", new Atomic::Color() });
		NODELIB.insert({ "gradient", new Atomic::GradientMap() });
	}
}

/* Universal nodes that make up other things */

// Loads a texture from a file as a node
namespace TARCF{ 

}