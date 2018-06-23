#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>


#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "vdf.hpp"

class Radar {
public:
	float pos_x = 0.0f;
	float pos_y = 0.0f;
	float scale = 0.0f;

	Radar(std::string path) {
		std::cout << "Opening radar file" << std::endl;

		std::ifstream t(path);
		std::string data((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		
		kv::FileData kvfile(data);

		this->pos_x = ::atof(kvfile.headNode.SubBlocks[0].Values["pos_x"].c_str());
		this->pos_y = ::atof(kvfile.headNode.SubBlocks[0].Values["pos_y"].c_str());
		this->scale = ::atof(kvfile.headNode.SubBlocks[0].Values["scale"].c_str());

		std::cout << "X:{" << this->pos_x << "} Y:{" << this->pos_y << "} SCALE:{" << this->scale << "}" << std::endl;
	}
};