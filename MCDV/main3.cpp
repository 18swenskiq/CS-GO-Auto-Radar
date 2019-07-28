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

// Opengl
#include "Shader.hpp"
#include "GBuffer.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

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

static void glfw_error_callback(int error, const char* description) {
	LOG_F(ERROR, "GLFW Error %d: %s", error, description);
}

int g_renderWidth = 1024;
int g_renderHeight = 1024;

// Source sdk config
std::string g_game_path = "D:/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo";
std::string g_mapfile_path = "sample_stuff/map_01";

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


	vmf* g_vmf_file = vmf::from_file(g_mapfile_path + ".vmf");
	tar_config* g_tar_config = new tar_config(g_vmf_file); // Create config

	LOG_F(1, "Pre-processing visgroups into bit masks");
	g_vmf_file->IterSolids([g_tar_config](solid* s) {
		if (s->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_layout.c_str()))) s->m_setChannels( TAR_CHANNEL_LAYOUT_0 );
		if (s->m_editorvalues.m_hashed_visgroups.count(hash(g_tar_config->m_visgroup_overlap.c_str()))) s->m_setChannels( TAR_CHANNEL_LAYOUT_1 );
	});

	g_vmf_file->IterEntities([g_tar_config](entity* e, const std::string& classname) {
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
	ImGuiIO& io = ImGui::GetIO(); (void)io;
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
	GBuffer testBuffer = GBuffer(g_renderWidth, g_renderHeight);
	GBuffer layoutBuf2 = GBuffer(g_renderWidth, g_renderHeight);

	// Compile shaders block.
	SHADER_COMPILE_START

		GBuffer::compile_shaders();
		Shader* g_shader_test = new Shader("shaders/source/se.shaded.vs", "shaders/source/se.shaded.solid.fs", "shader.test");

	if( !SHADER_COMPILE_END ) return safe_terminate();

#pragma endregion

	g_tar_config->gen_textures();

	// Get test model.
	//studiomdl* testmdl = studiomdl::getModel("models/props/de_nuke/car_nuke.mdl", filesys);
	studiomdl* testmdl = studiomdl::getModel("models/player/zombie.mdl", filesys);

	if (testmdl == NULL) throw std::exception("Model not loadey");

	glm::vec3 pos = glm::vec3(0.0, 4224.0, -4224.0);
	glm::vec3 dir = glm::normalize(glm::vec3(0.0, -1.0, 1.0));

	// Create test camera
	glm::mat4 projm = glm::perspective(glm::radians(45.0f / 2.0f), (float)1024 / (float)1024, 32.0f, 100000.0f);
	glm::mat4 viewm = glm::lookAt(pos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
	
	glm::mat4 sourcesdk_transform = glm::mat4(1.0f);
	sourcesdk_transform = glm::rotate(sourcesdk_transform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//sourcesdk_transform = glm::scale(sourcesdk_transform, glm::vec3(0.03f));

	// Init gbuffer shader
	GBuffer::s_gbufferwriteShader->use();
	GBuffer::s_gbufferwriteShader->setMatrix("projection", projm);

	g_shader_test->use();
	g_shader_test->setMatrix("projection", projm);
	g_shader_test->setMatrix("view", viewm);

	glm::vec4 test_color = glm::vec4(0, 0, 0, 1);
	ImGradient gradient;
	bool showgrad = false;

	bool show_window_about = false;

	int gradient_selection = 0;
	int gradient_selection_last = gradient_selection;

	static ImGradientMark* draggingMark = nullptr;
	static ImGradientMark* selectedMark = nullptr;

	float color[3];
	gradient.getColorAt(0.3f, color); //position from 0 to 1

	gradient.getMarks().clear();
	gradient.addMark(0.0f, ImColor(0.2f, 0.1f, 0.0f));
	gradient.addMark(0.7f, ImColor(120, 200, 255));

	gradient.getMarks().clear();
	gradient.addMark(0.0f, ImColor(0xA0, 0x79, 0x3D));
	gradient.addMark(0.2f, ImColor(0xAA, 0x83, 0x47));
	gradient.addMark(0.3f, ImColor(0xB4, 0x8D, 0x51));
	gradient.addMark(0.4f, ImColor(0xBE, 0x97, 0x5B));
	gradient.addMark(0.6f, ImColor(0xC8, 0xA1, 0x65));
	gradient.addMark(0.7f, ImColor(0xD2, 0xAB, 0x6F));
	gradient.addMark(0.8f, ImColor(0xDC, 0xB5, 0x79));
	gradient.addMark(1.0f, ImColor(0xE6, 0xBF, 0x83));

	while (!glfwWindowShouldClose(window)) {
		viewm = glm::lookAt(glm::vec3(glm::sin(glfwGetTime()) * 4222.0f, 4222.0f, glm::cos(glfwGetTime()) * 4222.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));

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

		// Standard pass =================================================================================================================
		g_shader_test->use();
		g_shader_test->setMatrix("view", viewm);

		glClearColor(0.07, 0.07, 0.07, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		TARChannel::setChannels(TAR_CHANNEL_ALL);
		g_vmf_file->DrawWorld(g_shader_test, glm::mat4(1.0f), sourcesdk_transform, [g_shader_test](solid* ptrSolid, entity* ptrEnt) {
		});

		testBuffer.DrawPreview(glm::vec2(0,0));
		layoutBuf2.DrawPreview(glm::vec2(0, -0.5));

		glfwPollEvents();

#pragma region ImGui

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ============================= FILE MENU =====================================
		if (ImGui::BeginMainMenuBar()){
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
					show_window_about = true;
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// ============================= ABOUT TAR ======================================
		if (show_window_about) {
			ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(600, -1), ImGuiCond_Always);
			ImGui::Begin("About TAR", &show_window_about, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "Version: %s", tar_version);
			ImGui::TextWrapped("%s", tar_credits_about);
			ImGui::Separator();
			ImGui::TextWrapped("Super mega cool donators:\n%s", tar_credits_donators);
			ImGui::Separator();
			ImGui::TextWrapped("Free software used:\n%s", tar_credits_freesoft);

			if (ImGui::Button("                                       Close                                      ")) {
				show_window_about = false;
			}

			ImGui::End();
		}

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x, 19), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f));
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
				showgrad = !showgrad;
			}
			
			if (showgrad) {
				ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
				ImGui::Begin("Editing gradient: 'Heightmap Colors'", &showgrad, ImGuiWindowFlags_NoCollapse);
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
			showgrad = false;
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#pragma endregion

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		glfwSwapBuffers(window);
	}

	return safe_terminate();
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

// NVIDIA Optimus systems
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#endif