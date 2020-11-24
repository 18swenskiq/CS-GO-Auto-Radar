#include "main.h"

std::string g_game_path = "H:/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo";
std::string g_mapfile_path = "sample_stuff/de_tavr_test";
std::string g_mapname;
std::string g_mapfile_name;
std::string g_folder_overviews;
std::string g_folder_resources;

bool		g_onlyMasks = false;
bool		g_Masks		= false;

vmf* g_vmf_file;
tar_config* g_tar_config;


#ifdef DXBUILD
DXShaderCombo* dx_shader_gBuffer;
DXShaderCombo* dx_shader_iBuffer;
DXShaderCombo* dx_shader_comp;
DXShaderCombo* dx_shader_multilayer_blend;
DXShaderCombo* dx_shader_multilayer_final;
DXShaderCombo* dx_shader_fxaa;
DXShaderCombo* dx_shader_msaa;

uint32_t dx_renderWidth = 1024;
uint32_t dx_renderHeight = 1024;
uint32_t dx_msaa_mul = 1;
#endif



#ifdef GLBUILD
Shader* g_shader_gBuffer;
Shader* g_shader_iBuffer;
Shader* g_shader_comp;
Shader* g_shader_multilayer_blend;
Shader* g_shader_multilayer_final;
Shader* g_shader_fxaa;
Shader* g_shader_msaa;

GBuffer* g_gbuffer;
GBuffer* g_gbuffer_clean;
MBuffer* g_mask_playspace;
MBuffer* g_mask_buyzone;
MBuffer* g_mask_objectives;
FBuffer* g_fbuffer_generic;
FBuffer* g_fbuffer_generic1;

Mesh* g_mesh_screen_quad;
Texture* g_texture_background;
Texture* g_texture_modulate;
std::vector<glm::vec3> g_ssao_samples;
Texture* g_ssao_rotations;

uint32_t g_renderWidth = 1024;
uint32_t g_renderHeight = 1024;
uint32_t g_msaa_mul = 1;
#endif


int main(int argc, const char** argv) {
	try {
		return app(argc, argv);
	}
	catch (cxxopts::OptionException& e) {
		std::cerr << "Parse error: " << e.what() << "\n";
	}

	return 1;
}


void render_to_png(int x, int y, const char* filepath) 
{
	#ifdef GLBUILD
	void* data = malloc(4 * x * y);

	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_flip_vertically_on_write(true);
	stbi_write_png(filepath, x, y, 4, data, x * 4);

	free(data);
	#endif
}

void save_to_dds(int x, int y, const char* filepath, IMG imgmode)
{
	#ifdef GLBUILD
	void* data = malloc(6 * x * y);

	glReadPixels(0, 0, x, y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	dds_write((uint8_t*)data, filepath, x, y, imgmode);

	free(data);
	#endif GLBUILD
}

int app(int argc, const char** argv) {

	#ifndef _DEBUG

	// -----------Command line options-----------
	cxxopts::Options options("AutoRadar", "Auto radar");
	options.add_options()
		("v,version",	"Shows the software version")
		("g,game",		"(REQUIRED) Specify game path", cxxopts::value<std::string>()->default_value(""))
		("m,mapfile",	"(REQUIRED) Specify the map file (vmf)", cxxopts::value<std::string>()->default_value(""))

		("d,dumpMasks", "Toggles whether auto radar should output mask images (resources/map_file.resources/)")
		("o,onlyMasks", "Specift whether auto radar should only output mask images and do nothing else (resources/map_file.resources)")

		("positional", "Positional parameters", cxxopts::value<std::vector<std::string>>());

	options.parse_positional("positional");
	auto result = options.parse(argc, argv);

	/* Check required parameters */
	if (result.count("game")) g_game_path = sutil::ReplaceAll(result["game"].as<std::string>(), "\n", "");
	else throw cxxopts::option_required_exception("game");

	if (result.count("mapfile")) g_mapfile_path = result["mapfile"].as<std::string>();
	else if (result.count("positional")) {
		auto& positional = result["positional"].as<std::vector<std::string>>();

		g_mapfile_path = sutil::ReplaceAll(positional[0], "\n", "");
	}
	else throw cxxopts::option_required_exception("mapfile"); // We need a map file

	//Clean paths to what we can deal with
	g_mapfile_path = sutil::ReplaceAll(g_mapfile_path, "\\", "/");
	g_game_path = sutil::ReplaceAll(g_game_path, "\\", "/");

	/* Check the rest of the flags */
	g_onlyMasks = result["onlyMasks"].as<bool>();
	g_Masks = result["dumpMasks"].as<bool>() || g_onlyMasks;

	#endif






	// -----------Output stuff-----------
	g_mapfile_name = split(g_mapfile_path, '/').back();
	g_folder_overviews = g_game_path + "/resource/overviews/";
	g_folder_resources = g_folder_overviews + g_mapfile_name + ".resources/";




	#ifdef DXBUILD
	// -----------Set up graphics API-----------
	DXRendering* dxr = new DXRendering();
	dxr->PrintVersion();
	#endif

	#ifdef GLBUILD
	// -----------Set up graphics API-----------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(g_renderWidth, g_renderHeight, "Ceci n'est pas une window", NULL, NULL);
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
	#endif




	// -----------Various filesystem things-----------
	vfilesys* filesys = new vfilesys(g_game_path + "/gameinfo.txt");
	vmf::LinkVFileSystem(filesys);
	g_vmf_file = vmf::from_file(g_mapfile_path + ".vmf", *dxr);
	g_vmf_file->InitModelDict(*dxr);
	g_tar_config = new tar_config(g_vmf_file);



	#ifdef DXBUILD
	// -----------Other graphics API setup-----------
	
	const std::wstring gBufferV = L"shaders/gBufferV.cso";
	const std::wstring gBufferP = L"shaders/gBufferP.cso";
	const std::wstring iBufferP = L"shaders/iBufferP.cso";
	const std::wstring fullscreenBaseV = L"shaders/fullscreenbaseV.cso";
	const std::wstring fullscreenBaseP = L"shaders/fullscreenbaseP.cso";
	const std::wstring multilayerBlendP = L"shaders/ss_comp_multilayer_blendP.cso";
	const std::wstring multilayerFinalP = L"shaders/ss_comp_multilayer_finalstageP.cso";
	const std::wstring ss_fxaaP = L"shaders/ss_fxaaP.cso";
	const std::wstring ss_msaaP = L"shaders/ss_msaaP.cso";

	dx_shader_gBuffer = new DXShaderCombo(dxr->LoadVertexShader(gBufferV), dxr->LoadPixelShader(gBufferP));
	dx_shader_iBuffer = new DXShaderCombo(dxr->LoadVertexShader(gBufferV), dxr->LoadPixelShader(iBufferP));
	dx_shader_comp = new DXShaderCombo(dxr->LoadVertexShader(fullscreenBaseV), dxr->LoadPixelShader(fullscreenBaseP));
	dx_shader_multilayer_blend = new DXShaderCombo(dxr->LoadVertexShader(fullscreenBaseV), dxr->LoadPixelShader(multilayerBlendP));
	dx_shader_multilayer_final = new DXShaderCombo(dxr->LoadVertexShader(fullscreenBaseV), dxr->LoadPixelShader(multilayerFinalP));
	dx_shader_fxaa = new DXShaderCombo(dxr->LoadVertexShader(fullscreenBaseV), dxr->LoadPixelShader(ss_fxaaP));
	dx_shader_msaa = new DXShaderCombo(dxr->LoadVertexShader(fullscreenBaseV), dxr->LoadPixelShader(ss_msaaP));

	if (g_tar_config->m_sampling_mode == sampling_mode::MSAA4x || g_tar_config->m_sampling_mode == sampling_mode::MSAA16x)
	{
		dx_msaa_mul = g_tar_config->m_sampling_mode;
	}

	// Set up draw buffers

	// Load Textures

	#endif


	#ifdef GLBUILD
	// -----------Other graphics API setup-----------
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
	g_shader_gBuffer =	new Shader("shaders/gBuffer.vs", "shaders/gBuffer.fs");
	g_shader_iBuffer =	new Shader("shaders/gBuffer.vs", "shaders/iBuffer.fs");
	g_shader_comp =		new Shader("shaders/fullscreenbase.vs", "shaders/fullscreenbase.fs");
	g_shader_multilayer_blend = new Shader("shaders/fullscreenbase.vs", "shaders/ss_comp_multilayer_blend.fs");
	g_shader_multilayer_final = new Shader("shaders/fullscreenbase.vs", "shaders/ss_comp_multilayer_finalstage.fs");
	g_shader_fxaa =		new Shader("shaders/fullscreenbase.vs", "shaders/ss_fxaa.fs");
	g_shader_msaa =		new Shader("shaders/fullscreenbase.vs", "shaders/ss_msaa.fs");

	if(g_tar_config->m_sampling_mode == sampling_mode::MSAA4x ||
		g_tar_config->m_sampling_mode == sampling_mode::MSAA16x)
	g_msaa_mul = g_tar_config->m_sampling_mode;

	// Set up draw buffers
	g_mask_playspace =	new MBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_mask_objectives = new MBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_mask_buyzone =	new MBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_gbuffer =			new GBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_gbuffer_clean =   new GBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_fbuffer_generic = new FBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);
	g_fbuffer_generic1 =new FBuffer(g_renderWidth * g_msaa_mul, g_renderHeight * g_msaa_mul);

	// Load textures
	g_texture_background = g_tar_config->m_texture_background;//new Texture("textures/grid.png");
	g_texture_modulate = new Texture("textures/modulate.png");
	g_ssao_samples = get_ssao_samples(TAR_AO_SAMPLES);
	g_ssao_rotations = new ssao_rotations_texture();

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CW);
	#endif




#ifdef GLBUILD
	// -----------Render It-----------

	std::map<tar_config_layer*, FBuffer*> _flayers;

	// Render all map segments
	int c = 0;
	for (auto && layer : g_tar_config->layers){
		_flayers.insert({ &layer, new FBuffer(g_renderWidth, g_renderHeight) });
		render_config(layer, "layer" + std::to_string(c++) + ".png", _flayers[&layer]);
	}

	// Render out everything so we got acess to G Buffer info in final composite
	if(g_tar_config->layers.size() > 1) render_config(tar_config_layer(), "layerx.png", NULL);
	GBuffer::Unbind();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	int i = 0;
	for (auto && megalayer : g_tar_config->layers){
		g_fbuffer_generic->Bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_shader_multilayer_blend->use();
		g_shader_multilayer_blend->setInt("tex_layer", 1);
		g_shader_multilayer_blend->setInt("gbuffer_height", 0);
		g_shader_multilayer_blend->setFloat("saturation", 0.1f);
		g_shader_multilayer_blend->setFloat("value", 0.5669f);
		g_shader_multilayer_blend->setFloat("active", 0.0f);

		bool above = false;

		for(int x = 0; x < g_tar_config->layers.size(); x++)
		{
			tar_config_layer* l = &g_tar_config->layers[g_tar_config->layers.size() - x - 1];
			if (l == &megalayer) { above = true; continue; }

			_flayers[l]->BindRTToTexSlot(1);
			_flayers[l]->BindHeightToTexSlot(0);

			g_shader_multilayer_blend->setFloat("layer_target", !above? l->layer_min: l->layer_max);
			g_shader_multilayer_blend->setFloat("layer_min", l->layer_min);
			g_shader_multilayer_blend->setFloat("layer_max", l->layer_max);
			
			g_mesh_screen_quad->Draw();
		}

		g_shader_multilayer_blend->setFloat("active", 1.0f);
		g_shader_multilayer_blend->setFloat("layer_min", megalayer.layer_min);
		g_shader_multilayer_blend->setFloat("layer_max", megalayer.layer_max);

		_flayers[&megalayer]->BindRTToTexSlot(1);
		_flayers[&megalayer]->BindHeightToTexSlot(0);
		g_mesh_screen_quad->Draw();

		
		FBuffer::Unbind();

		g_fbuffer_generic1->Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		g_shader_multilayer_final->use();
		g_shader_multilayer_final->setFloat("blend_outline", g_tar_config->m_outline_enable? 1.0f:0.0f);
		g_shader_multilayer_final->setVec4("color_outline", g_tar_config->m_color_outline);
		g_shader_multilayer_final->setInt("outline_width", g_tar_config->m_outline_width * g_msaa_mul);

		g_tar_config->m_texture_background->bindOnSlot(0);
		g_shader_multilayer_final->setInt("tex_background", 0);

		g_fbuffer_generic->BindRTToTexSlot(1);
		g_shader_multilayer_final->setInt("tex_layer", 1);
		g_mesh_screen_quad->Draw();

		// Apply FXAA
		if (g_tar_config->m_sampling_mode == sampling_mode::FXAA) {
			FBuffer::Unbind();

			g_shader_fxaa->use();
			g_shader_fxaa->setInt("sampler0", 0);
			g_shader_fxaa->setVec2("resolution", glm::vec2(g_renderWidth, g_renderHeight));
			g_fbuffer_generic1->BindRTToTexSlot(0);

			g_mesh_screen_quad->Draw();
		}
		else if (g_tar_config->m_sampling_mode == sampling_mode::MSAA16x
			|| g_tar_config->m_sampling_mode == sampling_mode::MSAA4x)
		{
			FBuffer::Unbind();

			g_shader_msaa->use();
			g_shader_msaa->setInt("sampler0", 0);
			g_fbuffer_generic1->BindRTToTexSlot(0);
			g_mesh_screen_quad->Draw();
		}

		// final composite

		if (i == 0) {
			if(g_tar_config->m_write_dds)
				save_to_dds(g_renderWidth, g_renderHeight, filesys->create_output_filepath("resource/overviews/" + g_mapfile_name +"_radar.dds", true).c_str(), g_tar_config->m_dds_img_mode);

			if(g_tar_config->m_write_png)
				render_to_png(g_renderWidth, g_renderHeight, filesys->create_output_filepath("resource/overviews/" + g_mapfile_name + "_radar.png", true).c_str());

			i++;
		}
		else {
			if (g_tar_config->m_write_dds)
				save_to_dds(g_renderWidth, g_renderHeight, filesys->create_output_filepath("resource/overviews/" + g_mapfile_name + "_layer" + std::to_string(i) + "_radar.dds", true).c_str(), g_tar_config->m_dds_img_mode);

			if (g_tar_config->m_write_png)
				render_to_png(g_renderWidth, g_renderHeight, filesys->create_output_filepath("resource/overviews/" + g_mapfile_name + "_layer" + std::to_string(i) + "_radar.png", true).c_str());

			i++;
		}
		FBuffer::Unbind();
	}
#endif 



	// -----------Write text file-----------
	if (g_tar_config->m_write_txt)
	{
		std::cout << "Generating radar .TXT... ";

		kv::DataBlock node_radar = kv::DataBlock();
		node_radar.name = "\"" + g_mapfile_name + "\"";
		node_radar.Values.insert({ "material", "overviews/" + g_mapfile_name });

		node_radar.Values.insert({ "pos_x", std::to_string(g_tar_config->m_view_origin.x) });
		node_radar.Values.insert({ "pos_y", std::to_string(g_tar_config->m_view_origin.y) });

		#ifdef GLBUILD
		node_radar.Values.insert({ "scale", std::to_string(g_tar_config->m_render_ortho_scale / g_renderWidth) });
		#else`
		node_radar.Values.insert({ "scale", std::to_string(g_tar_config->m_render_ortho_scale / dx_renderWidth) });
		#endif

		if (g_tar_config->layers.size() > 1) {
			kv::DataBlock node_vsections = kv::DataBlock();
			node_vsections.name = "\"verticalsections\"";

			int ln = 0;
			for (auto && layer : g_tar_config->layers) {
				kv::DataBlock node_layer = kv::DataBlock();
				if (ln == 0) {
					node_layer.name = "\"default\""; ln++;
				}
				else node_layer.name = "\"layer" + std::to_string(ln++) + "\"";

				node_layer.Values.insert({ "AltitudeMin", std::to_string(layer.layer_max) });
				node_layer.Values.insert({ "AltitudeMax", std::to_string(layer.layer_min) });

				node_vsections.SubBlocks.push_back(node_layer);
			}

			node_radar.SubBlocks.push_back(node_vsections);
		}

		// Try resolve spawn positions
		glm::vec3* loc_spawnCT = g_vmf_file->calculateSpawnAVG_PMIN("info_player_counterterrorist");
		glm::vec3* loc_spawnT = g_vmf_file->calculateSpawnAVG_PMIN("info_player_terrorist");

		if (loc_spawnCT != NULL) {
			node_radar.Values.insert({ "CTSpawn_x", std::to_string(util::roundf(remap(loc_spawnCT->x, g_tar_config->m_view_origin.x, g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
			node_radar.Values.insert({ "CTSpawn_y", std::to_string(util::roundf(remap(loc_spawnCT->z, g_tar_config->m_view_origin.y, g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
		}
		if (loc_spawnT != NULL) {
			node_radar.Values.insert({ "TSpawn_x", std::to_string(util::roundf(remap(loc_spawnT->x, g_tar_config->m_view_origin.x, g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
			node_radar.Values.insert({ "TSpawn_y", std::to_string(util::roundf(remap(loc_spawnT->z, g_tar_config->m_view_origin.y, g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
		}

		int hostn = 1;
		for (auto && hostage : g_vmf_file->get_entities_by_classname("info_hostage_spawn")) {
			node_radar.Values.insert({ "Hostage" + std::to_string(hostn) + "_x", std::to_string(util::roundf(remap(hostage->m_origin.x, g_tar_config->m_view_origin.x, g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
			node_radar.Values.insert({ "Hostage" + std::to_string(hostn++) + "_y", std::to_string(util::roundf(remap(hostage->m_origin.z, g_tar_config->m_view_origin.y, g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale, 0.0f, 1.0f), 0.01f)) });
		}

		std::ofstream out(filesys->create_output_filepath("resource/overviews/" + g_mapfile_name + ".txt", true).c_str());
		out << "// (Squidski's) TAVR - AUTO RADAR. v 3.0.0a\n";
		node_radar.Serialize(out);
		out.close();
	}

	#ifdef GLBUILD
	IL_EXIT:
	glfwTerminate();
	#endif
#ifdef _DEBUG
	system("PAUSE");
#endif
	return 0;
}




#ifdef GLBUILD
// ----------- Buffer gen geo -----------
void render_config(tar_config_layer layer, const std::string& layerName, FBuffer* drawTarget) {
	// G BUFFER GENERATION ======================================================================================

	glm::mat4 l_mat4_projm = glm::ortho(
		g_tar_config->m_view_origin.x,										// -X
		g_tar_config->m_view_origin.x + g_tar_config->m_render_ortho_scale,	// +X
		g_tar_config->m_view_origin.y - g_tar_config->m_render_ortho_scale,	// -Y
		g_tar_config->m_view_origin.y,										// +Y
		-10000.0f, //g_tar_config->m_map_bounds.NWU.y,									// NEARZ
		10000.0f);// g_tar_config->m_map_bounds.SEL.y);									// FARZ

	glm::mat4 l_mat4_viewm = glm::lookAt(
		glm::vec3(0, 0, 0),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0, 0, 1));

	std::cout << "v" << layer.layer_min << "\n";
	std::cout << "^" << layer.layer_max << "\n";

	g_vmf_file->SetMinMax(layer.layer_min, layer.layer_max);

	g_gbuffer->Bind();

	glClearColor(-10000.0, -10000.0, -10000.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	g_shader_gBuffer->use();
	g_shader_gBuffer->setMatrix("projection", l_mat4_projm);
	g_shader_gBuffer->setMatrix("view", l_mat4_viewm);

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

	// Draw cover with cover flag set

	GBuffer::Unbind();

	g_gbuffer_clean->Bind();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_layout, g_tar_config->m_visgroup_mask }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer);
	g_vmf_file->DrawEntities(g_shader_gBuffer);

	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_cover }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer, {}, TAR_MIBUFFER_COVER0);
	g_vmf_file->DrawEntities(g_shader_gBuffer, {}, TAR_MIBUFFER_COVER0);

	g_vmf_file->SetFilters({ g_tar_config->m_visgroup_overlap }, { "func_detail", "prop_static" });
	g_vmf_file->DrawWorld(g_shader_gBuffer, {}, TAR_MIBUFFER_OVERLAP);
	g_vmf_file->DrawEntities(g_shader_gBuffer, {}, TAR_MIBUFFER_OVERLAP);

	GBuffer::Unbind();





	//-----------Mask Gen-----------
	g_mask_playspace->Bind();
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shader_iBuffer->use();
	g_shader_iBuffer->setMatrix("projection", l_mat4_projm);
	g_shader_iBuffer->setMatrix("view", l_mat4_viewm);

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




	// ----------- FINAL COMPOSITE -----------

	MBuffer::Unbind(); // Release any frame buffer

	if(drawTarget != NULL)
		drawTarget->Bind();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);

	g_shader_comp->use();

	// Bind RT's

	g_tar_config->m_texture_gradient->bindOnSlot( 1 );
	g_shader_comp->setInt("tex_gradient",		1 );

	g_gbuffer->BindPositionBufferToTexSlot    (	2 );
	g_shader_comp->setInt("gbuffer_position",	2 );

	g_gbuffer->BindNormalBufferToTexSlot      (	3 );
	g_shader_comp->setInt("gbuffer_normal",  	3 );

	g_gbuffer_clean->BindInfoBufferToTexSlot		  ( 4 );
	g_shader_comp->setInt("gbuffer_info",       4 );

	g_mask_playspace->BindMaskBufferToTexSlot (	5 );
	g_shader_comp->setInt("umask_playspace",	5 );

	g_mask_objectives->BindMaskBufferToTexSlot( 6 );
	g_shader_comp->setInt("umask_objectives",	6 );

	g_mask_buyzone->BindMaskBufferToTexSlot   (	7 );
	g_shader_comp->setInt("umask_buyzone",		7 );

	g_gbuffer_clean->BindPositionBufferToTexSlot(8);
	g_shader_comp->setInt("gbuffer_clean_position", 8);

	g_gbuffer_clean->BindNormalBufferToTexSlot(12);
	g_shader_comp->setInt("gbuffer_clean_normal", 12);

	g_gbuffer_clean->BindOriginBufferToTexSlot(11);
	g_shader_comp->setInt("gbuffer_origin", 11);

	g_texture_modulate->bindOnSlot(10);
	g_shader_comp->setInt("tex_modulate", 10);

	g_ssao_rotations->bindOnSlot              ( 9 );
	g_shader_comp->setInt("ssaoRotations",      9 );
	g_shader_comp->setFloat("ssaoScale", g_tar_config->m_ao_scale);
	g_shader_comp->setMatrix("projection", l_mat4_projm);
	g_shader_comp->setMatrix("view", l_mat4_viewm);

	for (int i = 0; i < TAR_AO_SAMPLES; i++) {
		g_shader_comp->setVec3("samples[" + std::to_string(i) + "]", g_ssao_samples[i]);
	}

	// Bind uniforms
	g_shader_comp->setVec3("bounds_NWU", g_tar_config->m_map_bounds.NWU);
	g_shader_comp->setVec3("bounds_SEL", g_tar_config->m_map_bounds.SEL);

	g_shader_comp->setVec4("color_objective",	g_tar_config->m_color_objective);
	g_shader_comp->setVec4("color_buyzone",		g_tar_config->m_color_buyzone);
	g_shader_comp->setVec4("color_cover",		g_tar_config->m_color_cover);
	g_shader_comp->setVec4("color_cover2",		g_tar_config->m_color_cover2);
	g_shader_comp->setVec4("color_ao",			g_tar_config->m_color_ao);
	g_shader_comp->setFloat("blend_objective_stripes", g_tar_config->m_outline_stripes_enable? 0.0f: 1.0f);
	g_shader_comp->setFloat("blend_ao", g_tar_config->m_ao_enable? 1.0f: 0.0f);
	g_shader_comp->setInt("mssascale", g_msaa_mul);

	g_mesh_screen_quad->Draw();
}

#endif


/*

NVIDIA optimus systems will default to intel integrated graphics chips.

This export gets picked up by NVIDIA drivers (v 302+).
It will force usage of the dedicated video device in the machine, which likely has full coverage of 3.3

*/
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}