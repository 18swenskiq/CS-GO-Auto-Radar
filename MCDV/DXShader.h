#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "buildmode.h"

#include <d3d11.h>

bool USE_DEBUG = false;

// Prototype functions
unsigned int LoadShader(std::string path, int shaderType, int* load_success);

class DXShader
{
public:
	unsigned int programID;

	bool compileUnsuccessful = false;

	// Constructor
	DXShader(std::string vertexPath, std::string fragmentPath);
	~Shader();
};