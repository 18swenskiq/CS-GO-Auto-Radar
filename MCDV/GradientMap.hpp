#pragma once
#include <stdint.h>
#include <string>

#include "util.h"
#include "interpolation.h"

struct Color255 {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;

	Color255() {

	}

	Color255(std::string src) {
		std::vector<std::string> strings;
		strings = split(src);

		if (strings.size() >= 1) r = ::atoi(strings[0].c_str());
		if (strings.size() >= 2) g = ::atoi(strings[1].c_str());
		if (strings.size() >= 3) b = ::atoi(strings[2].c_str());
		if (strings.size() >= 4) a = ::atoi(strings[3].c_str());
	}
};

class GradientTexture : public Texture{
public:
	Color255 c0;
	Color255 c1;
	Color255 c2;

	//unsigned int texture_id;

	GradientTexture(Color255 _c0, Color255 _c1, Color255 _c2) {
		this->c0 = _c0; this->c1 = _c1; this->c2 = _c2;

		glGenTextures(1, &this->texture_id);

		unsigned char* data = (unsigned char*)malloc(256*4);

		// Do texture generation
		for (int i = 0; i < 256; i++) {
			Color255* a = &this->c0;
			Color255* b = &this->c1;

			if (i >= 128) { a = b; b = &this->c2; }

			data[i * 4 + 0] = lerpT(a->r, b->r, (float)(i % 128) / 128.0f);
			data[i * 4 + 1] = lerpT(a->g, b->g, (float)(i % 128) / 128.0f);
			data[i * 4 + 2] = lerpT(a->b, b->b, (float)(i % 128) / 128.0f);
			data[i * 4 + 3] = lerpT(a->a, b->a, (float)(i % 128) / 128.0f);
		}

		glBindTexture(GL_TEXTURE_2D, this->texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(data);
	}

	/*void bindOnSlot(int slot = 0) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, this->texture_id);
	}*/
};

class WGradientTexture : public Texture{
public:
	WGradientTexture(std::vector<entity*>& tarcol) {
		std::map<float, Color255> colorMap;

		for (auto && i : tarcol) {
			colorMap.insert({ i->m_origin.y, kv::tryGetStringValue(i->m_keyvalues, "_light", "128 128 128 255") });
		}

		// Fill with blank color
		if (colorMap.size() < 1) {
			colorMap.insert({ 0.0f, Color255("128 128 128 255") });
		}

		// Copy first to last
		if (colorMap.size() < 2) {
			colorMap.insert({ 949.4342231f , colorMap.begin()->second });
		}

		std::map<float, Color255>::iterator it_end = colorMap.end();
		it_end--;

		float dy = it_end->first - colorMap.begin()->first;

		std::map<float, Color255>::iterator it = colorMap.begin();
		std::map<float, Color255>::iterator it_next = colorMap.begin();
		it_next++;
		
		glGenTextures(1, &this->texture_id);
		unsigned char* data = (unsigned char*)malloc(2048 * 4);

		for (int i = 0; i < 2048; i++) {
			float pc = (float)i / (float)2048;
			float absdist = colorMap.begin()->first + (pc * dy);

			if (absdist > it_next->first) {
				if (it_next != colorMap.end()) {
					it++;
					it_next++;
				}
			}

			float subpc = remap(absdist, it->first, it_next->first, 0, 1);

			data[i * 4 + 0] = lerpT<float>(it->second.r, it_next->second.r, subpc);
			data[i * 4 + 1] = lerpT<float>(it->second.g, it_next->second.g, subpc);
			data[i * 4 + 2] = lerpT<float>(it->second.b, it_next->second.b, subpc);
			data[i * 4 + 3] = lerpT<float>(it->second.a, it_next->second.a, subpc);
		}

		glBindTexture(GL_TEXTURE_2D, this->texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(data);
	}
};