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

struct tar_config_layer {
	float layer_max;
	float layer_min;

	tar_config_layer() 
		: 
		layer_max(10000.0f), 
		layer_min(10000.0f) {}

	tar_config_layer(float min, float max)
		:
		layer_max(max),
		layer_min(min) {}
};

class tar_config {
public:
	std::vector<tar_config_layer> layers;

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
		this->m_color_cover2			= parseVec4(kv::tryGetStringValue(kvs, "zColCover2",	"0   0   0   0  "));
		this->m_color_outline			= parseVec4(kv::tryGetStringValue(kvs, "zColOutline",	"204 204 204 153"));
		this->m_color_ao				= parseVec4(kv::tryGetStringValue(kvs, "zColAO",		"0   0   0   255"));
		this->m_color_buyzone			= parseVec4(kv::tryGetStringValue(kvs, "zColBuyzone",   "46  211 57  170"));
		this->m_color_objective			= parseVec4(kv::tryGetStringValue(kvs, "zColObjective", "196 75  44  255"));
		
		this->m_outline_stripes_enable	= (kv::tryGetStringValue(kvs, "ObjectiveUseStripes", "0") == "1");

		this->m_visgroup_cover			= kv::tryGetStringValue(kvs, "vgroup_cover", "tar_cover");
		this->m_visgroup_layout			= kv::tryGetStringValue(kvs, "vgroup_layout", "tar_layout");
		this->m_visgroup_mask			= kv::tryGetStringValue(kvs, "vgroup_negative", "tar_mask");
		this->m_visgroup_overlap		= kv::tryGetStringValue(kvs, "vgroup_overlap", "tar_overlap");

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

		this->layers.push_back(tar_config_layer(-10000.0f, splits[0]));

		for (int i = 0; i < splits.size() - 1; i++) 
			this->layers.push_back(tar_config_layer(splits[i], splits[i + 1]));

		this->layers.push_back(tar_config_layer(splits.back(), 10000.0f));
	}
};