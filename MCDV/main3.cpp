#include "globals.h"
#ifdef entry_point_revis

#include "cxxopts.hpp"

//Loguru
#include "loguru.hpp"

// Source SDK
#include "vfilesys.hpp"
#include "studiomdl.hpp"
#include "vmf.hpp"

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

int g_renderWidth = 1024;
int g_renderHeight = 1024;

// Source sdk config
std::string g_game_path = "D:/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo";
std::string g_mapfile_path = "sample_stuff/de_tavr_test";

void setupconsole();

int app(int argc, char** argv) {
#pragma region loguru
	setupconsole();

	// Create log files ( log0 for me, contains everything. txt for user )
	loguru::g_preamble_date = false;
	loguru::g_preamble_time = false;
	loguru::g_preamble_uptime = false;

	loguru::init(argc, argv);
	loguru::add_file("log.log0", loguru::FileMode::Truncate, loguru::Verbosity_MAX);
	loguru::add_file("log.txt", loguru::FileMode::Truncate, loguru::Verbosity_INFO);

	LOG_SCOPE_FUNCTION(INFO); // log main
#pragma endregion

#pragma region Source_SDK_setup

	vfilesys* filesys = new vfilesys(g_game_path + "/gameinfo.txt");
	vmf* g_vmf_file = vmf::from_file(g_mapfile_path + ".vmf");
	vmf::LinkVFileSystem(filesys);

	g_vmf_file->IterSolids([](solid* s) {
		if (s->m_editorvalues.m_hashed_visgroups.count(hash("tar_layout"))) s->m_setChannels( TAR_CHANNEL_LAYOUT );
	});

	g_vmf_file->IterEntities([](entity* e, const std::string& classname) {
		if (e->m_editorvalues.m_hashed_visgroups.count(hash("tar_layout"))) e->m_setChannels( TAR_CHANNEL_LAYOUT );
	});

	// Just draw layout
	TARChannel::setChannels(TAR_CHANNEL_LAYOUT);

#pragma endregion

#pragma region opengl_setup
	LOG_F(1, "Initializing GLFW");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	//glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(g_renderWidth, g_renderHeight, "Ceci n'est pas une window", NULL, NULL);

	LOG_F(1, "Window created");

	if (window == NULL) {
		printf("GLFW died\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	LOG_F(1, "Loading GLAD");

	// Deal with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG_F(ERROR, "Glad failed to initialize");
		return -1;
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

#pragma region Opengl_setup2

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);

	// Initialize Gbuffer functions & create one
	GBuffer::INIT();
	GBuffer* testBuffer = new GBuffer(g_renderWidth, g_renderHeight);

#pragma endregion

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

	Shader* g_shader_test = new Shader("shaders/source/se.shaded.vs", "shaders/source/se.shaded.solid.fs");
	g_shader_test->use();
	g_shader_test->setMatrix("projection", projm);
	g_shader_test->setMatrix("view", viewm);

	while (!glfwWindowShouldClose(window)) {
		viewm = glm::lookAt(glm::vec3(glm::sin(glfwGetTime()) * 4222.0f, 4222.0f, glm::cos(glfwGetTime()) * 4222.0f), glm::vec3(0.0f), glm::vec3(0, 1, 0));

		// G buffer pass =================================================================================================================
		testBuffer->Bind();
		GBuffer::s_gbufferwriteShader->use();
		GBuffer::s_gbufferwriteShader->setMatrix("view", viewm);

		glClearColor(0.00, 0.00, 0.00, 0.00);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_vmf_file->DrawWorld(GBuffer::s_gbufferwriteShader, glm::mat4(1.0f), sourcesdk_transform, [](solid* ptrSolid, entity* ptrEnt) {
			if (ptrSolid) {
				glm::vec3 orig = (ptrSolid->NWU + ptrSolid->SEL) * 0.5f;
				GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(orig.x, orig.y, orig.z));
			}
			if (ptrEnt)
				GBuffer::s_gbufferwriteShader->setVec3("srcOrigin", glm::vec3(0, 0, 0));
		});

		GBuffer::Unbind();

		// Standard pass =================================================================================================================
		g_shader_test->use();
		g_shader_test->setMatrix("view", viewm);

		glClearColor(0.07, 0.07, 0.07, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_vmf_file->DrawWorld(g_shader_test, glm::mat4(1.0f), sourcesdk_transform, [g_shader_test](solid* ptrSolid, entity* ptrEnt) {
		});

		testBuffer->DrawPreview();


		glfwPollEvents();
		glfwSwapBuffers(window);
	}
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