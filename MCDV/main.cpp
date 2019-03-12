#include <iostream>

#include "vmf.hpp"

// OPENGL
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "GLFWUtil.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
//#include "Camera.hpp"
//#include "Mesh.hpp"
//#include "GameObject.hpp"
//#include "TextFont.hpp"
//#include "Console.hpp"
#include "FrameBuffer.hpp"

#include "interpolation.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void render_to_png(int x, int y, const char* filepath){
	void* data = malloc(4 * x * y);

	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (data != 0) {
		stbi_flip_vertically_on_write(true);
		stbi_write_png(filepath, x, y, 4, data, x * 4);
	}
}

int main(int argc, char* argv[]) {
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

	glClearColor(0.00f, 0.00f, 0.00f, 0.00f);

	std::cout << "Creating render buffers\n";

	FrameBuffer fb_tex_playspace = FrameBuffer(1024, 1024);
	FrameBuffer fb_tex_objectives = FrameBuffer(1024, 1024);
	FrameBuffer fb_comp = FrameBuffer(1024, 1024);
	FrameBuffer fb_comp_1 = FrameBuffer(1024, 1024); //Reverse ordered frame buffer

	// Screenspace quad
	std::cout << "Creating screenspace mesh\n";

	std::vector<float> __meshData = {
		-1, -1,
		-1, 1,
		1, -1,
		-1, 1,
		1, 1,
		1, -1
	};

	Mesh* mesh_screen_quad = new Mesh(__meshData, MeshMode::SCREEN_SPACE_UV);

#pragma endregion

#pragma region shader_compilation

	std::cout << "Compiling Shaders\n";

	// Internal engine shaders
	Shader shader_depth("shaders/depth.vs", "shaders/depth.fs");
	Shader shader_unlit("shaders/unlit.vs", "shaders/unlit.fs");

	// Compositing shaders
	Shader shader_comp_main("shaders/fullscreenbase.vs", "shaders/ss_test.fs"); // le big one
	Shader shader_precomp_playspace("shaders/fullscreenbase.vs", "shaders/ss_precomp_playspace.fs"); // computes distance map
	Shader shader_precomp_objectives("shaders/fullscreenbase.vs", "shaders/ss_precomp_objectives.fs"); // computes distance map

	std::cout << "Loading textures\n";

	Texture tex_background = Texture("textures/grid.png");
	Texture tex_gradient = Texture("textures/gradients/gradientmap_6.png", true);
	Texture tex_height_modulate = Texture("textures/modulate.png");

#pragma endregion

#pragma region map_load

	vmf::vmf vmf_main("sample_stuff/map_01.vmf");

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

	float x_bounds_min = -limits.NWU.x - padding; //inflate distances slightly
	float x_bounds_max = -limits.SEL.x + padding;

	float y_bounds_min = limits.SEL.z - padding;
	float y_bounds_max = limits.NWU.z + padding;

	float dist_x = x_bounds_max - x_bounds_min;
	float dist_y = y_bounds_max - y_bounds_min;

	float mx_dist = glm::max(dist_x, dist_y);

	float justify_x = (mx_dist - dist_x) * 0.5f;
	float justify_y = (mx_dist - dist_y) * 0.5f;

	float render_ortho_scale = glm::round((mx_dist / 1024.0f) / 0.01f) * 0.01f * 1024.0f; // Take largest, scale up a tiny bit. Clamp to 1024 min. Do some rounding.
	glm::vec2 view_origin = glm::vec2(x_bounds_min - justify_x, y_bounds_max + justify_y);

	std::cout << "done\n";
#pragma endregion 

#pragma endregion

#pragma region OpenGLRender

#pragma region generate_radar_txt

	kv::DataBlock node_radar = kv::DataBlock();
	node_radar.name = "de_tavr_test";
	node_radar.Values.insert({ "material", "overviews/de_tavr_test" });

	node_radar.Values.insert({ "pos_x", std::to_string(view_origin.x) });
	node_radar.Values.insert({ "pos_y", std::to_string(view_origin.y) });
	node_radar.Values.insert({ "scale", std::to_string(render_ortho_scale / 1024.0f) });

	// Try resolve spawn positions
	glm::vec3* loc_spawnCT = vmf_main.calculateSpawnLocation(vmf::team::counter_terrorist);
	glm::vec3* loc_spawnT = vmf_main.calculateSpawnLocation(vmf::team::terrorist);

	if (loc_spawnCT != NULL) {
		node_radar.Values.insert({ "CTSpawn_x", std::to_string(glm::round(remap(loc_spawnCT->x, view_origin.x, view_origin.x + render_ortho_scale, 0.0f, 1.0f) / 0.01f) * 0.01f) });
		node_radar.Values.insert({ "CTSpawn_y", std::to_string(glm::round(remap(loc_spawnCT->y, view_origin.y, view_origin.y - render_ortho_scale, 0.0f, 1.0f) / 0.01f) * 0.01f) });
	}
	if (loc_spawnT != NULL) {
		node_radar.Values.insert({ "TSpawn_x", std::to_string(glm::round(remap(loc_spawnT->x, view_origin.x, view_origin.x + render_ortho_scale, 0.0f, 1.0f) / 0.01f) * 0.01f) });
		node_radar.Values.insert({ "TSpawn_y", std::to_string(glm::round(remap(loc_spawnT->y, view_origin.y, view_origin.y - render_ortho_scale, 0.0f, 1.0f) / 0.01f) * 0.01f) });
	}

	std::ofstream out("de_tavr_test.txt");
	out << "// TAVR - AUTO RADAR. v 2.0.0\n";
	node_radar.Serialize(out);
	out.close();

#pragma endregion 

#pragma region render_playable_space
	std::cout << "Rendering playable space...";

	fb_comp.Bind(); //Bind framebuffer

	glClearColor(0.00f, 0.00f, 0.00f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);
	
	shader_depth.use();
	shader_depth.setMatrix("projection", glm::ortho(view_origin.x, view_origin.x + render_ortho_scale , view_origin.y - render_ortho_scale, view_origin.y, -1024.0f, 1024.0f));
	shader_depth.setMatrix("view", glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1.0f, 0), glm::vec3(0, 0, 1)));
		
	glm::mat4 model = glm::mat4();
	shader_depth.setMatrix("model", model);

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

	fb_comp_1.Bind();

	// Reverse rendering
	glClearDepth(0);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_GREATER);

	glClearColor(0.00f, 0.00f, 0.00f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);

	shader_depth.setFloat("HEIGHT_MIN", z_render_min);
	shader_depth.setFloat("HEIGHT_MAX", z_render_max);

	for (auto && s_solid : tavr_solids) {
		if (!s_solid->containsDisplacements)
			s_solid->mesh->Draw();
		else {
			for (auto && f : s_solid->faces) {
				if (f.displacement != NULL) {
					f.displacement->glMesh->Draw();
				}
			}
		}
	}

	// regular depth testing
	glClearDepth(1);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);

	// Apply diffusion
	fb_tex_playspace.Bind();

	glClearColor(0.00f, 0.00f, 0.00f, 0.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT, GL_FILL);

	shader_precomp_playspace.use();

	shader_precomp_playspace.setFloat("HEIGHT_MIN", z_render_min);
	shader_precomp_playspace.setFloat("HEIGHT_MAX", z_render_max);

	fb_comp.BindRTtoTexSlot(0);
	shader_precomp_playspace.setInt("tex_in", 0);

	fb_comp_1.BindRTtoTexSlot(1);
	shader_precomp_playspace.setInt("tex_in_1", 1);

	tex_height_modulate.bindOnSlot(2);
	shader_precomp_playspace.setInt("tex_modulate", 2);

	mesh_screen_quad->Draw();

	glEnable(GL_DEPTH_TEST);

	render_to_png(1024, 1024, "playable-space.png");

	std::cout << "done!\n";
#pragma endregion 

#pragma region render_objectives
	std::cout << "Rendering bombsites & buyzones space...";

	fb_comp.Bind();

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

	// Apply diffusion
	fb_tex_objectives.Bind();

	glClearColor(0.00f, 0.00f, 0.00f, 0.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT, GL_FILL);

	shader_precomp_objectives.use();

	fb_comp.BindRTtoTexSlot(0);
	shader_precomp_objectives.setInt("tex_in", 0);

	mesh_screen_quad->Draw();

	render_to_png(1024, 1024, "buyzone-bombtargets.png");

	glEnable(GL_DEPTH_TEST);
	std::cout << "done!\n";
#pragma endregion 

#pragma region compositing
	std::cout << "Compositing... \n";

	fb_comp.Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT, GL_FILL);

	shader_comp_main.use();

	tex_background.bindOnSlot(0);
	shader_comp_main.setInt("tex_background", 0);

	fb_tex_playspace.BindRTtoTexSlot(1);
	shader_comp_main.setInt("tex_playspace", 1);

	fb_tex_objectives.BindRTtoTexSlot(2);
	shader_comp_main.setInt("tex_objectives", 2);

	tex_gradient.bindOnSlot(4);
	shader_comp_main.setInt("tex_gradient", 4);

	mesh_screen_quad->Draw();

	render_to_png(1024, 1024, "1whammy.png");

	std::cout << "Done\n";

#pragma endregion 

#pragma endregion

#pragma region auto_export_game

#pragma endregion

	//Exit safely
	glfwTerminate();
	system("PAUSE");
	return 0;
}