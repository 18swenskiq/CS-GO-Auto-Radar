#include <iostream>

#include "vmf.hpp"

// OPENGL
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "GLFWUtil.hpp"

#include "Shader.hpp"
//#include "Texture.hpp"
//#include "Camera.hpp"
//#include "Mesh.hpp"
//#include "GameObject.hpp"
//#include "TextFont.hpp"
//#include "Console.hpp"
//#include "FrameBuffer.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void render_to_png(int x, int y, const char* filepath){
	void* data = malloc(3 * x * y);

	glReadPixels(0, 0, x, y, GL_RGB, GL_UNSIGNED_BYTE, data);

	if (data != 0) {
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filepath, x, y, 3, data, x * 3);
	}
}

int main(int argc, char* argv[]) {
	std::cout << "Loading VMF\n";
	vmf::vmf vmf_main("blimey.vmf");

	std::cout << "Initializing OpenGL\n";

#pragma region init_opengl

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //We are using version 3.3 of openGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE); // Window le nope
	
	//Create window
	GLFWwindow* window = glfwCreateWindow(1, 1, "If you are seeing this window, something is broken", NULL, NULL);

	//Check if window open
	if (window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); return -1;
	}
	glfwMakeContextCurrent(window);

	//Settingn up glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD" << std::endl; return -1;
	}

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 1024, 1024);

	glClearColor(0.00f, 0.00f, 0.00f, 1.0f);

#pragma endregion

#pragma region init_framebuffer

	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// generate texture
	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

	std::cout << "Generating Meshes\n";

	vmf_main.ComputeGLMeshes();
	vmf_main.ComputeDisplacements();
	std::vector<vmf::Solid*> tavr_solids = vmf_main.getAllBrushesInVisGroup("tavr_layout");

	std::vector<vmf::Solid*> tavr_buyzones = vmf_main.getAllBrushesByClassName("func_buyzone");
	std::vector<vmf::Solid*> tavr_bombtargets = vmf_main.getAllBrushesByClassName("func_bomb_target");

#pragma region bounds
	std::cout << "Calculating bounds... ";

	vmf::BoundingBox limits = vmf::getSolidListBounds(tavr_solids);
	float z_render_min = limits.SEL.y;
	float z_render_max = limits.NWU.y;

	float padding = 128.0f;

	float x_bounds_min = -limits.NWU.x -padding; //inflate distances slightly
	float x_bounds_max = -limits.SEL.x +padding;

	float y_bounds_min = limits.SEL.z -padding;
	float y_bounds_max = limits.NWU.z +padding;

	float dist_x = x_bounds_max - x_bounds_min;
	float dist_y = y_bounds_max - y_bounds_min;

	float mx_dist = glm::max(dist_x, dist_y);
	
	float justify_x = (mx_dist - dist_x) * 0.5f;
	float justify_y = (mx_dist - dist_y) * 0.5f;

	float render_ortho_scale = glm::round((mx_dist / 1024.0f) / 0.01f) * 0.01f * 1024.0f; // Take largest, scale up a tiny bit. Clamp to 1024 min. Do some rounding.
	glm::vec2 view_origin = glm::vec2(x_bounds_min - justify_x, y_bounds_max + justify_y);

	std::cout << "done\n";
#pragma endregion bounds

	std::cout << "Compiling Shaders\n";
	Shader shader_depth("shaders/depth.vs", "shaders/depth.fs");
	Shader shader_unlit("shaders/unlit.vs", "shaders/unlit.fs");

#pragma region render_playable_space
	std::cout << "Rendering playable space...";

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	shader_depth.use();
	shader_depth.setMatrix("projection", glm::ortho(view_origin.x, view_origin.x + render_ortho_scale , view_origin.y - render_ortho_scale, view_origin.y, -1024.0f, 1024.0f));
	shader_depth.setMatrix("view", glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1.0f, 0), glm::vec3(0, 0, 1)));
		
	glm::mat4 model = glm::mat4();
	shader_depth.setMatrix("model", model);

	shader_depth.setVec3("color", 1.0f, 1.0f, 1.0f);
	shader_depth.setFloat("HEIGHT_MIN", z_render_min);
	shader_depth.setFloat("HEIGHT_MAX", z_render_max);

	for (auto && s_solid : tavr_solids) {
		if(!s_solid->containsDisplacements)
			s_solid->mesh->Draw();
		else {
			for (auto && f : s_solid->faces) {
				if (f.displacement != NULL) {
					f.displacement->glMesh->Draw();
				}
			}
		}
	}

	render_to_png(1024, 1024, "playable-space.png");

	std::cout << "done!\n";
#pragma endregion render_playable_space

#pragma region render_buyzone_bombsites
	std::cout << "Rendering bombsites & buyzones space...";

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	shader_unlit.use();
	shader_unlit.setMatrix("projection", glm::ortho(view_origin.x, view_origin.x + render_ortho_scale, view_origin.y - render_ortho_scale, view_origin.y, -1024.0f, 1024.0f));
	shader_unlit.setMatrix("view", glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1.0f, 0), glm::vec3(0, 0, 1)));
	shader_unlit.setMatrix("model", model);

	shader_unlit.setVec3("color", 0.0f, 1.0f, 0.0f);

	for (auto && s_solid : tavr_buyzones) {
		s_solid->mesh->Draw();
	}

	shader_unlit.setVec3("color", 1.0f, 0.0f, 0.0f);

	for (auto && s_solid : tavr_bombtargets) {
		s_solid->mesh->Draw();
	}

	render_to_png(1024, 1024, "buyzone-bombtargets.png");

	std::cout << "done!\n";
#pragma endregion

#pragma region generate_radar_txt

	kv::DataBlock node_radar = kv::DataBlock();
	node_radar.name = "de_tavr_test";
	node_radar.Values.insert({ "material", "overviews/de_tavr_test" });

	node_radar.Values.insert({ "pos_x", std::to_string(view_origin.x) });
	node_radar.Values.insert({ "pos_y", std::to_string(view_origin.y) });
	node_radar.Values.insert({ "scale", std::to_string(render_ortho_scale / 1024.0f) });

	std::ofstream out("de_tavr_test.txt");
	out << "// TAVR - AUTO RADAR. v 2.0.0\n";
	node_radar.Serialize(out);
	out.close();
	
#pragma endregion generate_radar_txt

#pragma region compositing


#pragma endregion compositing

#pragma region auto_export_game

#pragma endregion

	system("PAUSE");
	//Exit safely
	glfwTerminate();

	return 0;
}