#pragma once
#include <string>
#include <fstream>
#include <iostream>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

bool USE_DEBUG = false;

//Prototype functions
unsigned int LoadShader(std::string path, GLint shaderType);

class Shader
{
public:
	unsigned int programID;

	//Constructor
	Shader(std::string vertexPath, std::string fragmentPath);
	Shader(std::string shaderName);
	~Shader();

	//Set active
	void use();

	//Util functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMatrix(const std::string &name, glm::mat4 matrix) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;

	void setVec3(const std::string &name, float v1, float v2, float v3) const;

	unsigned int getUniformLocation(const std::string &name) const;
};



//Constructor
Shader::Shader(std::string pVertexShader, std::string pFragmentShader)
{
	unsigned int vertexShader = LoadShader(pVertexShader, GL_VERTEX_SHADER); //Load the vertex shader
	unsigned int fragmentShader = LoadShader(pFragmentShader, GL_FRAGMENT_SHADER); //Load the fragment shader

	this->programID = glCreateProgram();

	//Attach the shaders to our program
	glAttachShader(this->programID, vertexShader);
	glAttachShader(this->programID, fragmentShader);
	glLinkProgram(this->programID);

	int success;
	char infoLog[512];

	glGetProgramiv(this->programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->programID, 512, NULL, infoLog);
		std::cout << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::Shader(std::string name)
{
	unsigned int vertexShader = LoadShader(name + ".hvert", GL_VERTEX_SHADER); //Load the vertex shader
	unsigned int fragmentShader = LoadShader(name + ".hfrag", GL_FRAGMENT_SHADER); //Load the fragment shader

	this->programID = glCreateProgram();

	//Attach the shaders to our program
	glAttachShader(this->programID, vertexShader);
	glAttachShader(this->programID, fragmentShader);
	glLinkProgram(this->programID);

	int success;
	char infoLog[512];

	glGetProgramiv(this->programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->programID, 512, NULL, infoLog);
		std::cout << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

unsigned int LoadShader(std::string path, GLint shaderType)
{
	//Load the data into an std::string
	std::ifstream shaderFile(path);
	std::string shaderString((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	//Debug for now by printing the data its opened to the console window
	if (USE_DEBUG)
		std::cout << "Compiling shader from source: " << path << std::endl << shaderString.c_str() << std::endl << std::endl;
	else
		std::cout << "Compiling shader from source: " << path << std::endl;

	const char* shaderSource = shaderString.c_str();

	GLint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED" << std::endl << infoLog << std::endl;
	}

	return shader;
}

//Destructor
Shader::~Shader()
{
	//Delete this shader
	glDeleteProgram(this->programID);
}

void Shader::use()
{
	glUseProgram(this->programID);
}

//Setter functions
void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(this->programID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(this->programID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(this->programID, name.c_str()), value);
}

unsigned int Shader::getUniformLocation(const std::string &name) const
{
	return glGetUniformLocation(this->programID, name.c_str());
}

void Shader::setMatrix(const std::string &name, glm::mat4 matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		GL_FALSE,
		glm::value_ptr(matrix));
}

void Shader::setVec3(const std::string &name, glm::vec3 vector) const
{
	glUniform3fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		glm::value_ptr(vector));
}

void Shader::setVec3(const std::string &name, float v1, float v2, float v3) const
{
	glUniform3f(glGetUniformLocation(this->programID, name.c_str()), v1, v2, v3);
}