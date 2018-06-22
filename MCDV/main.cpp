#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <regex>

#include "GLFWUtil.hpp"

#include "vbsp.hpp"

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

int window_width = 800;
int window_height = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool isClicking = false;

double mousex;
double mousey;

Camera camera;

int main() {

#pragma region Initialisation

	std::cout << "Harry Game Engine" << std::endl;

	//Initialize OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //We are using version 3.3 of openGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//Creating the window
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "bigman engine :: de_canals.bsp", NULL, NULL);

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
	glViewport(0, 0, 800, 600);
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
	Shader shader_unlit("unlit.vs", "unlit.fs");
	Shader shader_lit("depth.vs", "depth.fs");

	//Mesh handling -----------------------------------------------------------------------------

	vbsp_level bsp_map("de_canals.bsp", true);
	Mesh t200(bsp_map.generate_bigmesh());

	//test2.generate_mesh(0);
	
	std::cout << "GENERATED" << std::endl;

	std::vector<float> m_cube_verts = {
		// positions          // normals          rds
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	};
	Mesh cube(m_cube_verts);

	std::vector<GameObject> level_meshes = std::vector<GameObject>();

	Mesh hallo;

	for (int i = 0; i < bsp_map.texDataString.size(); i++) {
		continue;
		basic_mesh test_bsp_mesh = bsp_map.generate_mesh(i);
		std::vector<float> verts = std::vector<float>();
		for (int x = 0; x < test_bsp_mesh.vertices.size(); x++) {
			verts.push_back(test_bsp_mesh.vertices[x].x);
			verts.push_back(test_bsp_mesh.vertices[x].y);
			verts.push_back(test_bsp_mesh.vertices[x].z);
			verts.push_back(test_bsp_mesh.normals[x].x);
			verts.push_back(test_bsp_mesh.normals[x].y);
			verts.push_back(test_bsp_mesh.normals[x].z);
		}

		if (verts.size() < 3)
			continue;

		Mesh* m = new Mesh(verts);
		GameObject obj(m);

		level_meshes.push_back(obj);
	}


	std::cout << level_meshes.size() << std::endl;

	std::cout << "MESH GENERATION COMPLETE" << std::endl;



	util_keyHandler keys(window);
	//Create camera (test)
	camera = Camera(&keys);



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
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glm::vec3 sunColor = glm::vec3(2.0f, 2.0f, 2.0f);

		shader_lit.use();
		shader_lit.setVec3("directional.direction", 0.3f, -0.5f, -0.4f);
		shader_lit.setVec3("directional.ambient", sunColor * 0.2f);
		shader_lit.setVec3("directional.diffuse", sunColor * 0.4f);
		shader_lit.setVec3("viewPos", camera.cameraPos);

		shader_lit.setMatrix("projection", camera.getProjectionMatrix());
		shader_lit.setMatrix("view", camera.getViewMatrix());

		glm::mat4 model = glm::mat4();
		shader_lit.setMatrix("model", model);

		shader_lit.setVec3("color", 0.0f, 0.0f, 1.0f);

		//Do drawing logic
		for (int i = 0; i < level_meshes.size(); i++) {
			level_meshes[i].DrawMesh();
		}
		hallo.Draw();
		cube.Draw();
		t200.Draw();

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

void processInput(GLFWwindow* window, util_keyHandler keys)
{
	if (keys.getKeyDown(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.mouseUpdate(xpos, ypos, isClicking);
	mousex = xpos; mousey = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.fov = glm::clamp(camera.fov + (float)yoffset, 2.0f, 90.0f);
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