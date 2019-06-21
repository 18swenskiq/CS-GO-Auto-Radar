#pragma once
#include "vmf_new.hpp"

#include <vector>
#include <map>
#include <set>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Texture.hpp"
#include "GradientMap.hpp"
#include "dds.hpp"

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
public:
	std::vector<tar_config_layer> layers;

	// Camera settings
	glm::vec2		m_view_origin;
	float			m_render_ortho_scale;

	BoundingBox		m_map_bounds;
	IMG				m_dds_img_mode;
	sampling_mode	m_sampling_mode;

	// Textures
	Texture*		m_texture_gradient;
	Texture*		m_texture_background;

	// Lighting settings
	bool			m_ao_enable;
	float			m_ao_scale;
	bool			m_shadows_enable;

	// Outline settings
	bool			m_outline_enable;
	int				m_outline_width;
	bool			m_outline_stripes_enable;

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

	tar_config(vmf* v) {
		// Search for tar_config entity
		std::map<std::string, std::string> kvs;
		std::vector<entity*> cfgs = v->get_entities_by_classname("tar_config");
		if (cfgs.size() != 0) kvs = cfgs[0]->m_keyvalues;

		// Load color scheme gradient
		// TODO: Entity based gradients
		std::string schemeNum = kv::tryGetStringValue(kvs, "colorScheme", "0");
		if (schemeNum == "-1" || schemeNum == "-2") {
			this->m_texture_gradient = new GradientTexture(
				kv::tryGetStringValue(kvs, "customCol0", "39  56  79  255"),
				kv::tryGetStringValue(kvs, "customCol1", "77  74  72  255"),
				kv::tryGetStringValue(kvs, "customCol2", "178 113 65  255")
			);
		} else {
			this->m_texture_gradient = new Texture("textures/gradients/gradientmap_" + schemeNum + ".png", true);
		}

		this->m_texture_background = new Texture("textures/" + kv::tryGetStringValue(kvs, "background", "grid.png"), true);
		
		// Load the rest of the config options
		this->m_ao_enable				= (kv::tryGetStringValue(kvs, "enableAO", "1") == "1");
		this->m_ao_scale				= kv::tryGetValue(kvs, "aoSize", 1000.0f);

		this->m_outline_enable			= (kv::tryGetStringValue(kvs, "enableOutline", "0") == "1");
		this->m_outline_width			= kv::tryGetValue(kvs, "outlineWidth", 2);

		this->m_shadows_enable			= (kv::tryGetStringValue(kvs, "enableShadows", "0") == "1");

		this->m_color_cover				= parseVec4(kv::tryGetStringValue(kvs, "zColCover",		"179 179 179 255"));
		this->m_color_cover2			= parseVec4(kv::tryGetStringValue(kvs, "zColCover2",	"85  85  85  170"));
		this->m_color_outline			= parseVec4(kv::tryGetStringValue(kvs, "zColOutline",	"204 204 204 153"));
		this->m_color_ao				= parseVec4(kv::tryGetStringValue(kvs, "zColAO",		"0   0   0   255"));
		this->m_color_buyzone			= parseVec4(kv::tryGetStringValue(kvs, "zColBuyzone",   "46  211 57  170"));
		this->m_color_objective			= parseVec4(kv::tryGetStringValue(kvs, "zColObjective", "196 75  44  255"));
		
		this->m_outline_stripes_enable	= (kv::tryGetStringValue(kvs, "ObjectiveUseStripes", "0") == "1");

		this->m_visgroup_cover			= kv::tryGetStringValue(kvs, "vgroup_cover", "tar_cover");
		this->m_visgroup_layout			= kv::tryGetStringValue(kvs, "vgroup_layout", "tar_layout");
		this->m_visgroup_mask			= kv::tryGetStringValue(kvs, "vgroup_negative", "tar_mask");
		this->m_visgroup_overlap		= kv::tryGetStringValue(kvs, "vgroup_overlap", "tar_overlap");

		this->m_dds_img_mode = IMG::MODE_DXT1;

		switch (hash(kv::tryGetStringValue(kvs, "ddsMode", "0").c_str())) {
			case hash("1"): this->m_dds_img_mode = IMG::MODE_DXT5; break;
			case hash("2"): this->m_dds_img_mode = IMG::MODE_RGB888; break;		
		}

		this->m_sampling_mode = sampling_mode::FXAA;
		switch (hash(kv::tryGetStringValue(kvs, "ssaam", "3").c_str())) {
			case hash("1"): this->m_sampling_mode = sampling_mode::MSAA4x; break;
			case hash("2"): this->m_sampling_mode = sampling_mode::MSAA16x; break;
		}

		// Configure camera setup
		this->m_map_bounds = v->getVisgroupBounds(this->m_visgroup_layout);

		std::cout << -this->m_map_bounds.NWU.x << "," << this->m_map_bounds.NWU.y << "," << this->m_map_bounds.NWU.z << "\n";
		std::cout << -this->m_map_bounds.SEL.x << "," << this->m_map_bounds.SEL.y << "," << this->m_map_bounds.SEL.z << "\n";

		for (auto && min : v->get_entities_by_classname("tar_min"))
			this->m_map_bounds.SEL.y = glm::max(min->m_origin.y, this->m_map_bounds.SEL.y);

		for(auto && max : v->get_entities_by_classname("tar_max"))
			this->m_map_bounds.NWU.y = glm::min(max->m_origin.y, this->m_map_bounds.NWU.y);

		float padding = 128.0f;

		float x_bounds_min = -this->m_map_bounds.NWU.x - padding;
		float x_bounds_max = -this->m_map_bounds.SEL.x + padding;

		float y_bounds_min = this->m_map_bounds.SEL.z - padding;
		float y_bounds_max = this->m_map_bounds.NWU.z + padding;

		float dist_x = x_bounds_max - x_bounds_min;
		float dist_y = y_bounds_max - y_bounds_min;

		float mx_dist = glm::max(dist_x, dist_y);

		float justify_x = (mx_dist - dist_x) * 0.5f;
		float justify_y = (mx_dist - dist_y) * 0.5f;
		
		this->m_render_ortho_scale =	glm::round((mx_dist / 1024.0f) / 0.01f) * 0.01f * 1024.0f;
		this->m_view_origin =			glm::vec2(x_bounds_min - justify_x, y_bounds_max + justify_y);

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
};