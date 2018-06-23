#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <regex>

#include "GLFWUtil.hpp"

#include "vbsp.hpp"
#include "nav.hpp"
#include "radar.hpp"
#include "util.h"

#include "Shader.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "GameObject.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, util_keyHandler keys);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int window_width = 1024;
int window_height = 1024;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isClicking = false;

double mousex;
double mousey;

Camera camera;

int SV_WIREFRAME = 0; //0: off 1: overlay 2: only
int SV_RENDERMODE = 0;
int SV_PERSPECTIVE = 0;

float M_HEIGHT_MIN = 0.0f;
float M_HEIGHT_MAX = 10.0f;

float M_CLIP_NEAR = 0.1f;
float M_CLIP_FAR = 100.0f;

bool SV_DRAW_BSP = true;
bool SV_DRAW_NAV = false;

Radar* _radar;

float M_ORTHO_SIZE = 20.0f;

int main(int argc, char* argv[]) {
	std::string _FILE_BSP = "";
	std::string _FILE_NAV = "";

	for (int i = 1; i < argc; ++i) {
		char* _arg = argv[i];
		std::string arg = _arg;

		if (split(arg, '.').back() == "bsp") {
			_FILE_BSP = arg;
		}

		if (split(arg, '.').back() == "nav") {
			_FILE_NAV = arg;
		}
	}

	/*
	std::vector<glm::vec3> data = {
		glm::vec3(0,0,-5),
		glm::vec3(0,0,-4.5),
		glm::vec3(10,-2,0),
		glm::vec3(-3,5,0),
		glm::vec3(0,2,-9),
		glm::vec3(3,-5,0),
		glm::vec3(-2,0,4),
		glm::vec3(1,4,5),
		glm::vec3(1,4.5f, 5),
		glm::vec3(4,-5,0),
		glm::vec3(0,4,-4),
		glm::vec3(0,6,4),
		glm::vec3(0,10,0),
		glm::vec3(-2,-16,0),
		glm::vec3(0,-4,-8),
		glm::vec3(1,1,0),
		glm::vec3(3,3,-4),
		glm::vec3(0,4,0),
		glm::vec3(0,-1,0)
	};

	octree::Tree test(data, 3);

	octree::Node* tNode = test.head.getNodeByVec(glm::vec3(0, 0, -5));
	std::vector<glm::vec3> points = tNode->getContainedValues();

	for (int i = 0; i < points.size(); i++) {
		glm::vec3 p = points[i];

		std::cout << p.x << " : " << p.y << " : " << p.z << std::endl;
	}

	std::cout << "Total entries: " << test.head.getEntryCount() << std::endl;

	system("PAUSE");
	return 0;
	*/



#pragma region Initialisation

	std::cout << "Bigman Engine" << std::endl;

	//Initialize OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //We are using version 3.3 of openGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//Creating the window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "bigman engine", NULL, NULL);

	//Check if window open
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); return -1;
	}
	glfwMakeContextCurrent(window);

	//Settingn up glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl; return -1;
	}

	//Viewport
	glViewport(0, 0, 1024, 1024);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Mouse
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

#pragma endregion

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	//The unlit shader thing
	Shader shader_unlit("shaders/unlit.vs", "shaders/unlit.fs");
	Shader shader_lit("shaders/depth.vs", "shaders/depth.fs");

	//Mesh handling -----------------------------------------------------------------------------

	Mesh* t200 = NULL;
	Mesh* t201 = NULL;

	if (_FILE_BSP != ""){
		vbsp_level bsp_map(_FILE_BSP, true);
		t200 = new Mesh(bsp_map.generate_bigmesh());
	}

	if (_FILE_NAV != "") {
		Nav::Mesh bob(_FILE_NAV);
		t201 = new Mesh(bob.generateGLMesh());
	}

	//Radar rtest("de_overpass.txt");
	//_radar = &rtest;




	//VertAlphaMesh t300(vbsp_level::genVertAlpha(t200.vertices, t201.vertices));

	util_keyHandler keys(window);
	//Create camera (test)
	camera = Camera(&keys);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);


	//The main loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Input
		processInput(window, keys);
		camera.handleInput(deltaTime);

		//Rendering commands
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT, GL_FILL);



		shader_lit.use();
		if(SV_PERSPECTIVE == 0)
			shader_lit.setMatrix("projection", glm::perspective(glm::radians(90.0f / 2), (float)window_width / (float)window_height, M_CLIP_NEAR, M_CLIP_FAR));
		else
			shader_lit.setMatrix("projection", glm::ortho(-M_ORTHO_SIZE, M_ORTHO_SIZE, -M_ORTHO_SIZE, M_ORTHO_SIZE, M_CLIP_NEAR, M_CLIP_FAR));

		shader_lit.setMatrix("view", camera.getViewMatrix());

		glm::mat4 model = glm::mat4();
		shader_lit.setMatrix("model", model);

		shader_lit.setVec3("color", 0.0f, 0.0f, 1.0f);
		shader_lit.setFloat("HEIGHT_MIN", M_HEIGHT_MIN);
		shader_lit.setFloat("HEIGHT_MAX", M_HEIGHT_MAX);

		
		if (SV_RENDERMODE == 0)
			if(t200 != NULL)
				t200->Draw();
		
		if(SV_RENDERMODE == 1)
			if (t201 != NULL)
				t201->Draw();

		
			

		//t300.Draw();

		//glPolygonMode(GL_FRONT, GL_LINE);
		//
		//shader_unlit.use();
		//shader_unlit.setMatrix("projection", camera.getProjectionMatrix());
		//shader_unlit.setMatrix("view", camera.getViewMatrix());
		//shader_unlit.setMatrix("model", model);
		//shader_unlit.setVec3("color", 0.0f, 0.0f, 1.0f);
		//
		//t201.Draw();


		//Check and call events, swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Exit safely
	glfwTerminate();
	return 0;
}

//Automatically readjust to the new size we just received
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
}

bool K_CONTROL_MIN = false;
bool K_CONTROL_MAX = false;
int SV_EDITMODE = 0;

void setWindowTitle() {
	std::string title = "BigmanEngine | ";

}

void processInput(GLFWwindow* window, util_keyHandler keys)
{
	if (keys.getKeyDown(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);

	if (keys.getKeyDown(GLFW_KEY_1)) {
		SV_EDITMODE = 0; glfwSetWindowTitle(window, "Bigman Engine :: EDITING MIN");
	}
	if (keys.getKeyDown(GLFW_KEY_2)) {
		SV_EDITMODE = 1; glfwSetWindowTitle(window, "Bigman Engine :: EDITING MAX");
	}
	if (keys.getKeyDown(GLFW_KEY_3)) {
		//SV_EDITMODE = 2; glfwSetWindowTitle(window, "Bigman Engine :: de_overpass.bsp - EDITING NEAR");
	}
	if (keys.getKeyDown(GLFW_KEY_4)) {
		//SV_EDITMODE = 3; glfwSetWindowTitle(window, "Bigman Engine :: de_overpass.bsp - EDITING FAR");
	}

	if (keys.getKeyDown(GLFW_KEY_5)) {
		SV_PERSPECTIVE = 0; glfwSetWindowTitle(window, "Bigman Engine :: perspective");
	}

	if (keys.getKeyDown(GLFW_KEY_6)) {
		SV_PERSPECTIVE = 1; glfwSetWindowTitle(window, "Bigman Engine :: ortho");
	}


	if (keys.getKeyDown(GLFW_KEY_7)) {
		SV_RENDERMODE = 0; glfwSetWindowTitle(window, "Bigman Engine :: .bsp");

		glEnable(GL_CULL_FACE);
	}

	if (keys.getKeyDown(GLFW_KEY_8)) {
		SV_RENDERMODE = 1; glfwSetWindowTitle(window, "Bigman Engine :: .nav");

		glDisable(GL_CULL_FACE);
	}

	if (keys.getKeyDown(GLFW_KEY_9)) {
		SV_EDITMODE = 4;
		//M_ORTHO_SIZE = (_radar->scale / 0.1f) / 2.0f;
		//camera.cameraPos.x = (-_radar->pos_x ) * 0.01f;
		//camera.cameraPos.z = (_radar->pos_y - 1024) * 0.01f;
		glfwSetWindowTitle(window, "Bigman Engine :: EDITING ORTHO SCALE");

	}

	if (keys.getKeyDown(GLFW_KEY_0)) {
		camera.yaw = 0;
		camera.pitch = -90;
		camera.mouseUpdate(0, 0, true);
		//camera.cameraFront = glm::vec3(0, 0, -1);
		//camera.cameraUp = glm::vec3(0, 1, 0);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.mouseUpdate(xpos, ypos, isClicking);
	mousex = xpos; mousey = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//camera.fov = glm::clamp(camera.fov + (float)yoffset, 2.0f, 90.0f);

	if(SV_EDITMODE == 0)
		M_HEIGHT_MIN += (float)yoffset * 0.1f;

	if (SV_EDITMODE == 1)
		M_HEIGHT_MAX += (float)yoffset * 0.1f;

	if (SV_EDITMODE == 4)
		M_ORTHO_SIZE += (float)yoffset * 0.1f;

	//if (SV_EDITMODE == 2)	M_CLIP_NEAR += (float)yoffset * 0.1f;

	//if (SV_EDITMODE == 3)	M_CLIP_FAR += (float)yoffset * 0.1f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		isClicking = true;
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		isClicking = false;
	}
}