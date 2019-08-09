#include "globals.h"
#ifdef entry_point_revis
// Credits etc...
#include "strings.h"

// Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_color_gradient.h"
#include "imgui_color_gradient_presets.h"
#include "imgui_themes.h"

// STL
#include <stdio.h>

// CXXOpts
#include "cxxopts.hpp"

// Loguru
#include "loguru.hpp"

// Source SDK
#include "vfilesys.hpp"
#include "studiomdl.hpp"
#include "vmf.hpp"
#include "tar_config.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

// Engine
#include "Camera.hpp"
#include "CompositorFrame.hpp"
#include "vmftarcf.hpp"

// Opengl
#include "Shader.hpp"
#include "GBuffer.hpp"
#include "FrameBuffer.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// STB lib
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// OpenGL error callback.
void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	if (type == GL_DEBUG_TYPE_OTHER) return; // We dont want general openGL spam.

	LOG_F(WARNING, "--------------------------------------------------------- OPENGL ERROR ---------------------------------------------------------");
	LOG_F(WARNING, "OpenGL message: %s", message);

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		LOG_F(ERROR, "Type: ERROR"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		LOG_F(WARNING, "Type: DEPRECATED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		LOG_F(ERROR, "Type: UNDEFINED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_PORTABILITY:
		LOG_F(WARNING, "Type: PORTABILITY"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		LOG_F(WARNING, "Type: PERFORMANCE"); break;
	case GL_DEBUG_TYPE_OTHER:
		LOG_F(WARNING, "Type: OTHER"); break;
	}

	LOG_F(WARNING, "ID: %u", id);
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		LOG_F(WARNING, "Severity: LOW"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_F(WARNING, "Severity: MEDIUM"); break;
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_F(WARNING, "Severity: HIGH"); break;
	}

	LOG_F(WARNING, "--------------------------------------------------------------------------------------------------------------------------------");
}

// GLFW function declerations
static void glfw_error_callback(int error, const char* description) {
	LOG_F(ERROR, "GLFW Error %d: %s", error, description);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Source sdk config
std::string g_game_path = "D:/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo";
std::string g_mapfile_path = "sample_stuff/map_01";

// shaders
Shader* g_shader_color;
Shader* g_shader_id;

// Runtime
Camera* g_camera_main;
ImGuiIO* io;
UIBuffer* g_buff_selection;
FrameBuffer* g_buff_maskpreview;

glm::vec3 g_debug_line_orig;
glm::vec3 g_debug_line_point;

vmf* g_vmf_file;
tar_config* g_tar_config;

int display_w, display_h;

void setupconsole();

// Terminate safely
int safe_terminate() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	SHADER_CLEAR_ALL
	glfwTerminate();
	return 0;
}

uint32_t g_group_write = TAR_CHANNEL_LAYOUT_0;
uint32_t g_group_lock = TAR_CHANNEL_NONE;

Texture* tex_ui_padlock;
Texture* tex_ui_unpadlock;
Texture* tex_ui_rubbish;

void clear_channel(vmf* vmf, uint32_t channels) {
	for (auto&& i: vmf->m_solids)	{ i.m_visibility = (i.m_visibility & ~channels); if(i.m_visibility == TAR_CHANNEL_NONE) i.m_visibility |= TAR_CHANNEL_DEFAULT; }
	for (auto&& i: vmf->m_entities) { i.m_visibility = (i.m_visibility & ~channels); if(i.m_visibility == TAR_CHANNEL_NONE) i.m_visibility |= TAR_CHANNEL_DEFAULT; }
}

void ui_clear_conf(const char* id, uint32_t clearChannel) {
	if (ImGui::BeginPopupModal(id, NULL, ImGuiWindowFlags_AlwaysAutoResize)){
		ImGui::Text("This operation cannot be undone!\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); clear_channel(g_vmf_file, clearChannel); }
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void ui_sub_vgroup(const char* name, const ImVec4& color, const uint32_t& groupid) {
	ImGui::PushID((std::string(name) + "_lock").c_str());
	if (ImGui::ImageButton((void*)((g_group_lock & groupid) ? tex_ui_padlock->texture_id : tex_ui_unpadlock->texture_id), ImVec2(18, 18), ImVec2(1, 1), ImVec2(0, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
	{ g_group_lock ^= groupid; }
	ImGui::PopID();
	ImGui::SameLine();
	ImGui::PushID((std::string(name) + "_delete").c_str());
	if (ImGui::ImageButton((void*)tex_ui_rubbish->texture_id, ImVec2(18, 18), ImVec2(1, 1), ImVec2(0, 0), 0, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)))
	{ ImGui::OpenPopup(("Clear " + std::string(name) + " group?").c_str()); std::cout << "YEET\n";}
	ui_clear_conf(("Clear " + std::string(name) + " group?").c_str(), groupid);
	ImGui::PopID();
	ImGui::SameLine();

	if (TARChannel::_compFlags(&g_group_lock, groupid)) { ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f); }
	ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4(color.x * 0.5, color.y * 0.5, color.z * 0.5, color.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4(color.x * 0.8, color.y * 0.8, color.z * 0.8, color.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive,	color);
	if (ImGui::Button(name)) { g_group_write = groupid; }
	ImGui::PopStyleColor(3);
	if (TARChannel::_compFlags(&g_group_lock, groupid)) ImGui::PopStyleVar();

	
}

// UI voids
void ui_render_vgroup_edit() {
	ImGui::Begin("Editor", (bool*)0);

	ImGui::Text("Edit group:");

	ui_sub_vgroup("layout", ImVec4(0.7f, 0.8f, 0.9f, 1.0f), TAR_CHANNEL_LAYOUT_0);
	ui_sub_vgroup("overlap", ImVec4(0.3f, 0.6f, 0.9f, 1.0f), TAR_CHANNEL_LAYOUT_1);
	ui_sub_vgroup("mask", ImVec4(0.9f, 0.0f, 0.0f, 1.0f), TAR_CHANNEL_MASK);
	ui_sub_vgroup("cover", ImVec4(0.3f, 0.9f, 0.0f, 1.0f), TAR_CHANNEL_COVER);

	ImGui::Separator();

	ImGui::Text("Mask preview:");
	ImGui::Image((void*)g_buff_maskpreview->texColorBuffer, ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1,1,1,1), ImVec4(0,0,0,1));
}

// Main menu and related 'main' windows
void ui_render_main() {
	static bool s_ui_window_about = false;

	// ============================= FILE MENU =====================================
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("(dummy menu)", NULL, false, false);
			if (ImGui::MenuItem("New")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("Documentation")) {
				// Open documentation
			}
			if (ImGui::MenuItem("About TAR")) {
				s_ui_window_about = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	// ============================= ABOUT TAR ======================================
	if (s_ui_window_about) {
		ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x / 2, io->DisplaySize.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(600, -1), ImGuiCond_Always);
		ImGui::Begin("About TAR", &s_ui_window_about, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "Version: %s", tar_version);
		ImGui::TextWrapped("%s", tar_credits_about);
		ImGui::Separator();
		ImGui::TextWrapped("Super mega cool donators:\n%s", tar_credits_donators);
		ImGui::Separator();
		ImGui::TextWrapped("Free software used:\n%s", tar_credits_freesoft);

		if (ImGui::Button("                                       Close                                      ")) {
			s_ui_window_about = false;
		}

		ImGui::End();
	}
}

void vmf_render_mask_preview(vmf* v, FrameBuffer* fb, const glm::mat4& viewm, const glm::mat4& projm) {
	fb->Bind();
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static std::map<uint32_t, glm::vec3> color_lookup = {
		{ TAR_CHANNEL_LAYOUT_0, glm::vec3(0.8f, 0.8f, 0.8f) },
		{ TAR_CHANNEL_LAYOUT_1, glm::vec3(0.9f, 0.9f, 0.9f) },
		{ TAR_CHANNEL_COVER,	glm::vec3(0.5f, 0.5f, 0.5f) },
		{ TAR_CHANNEL_MASK,		glm::vec3(0.1f, 0.1f, 0.1f) }
	};

	TARChannel::setChannels( TAR_CHANNEL_LAYOUT | TAR_CHANNEL_COVER | TAR_CHANNEL_MASK );
	g_shader_color->use();
	g_shader_color->setMatrix("view", viewm);
	g_shader_color->setMatrix("projection", projm);
	g_vmf_file->DrawWorld(g_shader_color, glm::mat4(1.0f), [](solid* ptrSolid, entity* ptrEnt) {
		if(ptrSolid) g_shader_color->setVec3("color", color_lookup[ptrSolid->m_visibility] );
		if(ptrEnt) g_shader_color->setVec3("color", color_lookup[ptrEnt->m_visibility]);
	});

	FrameBuffer::Unbind();
}

ImGradient gradient;

void ui_render_dev() {
	static bool s_ui_show_gradient = false;

	static ImGradientMark* draggingMark = nullptr;
	static ImGradientMark* selectedMark = nullptr;

	ImGui::SetNextWindowPos(ImVec2(io->DisplaySize.x, 19), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
	//ImGui::Begin("Editor", (bool*)0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
	ImGui::Begin("Editor", (bool*)0);

	// ============================= LIGHTING TAB ==========================================
	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Lighting Options")) {
		if (ImGui::BeginTabBar("Lighting")) {
			if (ImGui::BeginTabItem("Occlusion"))
			{
				ImGui::Checkbox("Enable AO", &g_tar_config->m_ao_enable);
				if (g_tar_config->m_ao_enable) {
					ImGui::SliderFloat("AO Scale", &g_tar_config->m_ao_scale, 1.0f, 1500.0f, "%.1f");
					//ImGui::ColorPicker4("AO Color", glm::value_ptr(g_tar_config->m_color_ao), ImGuiColorEditFlags_NoPicker);

					ImGui::Text("AO Color: ");
					ImGui::SameLine();
					ImGui::ColorEdit4("AO Color", glm::value_ptr(g_tar_config->m_color_ao), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sun Light"))
			{
				ImGui::Checkbox("Shadows", &g_tar_config->m_shadows_enable);
				if (g_tar_config->m_shadows_enable) {
					ImGui::SliderFloat("Trace length", &g_tar_config->m_shadows_tracelength, 1.0f, 2048.0f, "%.1f");
					ImGui::SliderInt("Sample Count", &g_tar_config->m_shadows_samplecount, 1, 512, "%d");
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	// =============================== COLORS TAB ============================================
	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
	if (ImGui::CollapsingHeader("Color Options")) {
		ImGui::Text("Heightmap Colors:");

		if (ImGui::GradientButton(&gradient)) {
			s_ui_show_gradient = !s_ui_show_gradient;
		}
			
		if (s_ui_show_gradient) {
			ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
			ImGui::Begin("Editing gradient: 'Heightmap Colors'", &s_ui_show_gradient, ImGuiWindowFlags_NoCollapse);
			bool updated = ImGui::GradientEditor(&gradient, draggingMark, selectedMark);
			if (updated) {
				// Routine update gradient ... 
				g_tar_config->update_gradient(gradient);
			}

			ImGui::Separator();

			ImGui::Text("Presets:");
			if (ImGui::Button("Dust2")) { gradpreset::load_dust_2(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Mirage")) { gradpreset::load_mirage(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Overpass")) { gradpreset::load_overpass(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Cache")) { gradpreset::load_cache(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Inferno")) { gradpreset::load_inferno(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Train")) { gradpreset::load_train(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Nuke")) { gradpreset::load_nuke(&gradient); } ImGui::SameLine();
			if (ImGui::Button("Vertigo")) { gradpreset::load_vertigo(&gradient); }
			ImGui::End();
		}
	} else {
		s_ui_show_gradient = false;
	}
	ImGui::End();
}

int app(int argc, char** argv) {
#pragma region loguru
	setupconsole();

	// Create log files ( log0 for me, contains everything. txt for user )
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	loguru::g_preamble_uptime = false;
	loguru::g_preamble_thread = false;

	loguru::init(argc, argv);
	loguru::add_file("log.log0", loguru::FileMode::Truncate, loguru::Verbosity_MAX);
	loguru::add_file("log.txt", loguru::FileMode::Truncate, loguru::Verbosity_INFO);

	LOG_SCOPE_FUNCTION(INFO); // log main
#pragma endregion

#pragma region Source_SDK_setup

	vfilesys* filesys = new vfilesys(g_game_path + "/gameinfo.txt");
	vmf::LinkVFileSystem(filesys);


	g_vmf_file = vmf::from_file(g_mapfile_path + ".vmf");
	g_tar_config = new tar_config(g_vmf_file); // Create config

	LOG_F(1, "Pre-processing visgroups into bit masks");
	g_vmf_file->IterSolids([](solid* s) {
		if (s->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_layout.c_str()))) s->m_setChannels( TAR_CHANNEL_LAYOUT_0 );
		if (s->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_overlap.c_str()))) s->m_setChannels( TAR_CHANNEL_LAYOUT_1 );
	});

	g_vmf_file->IterEntities([](entity* e, const std::string& classname) {
		if (e->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_layout.c_str()))) e->m_setChannels( TAR_CHANNEL_LAYOUT_0 );
		if (e->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_overlap.c_str()))) e->m_setChannels( TAR_CHANNEL_LAYOUT_1 );
	});

#pragma endregion

#pragma region opengl_setup
	LOG_F(1, "Initializing GLFW");

	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	//glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Terri00's Auto Radar V3.0.0", NULL, NULL);
	LOG_F(1, "Window created");

	if (window == NULL) {
		printf("GLFW died\n");
		return safe_terminate();
	}

	glfwMaximizeWindow(window);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Set callbacks
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	LOG_F(1, "Loading GLAD");

	// Deal with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG_F(ERROR, "Glad failed to initialize");
		return safe_terminate();
	}

	const unsigned char* glver = glGetString(GL_VERSION);
	
	LOG_F(1, "OpenGL context: %s", glver);

	// Subscribe to error callbacks
	if (glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	} else {
		LOG_F(ERROR, "glDebugMessageCallback not availible");
	}

#pragma endregion

#pragma region Imgui_setup

	// Setup Imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Theme
	ImGui::theme_apply_psui();
	ImGui::theme_apply_eu4();

	// Setup platform / renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

#pragma endregion

#pragma region Opengl_setup2

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);

	// Initialize Gbuffer functions & create one
	GBuffer::INIT();
	GBuffer testBuffer = GBuffer(256, 256);
	GBuffer layoutBuf2 = GBuffer(256, 256);
	g_buff_selection = new UIBuffer(512, 512);
	g_buff_maskpreview = new FrameBuffer(256, 256);

	// Compile shaders block.
	SHADER_COMPILE_START

		GBuffer::compile_shaders();
		UIBuffer::compile_shaders();
		Shader* g_shader_test = new Shader("shaders/source/se.shaded.vs", "shaders/source/se.shaded.solid.fs", "shader.test");
		g_shader_color = new Shader("shaders/engine/line.vs", "shaders/engine/line.fs");
		g_shader_id = new Shader("shaders/engine/line.vs", "shaders/engine/id.fs");
		TARCF::init();
		TARCF::VMF_NODES_INIT();

	if( !SHADER_COMPILE_END ) return safe_terminate();

	// Load textures
	tex_ui_padlock = new Texture("textures/ui/lock_locked.png", false);
	tex_ui_unpadlock = new Texture("textures/ui/lock_unlocked.png", false);
	tex_ui_rubbish = new Texture("textures/ui/rubbish.png", false);

	g_camera_main = new Camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

#pragma endregion

#pragma region

	Mesh* mesh_debug_line = new Mesh({0,0,0, 0,0,-4096.0f, 0,0,0}, MeshMode::POS_XYZ);

#pragma endregion

	g_tar_config->gen_textures();
	g_vmf_file->InitOpenglData();

	// Get test model.
	//studiomdl* testmdl = studiomdl::getModel("models/props/de_nuke/car_nuke.mdl", filesys);
	studiomdl* testmdl = studiomdl::getModel("models/player/zombie.mdl", filesys);

	if (testmdl == NULL) throw std::exception("Model not loadey");

	glm::vec3 pos = glm::vec3(0.0, 4224.0, -4224.0);
	glm::vec3 dir = glm::normalize(glm::vec3(0.0, -1.0, 1.0));

	// Create test camera
	glm::mat4 projm = glm::perspective(glm::radians(45.0f / 2.0f), (float)1024 / (float)1024, 32.0f, 100000.0f);
	glm::mat4 viewm = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	
	//glm::mat4 sourcesdk_transform = glm::mat4(1.0f);
	//sourcesdk_transform = glm::rotate(sourcesdk_transform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//sourcesdk_transform = glm::scale(sourcesdk_transform, glm::vec3(0.03f));

	// Init gbuffer shader
	GBuffer::s_gbufferwriteShader->use();
	GBuffer::s_gbufferwriteShader->setMatrix("projection", projm);

	gradpreset::load_vertigo(&gradient);

	vmf_render_mask_preview(g_vmf_file, g_buff_maskpreview, glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)), glm::ortho(0, 5000, -5000, 0, -4000, 4000));

	
	TARCF::NodeInstance nodet_texture = TARCF::NodeInstance(1024, 1024, "texture");
	nodet_texture.setProperty("source", "textures/testimg_bw.png");

	//TARCF::NodeInstance nodet_invert = TARCF::NodeInstance(1024, 1024, "invert");
	TARCF::NodeInstance nodet_outline = TARCF::NodeInstance(1024, 1024, "outline");

	TARCF::NodeInstance nodet_dist = TARCF::NodeInstance(1024, 1024, "distance");
	TARCF::NodeInstance nodet_blur = TARCF::NodeInstance(1024, 1024, "guassian");
	nodet_blur.setPropertyEx("radius", 10.0f);

	// Connect nodes
	TARCF::NodeInstance::connect(&nodet_texture, &nodet_outline, 0, 0);
	TARCF::NodeInstance::connect(&nodet_outline, &nodet_dist, 0, 0);
	TARCF::NodeInstance::connect(&nodet_outline, &nodet_blur, 0, 0);

	TARCF::NodeInstance nodet_vmf = TARCF::NodeInstance(1024, 1024, "vmf.gbuffer");
	nodet_vmf.setPropertyEx<vmf*>("vmf", g_vmf_file);
	nodet_vmf.setPropertyEx<unsigned int>("layers", TAR_CHANNEL_ALL);
	nodet_vmf.setPropertyEx<glm::mat4>("matrix.view", glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)));
	nodet_vmf.setPropertyEx<glm::mat4>("matrix.proj", glm::ortho(
		g_tar_config->m_view_origin.x,										// -X
		g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale,	// +X
		g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale,	// -Y
		g_tar_config->m_view_origin.y,										// +Y
		-10000.0f,  // NEARZ
		10000.0f));

	nodet_vmf.compute();

	//nodet_blur.compute();


	float time_last = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		float deltaTime = glfwGetTime() - time_last;
		time_last = deltaTime + time_last;

		g_camera_main->handleInput(window, deltaTime);

#if 0
		// G buffer pass =================================================================================================================
		GBUFFER_WRITE_START(testBuffer, viewm)

			TARChannel::setChannels(TAR_CHANNEL_LAYOUT_0);
			g_vmf_file->DrawWorld(GBuffer::s_gbufferwriteShader, glm::mat4(1.0f), sourcesdk_transform, [](solid* ptrSolid, entity* ptrEnt) {
				if (ptrSolid) {
					glm::vec3 orig = (ptrSolid->NWU + ptrSolid->SEL) * 0.5f;
					GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(orig.x, orig.y, orig.z));
				}
				if (ptrEnt)
					GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(0, 0, 0));
			});

		GBUFFER_WRITE_END

		// G buffer pass =================================================================================================================
		GBUFFER_WRITE_START(layoutBuf2, viewm)

			TARChannel::setChannels(TAR_CHANNEL_LAYOUT_1);
			g_vmf_file->DrawWorld(GBuffer::s_gbufferwriteShader, glm::mat4(1.0f), sourcesdk_transform, [](solid* ptrSolid, entity* ptrEnt) {
			if (ptrSolid) {
				glm::vec3 orig = (ptrSolid->NWU + ptrSolid->SEL) * 0.5f;
				GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(orig.x, orig.y, orig.z));
			}
			if (ptrEnt)
				GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(0, 0, 0));
			});

			GBUFFER_WRITE_END

#endif

		GBuffer::Unbind();

		// Standard pass =================================================================================================================
		g_shader_test->use();
		g_shader_test->setMatrix("projection", g_camera_main->getProjectionMatrix(display_w, display_h));
		g_shader_test->setMatrix("view", g_camera_main->getViewMatrix());

		glClearColor(0.07, 0.07, 0.07, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static  std::map<uint32_t, glm::vec4> color_lookup = {
			{ TAR_CHANNEL_LAYOUT_0, glm::vec4(0.4f, 0.5f, 0.6f, 1.0f) },
			{ TAR_CHANNEL_LAYOUT_1, glm::vec4(0.1f, 0.3f, 0.6f, 1.0f) },
			{ TAR_CHANNEL_COVER,	glm::vec4(0.1f, 0.6f, 0.0f, 1.0f) },
			{ TAR_CHANNEL_MASK,		glm::vec4(0.6f, 0.0f, 0.0f, 1.0f) }
		};

		TARChannel::setChannels(TAR_CHANNEL_ALL);
		g_vmf_file->DrawWorld(g_shader_test, glm::mat4(1.0f), [g_shader_test](solid* ptrSolid, entity* ptrEnt) {
			g_shader_test->setVec4("color", glm::vec4(0.2, 0.2, 0.2, 0.2));
			if(ptrSolid) if(color_lookup.count(ptrSolid->m_visibility)) g_shader_test->setVec4("color", color_lookup[ptrSolid->m_visibility] );
			if(ptrEnt) if (color_lookup.count(ptrEnt->m_visibility)) g_shader_test->setVec4("color", color_lookup[ptrEnt->m_visibility]);
		});

		//testBuffer.DrawPreview(glm::vec2(0,0));
		//layoutBuf2.DrawPreview(glm::vec2(0, -0.5));

		// Debug stuff
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		g_shader_color->use();
		g_shader_color->setMatrix("projection", g_camera_main->getProjectionMatrix(display_w, display_h));
		g_shader_color->setMatrix("view", g_camera_main->getViewMatrix());
		g_shader_color->setVec3("color", glm::vec3(0, 1, 0));
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::inverse(glm::lookAt(g_debug_line_orig, g_debug_line_orig + g_debug_line_point, glm::vec3(0, 1, 0)));
		g_shader_color->setMatrix("model", model);

		mesh_debug_line->Draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//testnode.compute();
		//glViewport(0, 0, display_w, display_h);

		nodet_vmf.debug_fs();

#pragma region ImGui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ui_render_main();
		ui_render_vgroup_edit();
		ui_render_dev();

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#pragma endregion


		glfwSwapBuffers(window);
	}

	return safe_terminate();
}

void render_idmap() {
	g_shader_id->use();
	g_shader_id->setMatrix("projection", g_camera_main->getProjectionMatrix(display_w, display_h));
	g_shader_id->setMatrix("view", g_camera_main->getViewMatrix());

	g_buff_selection->Bind();
	g_buff_selection->clear();

	g_vmf_file->DrawWorld(g_shader_id, glm::mat4(1.0f), [](solid* solidPtr, entity* entPtr) {
		if (solidPtr) g_shader_id->setUnsigned("id", solidPtr->_id);
		if (entPtr) g_shader_id->setUnsigned("id", entPtr->_id);
	}, false);

	// Read pixels
	UIBuffer::Unbind();
}

// Entry point
int main(int argc, char** argv) {
	try {
		return app(argc, argv);
	}
	catch (cxxopts::OptionException& e) {
		std::cerr << "Parse error: " << e.what() << "\n";
	}
	catch (std::exception& e) {
		std::cerr << "Program error: " << e.what() << "\n";
	}

	system("PAUSE");
	return 1;
}

// Does something to the console to make it readable with loguru
#include <windows.h>
void setupconsole() {
	HWND console = GetConsoleWindow();
	MoveWindow(console, 0, 0, 1900, 900, TRUE);
}

bool g_is_clicking = false;
bool g_is_rightclick = false;
double g_mouse_x = 0;
double g_mouse_y = 0;

void selection_update(GLFWwindow* hWindow) {
	if (g_is_rightclick) {
		if (!io->WantCaptureMouse) {
			g_debug_line_orig = g_camera_main->cameraPos;
			g_debug_line_point = g_camera_main->getViewRay(g_mouse_x, g_mouse_y, display_w, display_h);

			if (g_camera_main->isDirty) {
				render_idmap(); g_camera_main->startFrame();
			}

			bool mod = glfwGetKey(hWindow, GLFW_KEY_LEFT_ALT);

			unsigned int uid = g_buff_selection->pick_normalized_pixel(g_mouse_x, display_h - g_mouse_y, display_w, display_h);
			for (auto&& i : g_vmf_file->m_solids) { if (i._id == uid) { if(i.m_visibility & ~g_group_lock) { i.m_setChannels(mod? TAR_CHANNEL_DEFAULT: g_group_write); } goto IL_FOUND; } }
			for (auto&& i : g_vmf_file->m_entities) { if (i._id == uid) { if(i.m_visibility & ~g_group_lock) { i.m_setChannels(mod? TAR_CHANNEL_DEFAULT: g_group_write); } goto IL_FOUND; } }

			//vmf_render_mask_preview(g_vmf_file, g_buff_maskpreview, glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,-1,0), glm::vec3(0,0,1)), glm::ortho(0, 5000, -5000, 0, -4000, 4000));
			
			return;

		IL_FOUND:
			vmf_render_mask_preview(g_vmf_file, g_buff_maskpreview, glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1)), glm::ortho(
				g_tar_config->m_view_origin.x,										// -X
				g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale,	// +X
				g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale,	// -Y
				g_tar_config->m_view_origin.y,										// +Y
				-10000.0f,  // NEARZ
				10000.0f));	// FARZ);
			//vmf_render_mask_preview(g_vmf_file, g_buff_maskpreview, g_tar_config., g_camera_main->getProjectionMatrix(display_w, display_h));
			return;
		}
	}
}

// GLFW callback definitions
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	if(!io->WantCaptureMouse)
	g_camera_main->mouseUpdate(xpos, ypos, g_is_clicking);

	g_mouse_x = xpos;
	g_mouse_y = ypos;

	selection_update(window);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		if (!io->WantCaptureMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			g_is_clicking = true;
		}
	}
	else{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		g_is_clicking = false;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		g_is_rightclick = true;
		selection_update(window);
	} else {
		g_is_rightclick = false;
	}
}

// NVIDIA Optimus systems
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#endif