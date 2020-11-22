#pragma once

#include "buildmode.h"

#include "vmf_new.hpp"

#ifdef GLBUILD
#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#endif

#include <iostream>
#include <vector>
#include <sstream>

#ifdef GLBUILD
#include "GBuffer.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "GradientMap.hpp"
#include "SSAOKernel.hpp"
#endif

#include "tar_config.hpp"
#include "dds.hpp"

#include "cxxopts.hpp"

#include "dxrendering.h"
#include "DXShaderCombo.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

#define TAR_MAX_LAYERS 5
#define TAR_AO_SAMPLES 256

// figure out why this makes it work
#undef main

int app(int argc, const char** argv);
void render_to_png(int x, int y, const char* filepath);
void save_to_dds(int x, int y, const char* filepath, IMG imgmode = IMG::MODE_DXT1);
#ifdef GLBUILD
void render_config(tar_config_layer layer, const std::string& layerName, FBuffer* drawTarget = NULL);
#endif