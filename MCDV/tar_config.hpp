#pragma once
#include "vmf.hpp"

#include <vector>
#include <map>
#include <set>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Texture.hpp"
#include "GradientMap.hpp"
#include "dds.hpp"

#include "imgui_color_gradient.h"

struct tar_config_layer {
	float layer_max;
	float layer_min;

	tar_config_layer() 
		: 
		layer_max(-10000.0f), 
		layer_min(10000.0f) {}

	tar_config_layer(float min, float max)
		:
		layer_max(max),
		layer_min(min) {}
};

enum sampling_mode {
	FXAA,
	NONE,
	MSAA4x,
	MSAA16x
};

class tar_config {
private:
	vmf* v;
	std::map<std::string, std::string> kvs;

public:
	unsigned int m_gradient_texture;

	std::vector<tar_config_layer> layers;

	// Camera settings
	glm::vec2		m_view_origin;
	float			m_render_ortho_scale;
	glm::mat4		m_pmPersp;	// Recommended perspective calculation
	glm::mat4		m_pmView;	// Recommended model matrix

	BoundingBox		m_map_bounds;
	IMG				m_dds_img_mode;
	sampling_mode	m_sampling_mode;

	// Lighting settings
	bool			m_ao_enable;
	float			m_ao_scale;
	bool			m_shadows_enable;
	float			m_shadows_tracelength = 1024.0f;
	int				m_shadows_samplecount = 128;

	// Outline settings
	bool			m_outline_enable;
	int				m_outline_width;
	bool			m_outline_stripes_enable;

	bool			m_write_dds = true;
	bool			m_write_txt = true;
	bool			m_write_png = false;

	// Color settings
	glm::vec4		m_color_cover;
	glm::vec4		m_color_cover2;
	glm::vec4		m_color_outline;
	glm::vec4		m_color_ao;
	glm::vec4		m_color_buyzone;
	glm::vec4		m_color_objective;

	// Visgroups
	std::string		m_visgroup_layout;
	std::string		m_visgroup_mask;
	std::string		m_visgroup_cover;
	std::string		m_visgroup_overlap;

	tar_config() {}

	tar_config(vmf* v) : v(v) {
		LOG_F(1, "Loading config entity");

		// Search for tar_config entity
		std::vector<entity*> cfgs = v->get_entities_by_classname("tar_config");
		if (cfgs.size() != 0) kvs = cfgs[0]->m_keyvalues;

		// Load the rest of the config options
		this->m_ao_enable = (kv::tryGetStringValue(kvs, "enableAO", "1") == "1");
		this->m_ao_scale = kv::tryGetValue(kvs, "aoSize", 1000.0f);

		this->m_outline_enable = (kv::tryGetStringValue(kvs, "enableOutline", "0") == "1");
		this->m_outline_width = kv::tryGetValue(kvs, "outlineWidth", 2);

		this->m_shadows_enable = (kv::tryGetStringValue(kvs, "enableShadows", "0") == "1");

		this->m_color_cover = parseVec4(kv::tryGetStringValue(kvs, "zColCover", "179 179 179 255"));
		this->m_color_cover2 = parseVec4(kv::tryGetStringValue(kvs, "zColCover2", "85  85  85  170"));
		this->m_color_outline = parseVec4(kv::tryGetStringValue(kvs, "zColOutline", "204 204 204 153"));
		this->m_color_ao = parseVec4(kv::tryGetStringValue(kvs, "zColAO", "0   0   0   255"));
		this->m_color_buyzone = parseVec4(kv::tryGetStringValue(kvs, "zColBuyzone", "46  211 57  170"));
		this->m_color_objective = parseVec4(kv::tryGetStringValue(kvs, "zColObjective", "196 75  44  255"));

		this->m_outline_stripes_enable = (kv::tryGetStringValue(kvs, "ObjectiveUseStripes", "0") == "1");

		this->m_visgroup_cover = kv::tryGetStringValue(kvs, "vgroup_cover", "tar_cover");
		this->m_visgroup_layout = kv::tryGetStringValue(kvs, "vgroup_layout", "tar_layout");
		this->m_visgroup_mask = kv::tryGetStringValue(kvs, "vgroup_negative", "tar_mask");
		this->m_visgroup_overlap = kv::tryGetStringValue(kvs, "vgroup_overlap", "tar_overlap");

		this->m_dds_img_mode = IMG::MODE_DXT1;

		switch (hash(kv::tryGetStringValue(kvs, "ddsMode", "0").c_str())) {
		case hash("1"): this->m_dds_img_mode = IMG::MODE_DXT5; break;
		case hash("3"): case hash("2"): this->m_dds_img_mode = IMG::MODE_RGB888; break;
		case hash("4"): this->m_dds_img_mode = IMG::MODE_DXT1_1BA; break;
		}

		this->m_sampling_mode = sampling_mode::FXAA;
		switch (hash(kv::tryGetStringValue(kvs, "ssaam", "3").c_str())) {
		case hash("1"): this->m_sampling_mode = sampling_mode::MSAA4x; break;
		case hash("2"): this->m_sampling_mode = sampling_mode::MSAA16x; break;
		case hash("0"): this->m_sampling_mode = sampling_mode::NONE; break;
		}

		switch (hash(kv::tryGetStringValue(kvs, "outputMode", "0").c_str())) {
		case hash("1"):
			this->m_write_png = true;
		case hash("0"):
			this->m_write_dds = true;
			this->m_write_txt = true;
			break;

		case hash("2"):
			this->m_write_png = true;
			this->m_write_txt = true;
			this->m_write_dds = false;
			break;

		case hash("3"):
			this->m_write_txt = false;
			this->m_write_png = false;
			this->m_write_dds = true;
			break;

		case hash("4"):
			this->m_write_dds = false;
			this->m_write_txt = false;
			this->m_write_png = true;
			break;
		}

		LOG_F(1, "Pre-processing visgroups into bit masks");
		v->IterSolids([=](solid* s) {
			if (s->m_editorvalues.m_hashed_visgroups.count(hash(m_visgroup_layout.c_str()))) s->m_setChannels(TAR_CHANNEL_LAYOUT_0);
			if (s->m_editorvalues.m_hashed_visgroups.count(hash(m_visgroup_overlap.c_str()))) s->m_setChannels(TAR_CHANNEL_LAYOUT_1);
			if (s->m_editorvalues.m_hashed_visgroups.count(hash(m_visgroup_mask.c_str()))) s->m_setChannels(TAR_CHANNEL_MASK);
		});

		v->IterEntities([=](entity* e, const std::string& classname) {
			if (e->m_editorvalues.m_hashed_visgroups.count(hash(m_visgroup_layout.c_str()))) e->m_setChannels(TAR_CHANNEL_LAYOUT_0);
			if (e->m_editorvalues.m_hashed_visgroups.count(hash(m_visgroup_overlap.c_str()))) e->m_setChannels(TAR_CHANNEL_LAYOUT_1);

			if (classname == "func_buyzone") e->m_appendChannels(TAR_CHANNEL_BUYZONE);
			if (classname == "func_bomb_target" || classname == "func_hostage_rescue") 
				e->m_appendChannels(TAR_CHANNEL_OBJECTIVES);
		});


		LOG_F(INFO, "DDS Write: %s", (this->m_write_dds ? "ON" : "OFF"));
		LOG_F(INFO, "TXT Write: %s", (this->m_write_txt ? "ON" : "OFF"));
		LOG_F(INFO, "PNG Write: %s", (this->m_write_png ? "ON" : "OFF"));

		// Configure camera setup
		this->m_map_bounds = v->getVisgroupBounds(TAR_CHANNEL_LAYOUT);

		LOG_F(WARNING, "Map bounds: %.2f %.2f %.2f  ::  %.2f %.2f %.2f", m_map_bounds.MIN.x, m_map_bounds.MIN.y, m_map_bounds.MIN.z, m_map_bounds.MAX.x, m_map_bounds.MAX.y, m_map_bounds.MAX.z);

		for (auto&& min : v->get_entities_by_classname("tar_min"))
			this->m_map_bounds.MIN.y = glm::max(min->m_origin.y, this->m_map_bounds.MIN.y);
		
		for (auto&& max : v->get_entities_by_classname("tar_max"))
			this->m_map_bounds.MAX.y = glm::min(max->m_origin.y, this->m_map_bounds.MAX.y);

		float padding = 128.0f;

		float x_bounds_min = this->m_map_bounds.MIN.x - padding;
		float x_bounds_max = this->m_map_bounds.MAX.x + padding;

		float y_bounds_min = this->m_map_bounds.MIN.z - padding;
		float y_bounds_max = this->m_map_bounds.MAX.z + padding;

		float dist_x = x_bounds_max - x_bounds_min;
		float dist_y = y_bounds_max - y_bounds_min;

		float mx_dist = glm::max(dist_x, dist_y);

		float justify_x = (mx_dist - dist_x) * 0.5f;
		float justify_y = (mx_dist - dist_y) * 0.5f;

		this->m_render_ortho_scale = glm::round((mx_dist / 1024.0f) / 0.01f) * 0.01f * 1024.0f;
		this->m_view_origin = glm::vec2((x_bounds_min + x_bounds_max) * 0.5f, (y_bounds_min + y_bounds_max) * 0.5f);

		// Calculate optimal projections
		this->m_pmPersp = glm::ortho(mx_dist * 0.5f, -mx_dist * 0.5f, mx_dist * 0.5f, -mx_dist * 0.5f, -10000.0f, 10000.0f);
		this->m_pmView = glm::lookAt(
			glm::vec3(m_view_origin.x, 0, m_view_origin.y), 
			glm::vec3(m_view_origin.x, -1, m_view_origin.y), 
			glm::vec3(0, 0, 1));

		// Get map splits
		std::vector<entity*> splitters = v->get_entities_by_classname("tar_map_divider");

		if (splitters.size() == 0) {
			this->layers.push_back(tar_config_layer());
			return;
		}
		// Process the split entities
		std::vector<float> splits = {};
		std::sort(splits.begin(), splits.end());

		for (auto && s : splitters) splits.push_back(s->m_origin.y);

		this->layers.push_back(tar_config_layer(10000.0f, splits[0]));

		for (int i = 0; i < splits.size() - 1; i++)
			this->layers.push_back(tar_config_layer(splits[i], splits[i + 1]));

		this->layers.push_back(tar_config_layer(splits.back(), -10000.0f));
	}

	void gen_textures(ImGradient& grad) {
		glGenTextures(1, &this->m_gradient_texture);
		glBindTexture(GL_TEXTURE_2D, this->m_gradient_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, grad.cache_ptr());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	// Update the gradient texture with ImGradient
	void update_gradient(ImGradient& grad) {
		glBindTexture(GL_TEXTURE_2D, this->m_gradient_texture);
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			0,
			0,
			256,
			1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			grad.cache_ptr()
		);
	}

	// Update the gradient texture via presets
	void update_gradient(const int& presetID) { }

	// Bind gradient texture to slot
	void bind_gradient_texture(const unsigned int& slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->m_gradient_texture);
	}
};