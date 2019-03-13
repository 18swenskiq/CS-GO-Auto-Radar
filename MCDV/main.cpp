// STDLib
#include <iostream>

// OPENGL related
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include "GLFWUtil.hpp"

// Engine header files
#include "Shader.hpp"
#include "Texture.hpp"
#include "FrameBuffer.hpp"

// Valve header files
#include "vmf.hpp"

// Util
#include "cxxopts.hpp"
#include "interpolation.h"

// Image stuff
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "dds.hpp"

/* Grabs the currently bound framebuffer and saves it to a .png */
void render_to_png(int x, int y, const char* filepath){
	void* data = malloc(4 * x * y);

	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_flip_vertically_on_write(true);
	stbi_write_png(filepath, x, y, 4, data, x * 4);

	free(data);
}

/* Grabs the currently bound framebuffer and saves it to a .dds */
void save_to_dds(int x, int y, const char* filepath) {
	void* data = malloc(4 * x * y);

	glReadPixels(0, 0, x, y, GL_RGB, GL_UNSIGNED_BYTE, data);

	dds_write((uint8_t*)data, filepath, x, y, IMG::MODE_DXT1);

	free(data);
}

/* Command line variables */
std::string m_mapfile_path;
std::string m_game_path;

//derived strings
std::string m_mapfile_name;
std::string m_overviews_folder;
std::string m_resources_folder;

bool m_outputMasks;
bool m_onlyOutputMasks;

uint32_t m_renderWidth;
uint32_t m_renderHeight;

/* Main program */
int app(int argc, const char** argv) {
	cxxopts::Options options("AutoRadar", "Auto radar");
	options.add_options()
		("v,version", "Shows the software version")
		("g,game", "(REQUIRED) Specify game path", cxxopts::value<std::string>()->default_value(""))
		("m,mapfile", "(REQUIRED) Specify the map file (vmf)", cxxopts::value<std::string>()->default_value(""))

		("d,dumpMasks", "Toggles whether auto radar should output mask images (resources/map_file.resources/)")
		("o,onlyMasks", "Specift whether auto radar should only output mask images and do nothing else (resources/map_file.resources)")

		("w,width", "Render width in pixels (experimental)", cxxopts::value<uint32_t>()->default_value("1024"))
		("h,height", "Render height in pixels (experimental)", cxxopts::value<uint32_t>()->default_value("1024"))

		("positional", "Positional parameters", cxxopts::value<std::vector<std::string>>());

	options.parse_positional("positional");
	auto result = options.parse(argc, argv);

	/* Check required parameters */
	if (result.count("game")) m_game_path = result["game"].as<std::string>();
	else throw cxxopts::option_required_exception("game");
	
	if(result.count("mapfile")) m_mapfile_path = result["mapfile"].as<std::string>();
	else if (result.count("positional")) {
		auto& positional = result["positional"].as<std::vector<std::string>>();
		
		m_mapfile_path = positional[0];
	}
	else throw cxxopts::option_required_exception("mapfile"); // We need a map file

	//Clean paths to what we can deal with
	m_mapfile_path = sutil::ReplaceAll(m_mapfile_path, "\\", "/");
	m_game_path = sutil::ReplaceAll(m_game_path, "\\", "/");

	//Derive the ones
	m_mapfile_name = split(m_mapfile_path, '/').back();
	m_overviews_folder = m_game_path + "/resource/overviews/";
	m_resources_folder = m_overviews_folder + m_mapfile_name + ".resources/";

	/* Check the rest of the flags */
	m_onlyOutputMasks = result["onlyMasks"].as<bool>();
	m_outputMasks = result["dumpMasks"].as<bool>() || m_onlyOutputMasks;

	/* Render options */
	m_renderWidth = result["width"].as<uint32_t>();
	m_renderHeight = result["height"].as<uint32_t>();

	std::cout << "Launching with options:\n";
	std::cout << "  Render width:    " << m_renderWidth << "\n";
	std::cout << "  Render height:   " << m_renderHeight << "\n";
	std::cout << "  Save masks?      " << (m_outputMasks ? "YES" : "NO") << "\n";
	std::cout << "  Output to game?  " << (!m_onlyOutputMasks ? "YES" : "NO") << "\n\n";
	std::cout << "  Game path:       " << m_game_path << "\n";
	std::cout << "  Map path:        " << m_mapfile_path << "\n";

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
	std::cout << "______________________________________________________________\n\n";

	// Internal engine shaders
	Shader shader_depth("shaders/depth.vs", "shaders/depth.fs");
	Shader shader_unlit("shaders/unlit.vs", "shaders/unlit.fs");

	// Compositing shaders
	Shader shader_comp_main("shaders/fullscreenbase.vs", "shaders/ss_comp_main.fs"); // le big one
	Shader shader_precomp_playspace("shaders/fullscreenbase.vs", "shaders/ss_precomp_playspace.fs"); // computes distance map
	Shader shader_precomp_objectives("shaders/fullscreenbase.vs", "shaders/ss_precomp_objectives.fs"); // computes distance map

	if (shader_depth.compileUnsuccessful || 
		shader_unlit.compileUnsuccessful || 
		shader_comp_main.compileUnsuccessful || 
		shader_precomp_playspace.compileUnsuccessful || 
		shader_precomp_objectives.compileUnsuccessful) {

		std::cout << "______________________________________________________________\n";
		std::cout << "Shader compilation step failed.\n";
		glfwTerminate();
		return 1;
	}

	std::cout << "______________________________________________________________\n";
	std::cout << "Shader compilation successful\n\n";

	std::cout << "Loading textures... ";

	Texture tex_background = Texture("textures/grid.png");
	Texture tex_gradient = Texture("textures/gradients/gradientmap_6.png", true);
	Texture tex_height_modulate = Texture("textures/modulate.png");

	std::cout << "done!\n\n";

#pragma endregion

#pragma region map_load

	std::cout << "Loading map file...\n";

	vmf::vmf vmf_main(m_mapfile_path + ".vmf");

	std::cout << "Generating Meshes...\n";

	vmf_main.ComputeGLMeshes();
	vmf_main.ComputeDisplacements();
	std::vector<vmf::Solid*> tavr_solids = vmf_main.getAllBrushesInVisGroup("tavr_layout");
	std::vector<vmf::Solid*> tavr_solids_funcbrush = vmf_main.getAllBrushesByClassName("func_brush");
	std::vector<vmf::Solid*> tavr_buyzones = vmf_main.getAllBrushesByClassName("func_buyzone");
	std::vector<vmf::Solid*> tavr_bombtargets = vmf_main.getAllBrushesByClassName("func_bomb_target");

	std::cout << "done!\n";

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

	std::cout << "done\n\n";
#pragma endregion 

#pragma endregion

#pragma region OpenGLRender

	std::cout << "Starting OpenGL Render\n";

#pragma region render_playable_space
	std::cout << "Rendering playable space... ";

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

	if(m_outputMasks)
		render_to_png(1024, 1024, "playable-space.png");

	std::cout << "done!\n";
#pragma endregion 

#pragma region render_objectives
	std::cout << "Rendering bombsites & buyzones space... ";

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

	if (m_outputMasks)
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

	std::cout << "done!\n";

#pragma endregion 

#pragma endregion

#pragma region auto_export_game
	if (!m_onlyOutputMasks) {
		save_to_dds(1024, 1024, std::string(m_overviews_folder + m_mapfile_name + "_radar.dds").c_str());
	}

#pragma region generate_radar_txt

	std::cout << "Generating radar .TXT... ";

	kv::DataBlock node_radar = kv::DataBlock();
	node_radar.name = m_mapfile_name;
	node_radar.Values.insert({ "material", "overviews/" + m_mapfile_name });

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

	std::ofstream out(std::string(m_overviews_folder + m_mapfile_name + ".txt").c_str());
	out << "// TAVR - AUTO RADAR. v 2.0.0\n";
	node_radar.Serialize(out);
	out.close();

	std::cout << "done!";

#pragma endregion 
#pragma endregion

	std::cout << "\n- Radar generation successful... cleaning up. -\n";

	//Exit safely
	glfwTerminate();
	return 0;
}

/* Entry point */
int main(int argc, const char** argv) {
	try {
		return app(argc, argv);
	}
	catch (cxxopts::OptionException& e) {
		std::cerr << "Parse error: " << e.what() << "\n";
	}

	return 1;
}