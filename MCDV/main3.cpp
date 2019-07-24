#include "globals.h"
#ifdef entry_point_revis

#include "cxxopts.hpp"

//Loguru
#include "loguru.hpp"

// Source SDK
#include "vfilesys.hpp"
#include "studiomdl.hpp"
#include "vmf_new.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

// Opengl
#include "Shader.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

void opengl_dbgMsg_callback(GLenum src, GLenum type, GLenum id, GLenum severity, GLsizei length, const GLchar* msg, void* userParam);

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

#pragma endregion

#pragma region opengl_setup
	LOG_F(1, "Initializing GLFW");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
		printf("GLAD died\n");
		return -1;
	}

	const unsigned char* glver = glGetString(GL_VERSION);
	
	LOG_F(1, "OpenGL context: ", glver);

	

#pragma endregion

#pragma region Opengl_setup2

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);

	Shader* g_shader_test = new Shader("shaders/source/basic.vs", "shaders/source/basic.fs");

#pragma endregion

	// Get test model.
	//studiomdl* testmdl = studiomdl::getModel("models/props/de_nuke/car_nuke.mdl", filesys);
	studiomdl* testmdl = studiomdl::getModel("models/player/zombie.mdl", filesys);

	if (testmdl == NULL) throw std::exception("Model not loadey");

	glm::vec3 pos = glm::vec3(0.0, 4224.0, -4224.0);
	glm::vec3 dir = glm::normalize(glm::vec3(0.0, -1.0, 1.0));

	// Create test camera
	glm::mat4 projm = glm::perspective(glm::radians(45.0f / 2.0f), (float)1024 / (float)1024, 32.0f, 100000.0f);
	glm::mat4 viewm = glm::lookAt(pos, pos+dir, glm::vec3(0, 1, 0));
	
	glm::mat4 sourcesdk_transform = glm::mat4(1.0f);
	sourcesdk_transform = glm::rotate(sourcesdk_transform, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	//sourcesdk_transform = glm::scale(sourcesdk_transform, glm::vec3(0.03f));

	g_shader_test->use();
	g_shader_test->setMatrix("projection", projm);
	g_shader_test->setMatrix("view", viewm);

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.07, 0.07, 0.07, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(glm::sin(glfwGetTime()), 0, glm::cos(glfwGetTime())) * 43.0f);
		model = glm::rotate(model, (float)glfwGetTime() * 4.0f, glm::vec3(0, 1, 0));
		
		model *= sourcesdk_transform;
		g_shader_test->setMatrix("model", model);

		testmdl->Bind();
		testmdl->Draw();

		g_vmf_file->DrawWorld(g_shader_test, glm::mat4(1.0f), sourcesdk_transform);

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

void opengl_dbgMsg_callback(GLenum src, GLenum type, GLenum id, GLenum severity, GLsizei length, const GLchar* msg, void* userParam) {
	LOG_F(WARNING, "OpenGL message: %s", msg);
}

#endif