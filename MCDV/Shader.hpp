#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <map>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "loguru.hpp"

class Shader{
public:
	unsigned int programID;
	std::string symbolicName;
	bool compileUnsuccessful = false;

	//Constructor
	Shader(const std::string& pVertexShader, const std::string& pFragmentShader, const std::string& symbolicName = "defaultshader");
	~Shader();

	//Set active
	void use();

	//Util functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setUnsigned(const std::string &name, unsigned int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMatrix(const std::string &name, glm::mat4 matrix) const;
	void setVec3(const std::string &name, glm::vec3 vector) const;

	void setVec2(const std::string& name, glm::vec2 vector) const;
	void setVec3(const std::string &name, float v1, float v2, float v3) const;
	void setVec4(const std::string &name, float v1, float v2, float v3, float v4) const;
	void setVec4(const std::string &name, glm::vec4 vector) const;
	void setFragDataLocation(const std::string& name, unsigned int location) const;

	unsigned int getUniformLocation(const std::string &name) const;

	// Keep track of compiles
	inline static unsigned int compileCount = 0;
	inline static unsigned int compileSuccess = 0;

	enum compileStatus {
		SUCCESS,
		FAIL
	};

	struct compiledShader {
		unsigned int shader;
		compileStatus status;

		compiledShader(unsigned int _shader, compileStatus _status)
			: shader(_shader), status(_status) {}

		compiledShader() {}
	};

	// Shader 'library' for managing duplicate programs.
	inline static std::map<std::string, compiledShader> s_shaderlib;
	inline static std::vector<Shader*> s_shaderprogram_lib;

	static void _shader_compile_start() {
		compileCount = 0;
		compileSuccess = 0;

		LOG_F(0, "Compiling shaders");
	}

	static bool _shader_compile_end() {
		for(auto&& x: s_shaderlib){
			glDeleteShader(x.second.shader);
		}

		s_shaderlib.clear();

		if(Shader::compileCount > Shader::compileSuccess)
			LOG_F(ERROR, "%u/%u shaders compiled", Shader::compileSuccess, Shader::compileCount);
		else
			LOG_F(0, "%u/%u shaders compiled", Shader::compileSuccess, Shader::compileCount);

		return !(Shader::compileCount > Shader::compileSuccess);
	}

	static void _shaders_clear() {
		for(auto&& s: s_shaderprogram_lib)
			delete s;
	}
};

#define SHADER_COMPILE_START Shader::_shader_compile_start();
#define SHADER_COMPILE_END Shader::_shader_compile_end()
#define SHADER_CLEAR_ALL Shader::_shaders_clear();

Shader::compiledShader* LoadShader(std::string path, GLint shaderType, Shader::compileStatus* load_success);

//Constructor
Shader::Shader(const std::string& pVertexShader, const std::string& pFragmentShader, const std::string& _symbolicName)
	: symbolicName(_symbolicName)
{
	LOG_F(0, "Shader: Name( %s ), vert( %s ), frag( %s )", _symbolicName.c_str(), pVertexShader.c_str(), pFragmentShader.c_str());
	Shader::compileCount++;

	compileStatus success_vert;
	compileStatus success_frag;

	Shader::compiledShader* vertexShader = LoadShader(pVertexShader, GL_VERTEX_SHADER, &success_vert); //Load the vertex shader
	Shader::compiledShader* fragmentShader = LoadShader(pFragmentShader, GL_FRAGMENT_SHADER, &success_frag); //Load the fragment shader

	if (success_vert == compileStatus::FAIL || success_frag == compileStatus::FAIL) {
		LOG_F(ERROR, "Shader compile failed ( %s )", _symbolicName.c_str());
		this->compileUnsuccessful = true;
		return;
	}

	this->programID = glCreateProgram();

	//Attach the shaders to our program
	glAttachShader(this->programID, vertexShader->shader);
	glAttachShader(this->programID, fragmentShader->shader);
	glLinkProgram(this->programID);

	char infoLog[512];

	int success_link = 1;
	glGetProgramiv(this->programID, GL_LINK_STATUS, &success_link);
	if (!success_link) {
		glGetProgramInfoLog(this->programID, 512, NULL, infoLog);
		this->compileUnsuccessful = true;
		LOG_F(ERROR, "Link failed: %s", infoLog);
	} else {
		Shader::compileSuccess++;
	}

	// Delete the shader thingies
	Shader::s_shaderprogram_lib.push_back(this);
}

Shader::compiledShader* LoadShader(std::string path, GLint shaderType, Shader::compileStatus* load_success){
	if (Shader::s_shaderlib.count(path)) return &Shader::s_shaderlib[path];

	//Load the data into an std::string
	std::ifstream shaderFile(path);

	if (!shaderFile) {
		*load_success = Shader::compileStatus::FAIL;
		return nullptr;
	}

	std::string shaderString((std::istreambuf_iterator<char>(shaderFile)),
		std::istreambuf_iterator<char>());

	//Debug for now by printing the data its opened to the console window
	const char* shaderSource = shaderString.c_str();

	GLint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success){
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		LOG_F(ERROR, "Error Info:\n%s", infoLog);
		*load_success = Shader::compileStatus::FAIL;
	}

	// Add to cache
	Shader::s_shaderlib.insert({ path, Shader::compiledShader(shader, *load_success) });
	return &Shader::s_shaderlib[path];
}

//Destructor
Shader::~Shader(){
	glDeleteProgram(this->programID);
	LOG_F(INFO, "Delete shader ( %s )", this->symbolicName.c_str() );
}

void Shader::use(){
	glUseProgram(this->programID);
}

//Setter functions
void Shader::setBool(const std::string &name, bool value) const{
	glUniform1i(glGetUniformLocation(this->programID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const{
	glUniform1i(glGetUniformLocation(this->programID, name.c_str()), value);
}

void Shader::setUnsigned(const std::string &name, unsigned int value) const{
	glUniform1ui(glGetUniformLocation(this->programID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const{
	glUniform1f(glGetUniformLocation(this->programID, name.c_str()), value);
}

unsigned int Shader::getUniformLocation(const std::string &name) const{
	return glGetUniformLocation(this->programID, name.c_str());
}

void Shader::setMatrix(const std::string &name, glm::mat4 matrix) const{
	glUniformMatrix4fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		GL_FALSE,
		glm::value_ptr(matrix));
}

void Shader::setVec2(const std::string& name, glm::vec2 vector) const{
	glUniform2fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		glm::value_ptr(vector));
}

void Shader::setVec3(const std::string &name, glm::vec3 vector) const{
	glUniform3fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		glm::value_ptr(vector));
}

void Shader::setVec3(const std::string &name, float v1, float v2, float v3) const{
	glUniform3f(glGetUniformLocation(this->programID, name.c_str()), v1, v2, v3);
}

void Shader::setVec4(const std::string &name, float v1, float v2, float v3, float v4) const{
	glUniform4f(glGetUniformLocation(this->programID, name.c_str()), v1, v2, v3, v4);
}

void Shader::setVec4(const std::string &name, glm::vec4 vector) const{
	glUniform4fv(glGetUniformLocation(this->programID, name.c_str()),
		1,
		glm::value_ptr(vector));
}

void Shader::setFragDataLocation(const std::string& name, unsigned int location) const {
	glBindFragDataLocation(this->programID,
		location,
		name.c_str());
}