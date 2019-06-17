#include "globals.h"
#include "vmf_new.hpp"

#ifdef entry_point_testing

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>
#include <vector>

#include "GBuffer.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "GradientMap.hpp"
#include "SSAOKernel.hpp"
#include "tar_config.hpp"

std::string m_game_path = "D:/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo";

void render_config(tar_config_layer layer);

glm::mat4 g_mat4_viewm;
glm::mat4 g_mat4_projm;

Shader* g_shader_gBuffer;
Shader* g_shader_iBuffer;
Shader* g_shader_comp;

GBuffer* g_gbuffer;
GBuffer* g_gbuffer_clean;
MBuffer* g_mask_playspace;
MBuffer* g_mask_buyzone;
MBuffer* g_mask_objectives;

vmf* g_vmf_file;
tar_config* g_tar_config;
BoundingBox g_vmf_info_bounds;

Mesh* g_mesh_screen_quad;
Texture* g_texture_background;
Texture* g_texture_modulate;
std::vector<glm::vec3> g_ssao_samples;
Texture* g_ssao_rotations;

int main(){
#pragma region opengl_setup
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(1024, 1024, "Ceci n'est pas une window", NULL, NULL);

	if (window == NULL) {
		printf("GLFW died\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Deal with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("GLAD died\n");
		return -1;
	}

	const unsigned char* glver = glGetString(GL_VERSION);
	printf("(required: min core 3.3.0) opengl version: %s\n", glver);
#pragma endregion

	vfilesys* filesys = new vfilesys(m_game_path + "/gameinfo.txt");

	vmf::LinkVFileSystem(filesys);
	g_vmf_file = vmf::from_file("sample_stuff/de_tavr_test.vmf");
	g_vmf_file->InitModelDict();
	g_tar_config = new tar_config(g_vmf_file);
	g_vmf_info_bounds = g_vmf_file->getVisgroupBounds(g_tar_config->m_visgroup_layout);

#pragma region opengl_extra

	std::vector<float> __meshData = {
		-1, -1,
		-1, 1,
		1, -1,
		-1, 1,
		1, 1,
		1, -1
	};
	g_mesh_screen_quad = new Mesh(__meshData, MeshMode::SCREEN_SPACE_UV);

	// Set up shaders
	g_shader_gBuffer = new Shader("shaders/gBuffer.vs", "shaders/gBuffer.fs");
	g_shader_iBuffer = new Shader("shaders/gBuffer.vs", "shaders/iBuffer.fs");
	g_shader_comp = new Shader("shaders/fullscreenbase.vs", "shaders/fullscreenbase.fs");

	// Set up draw buffers
	g_mask_playspace =	new MBuffer(1024, 1024);
	g_mask_objectives = new MBuffer(1024, 1024);
	g_mask_buyzone =	new MBuffer(1024, 1024);
	g_gbuffer =			new GBuffer(1024, 1024);
	g_gbuffer_clean =   new GBuffer(1024, 1024);

	// Setup camera projection matrices
	g_mat4_projm = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, -1024.0f, 1024.0f);
	g_mat4_viewm = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0, 0, 1));

	// Load textures
	g_texture_background = g_tar_config->m_texture_background;//new Texture("textures/grid.png");
	g_texture_modulate = new Texture("textures/modulate.png");
	g_ssao_samples = get_ssao_samples(256);
	g_ssao_rotations = new ssao_rotations_texture();

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);

	std::vector<entity*> configs = g_vmf_file->get_entities_by_classname("tar_config");

#pragma endregion

	while (!glfwWindowShouldClose(window)) {
		Sleep(1000);

		render_config(tar_config_layer());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	IL_EXIT:
	glfwTerminate();
	return 0;
}

#endif

void render_config(tar_config_layer layer) {
	// G BUFFER GENERATION ======================================================================================
#pragma region buffer_gen_geo

	g_gbuffer->Bind();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	g_shader_gBuffer->use();
	g_shader_gBuffer->setMatrix("projection", g_mat4_projm);
	g_shader_gBuffer->setMatrix("view", g_mat4_viewm);

	glm::mat4 model = glm::mat4();
	g_shader_gBuffer->setMatrix("model", model);

	// Draw everything
	g_vmf_file->SetFilters({}, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer);
	g_vmf_file->DrawEntities(g_shader_gBuffer);

	// Clear depth
	glClear(GL_DEPTH_BUFFER_BIT);

	// Render again BUT JUST THE IMPORTANT BITS
	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_layout, g_tar_config->m_visgroup_mask }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer);
	g_vmf_file->DrawEntities(g_shader_gBuffer);

	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_overlap }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer, {}, TAR_MIBUFFER_OVERLAP);
	g_vmf_file->DrawEntities(g_shader_gBuffer, {}, TAR_MIBUFFER_OVERLAP);

	// Draw cover with cover flag set
	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_cover }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer, {}, TAR_MIBUFFER_COVER0);
	g_vmf_file->DrawEntities(g_shader_gBuffer, {}, TAR_MIBUFFER_COVER0);

	GBuffer::Unbind();

	g_gbuffer_clean->Bind();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_layout, g_tar_config->m_visgroup_mask }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer);
	g_vmf_file->DrawEntities(g_shader_gBuffer);

	GBuffer::Unbind();

#pragma endregion

#pragma region mask_gen

	g_mask_playspace->Bind();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shader_iBuffer->use();
	g_shader_iBuffer->setMatrix("projection", g_mat4_projm);
	g_shader_iBuffer->setMatrix("view", g_mat4_viewm);

	// LAYOUT ================================================================

	g_shader_iBuffer->setUnsigned("srcChr", 0x1U);
	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_layout }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_iBuffer);
	g_vmf_file->DrawEntities(g_shader_iBuffer);

	// Subtractive brushes
	g_shader_iBuffer->setUnsigned("srcChr", 0x0U);
	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_mask }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_iBuffer);
	g_vmf_file->DrawEntities(g_shader_iBuffer);

	// OBJECTIVES ============================================================

	g_mask_objectives->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shader_iBuffer->setUnsigned("srcChr", 0x1U);
	g_vmf_file->SetFilters({}, { "func_bomb_target", "func_hostage_rescue" });
	g_vmf_file->DrawEntities(g_shader_iBuffer);

	// BUY ZONES =============================================================

	g_mask_buyzone->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shader_iBuffer->setUnsigned("srcChr", 0x1U);
	g_vmf_file->SetFilters({}, { "func_buyzone" });
	g_vmf_file->DrawEntities(g_shader_iBuffer);

#pragma endregion

	// FINAL COMPOSITE ===============================================================
#pragma region final_composite

	MBuffer::Unbind(); // Release any frame buffer

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);

	g_shader_comp->use();

	// Bind RT's
	//g_texture_background->bindOnSlot		  ( 0 );
	//g_shader_comp->setInt("tex_background",		0 );

	g_tar_config->m_texture_gradient->bindOnSlot( 1 );
	g_shader_comp->setInt("tex_gradient",		1 );

	g_gbuffer->BindPositionBufferToTexSlot    (	2 );
	g_shader_comp->setInt("gbuffer_position",	2 );

	g_gbuffer->BindNormalBufferToTexSlot      (	3 );
	g_shader_comp->setInt("gbuffer_normal",  	3 );

	g_gbuffer->BindInfoBufferToTexSlot		  ( 4 );
	g_shader_comp->setInt("gbuffer_info",       4 );

	g_mask_playspace->BindMaskBufferToTexSlot (	5 );
	g_shader_comp->setInt("umask_playspace",	5 );

	g_mask_objectives->BindMaskBufferToTexSlot( 6 );
	g_shader_comp->setInt("umask_objectives",	6 );

	g_mask_buyzone->BindMaskBufferToTexSlot   (	7 );
	g_shader_comp->setInt("umask_buyzone",		7 );

	g_gbuffer_clean->BindPositionBufferToTexSlot(8);
	g_shader_comp->setInt("gbuffer_clean_position", 8);

	g_texture_modulate->bindOnSlot(10);
	g_shader_comp->setInt("tex_modulate", 10);

	g_ssao_rotations->bindOnSlot              ( 9 );
	g_shader_comp->setInt("ssaoRotations",      9 );
	g_shader_comp->setFloat("ssaoScale", 1000.0f);
	g_shader_comp->setMatrix("projection", g_mat4_projm);
	g_shader_comp->setMatrix("view", g_mat4_viewm);

	for (int i = 0; i < 256; i++) {
		g_shader_comp->setVec3("samples[" + std::to_string(i) + "]", g_ssao_samples[i]);
	}

	// Bind uniforms
	g_shader_comp->setVec3("bounds_NWU", g_vmf_info_bounds.NWU);
	g_shader_comp->setVec3("bounds_SEL", g_vmf_info_bounds.SEL);

	g_shader_comp->setVec4("color_objective",	g_tar_config->m_color_objective);
	g_shader_comp->setVec4("color_buyzone",		g_tar_config->m_color_buyzone);
	g_shader_comp->setVec4("color_cover",		g_tar_config->m_color_cover);
	g_shader_comp->setVec4("color_cover2",		g_tar_config->m_color_cover2);

	g_mesh_screen_quad->Draw();
#pragma endregion
}