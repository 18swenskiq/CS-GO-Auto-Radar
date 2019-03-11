#pragma once
#include <string>
#include <iostream>
#include <string>

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


class Texture
{
public:
	unsigned int texture_id;
	Texture(std::string filepath);

	void bind();
	void bindOnSlot(int slot);

	~Texture();
};



bool USE_DEBUG2 = false;


Texture::Texture(std::string filepath)
{
	//stbi_set_flip_vertically_on_load(true);

	glGenTextures(1, &this->texture_id);

	//Load texture using stb_image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, this->texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		if (USE_DEBUG2)
		{
			std::cout << "Texture loaded, info:" << std::endl;
			std::cout << filepath << std::endl;
			std::cout << "width: " << width << std::endl;
			std::cout << "height: " << height << std::endl;
		}
		else
		{
			std::cout << "Loading texture: " << filepath << std::endl;
		}
	}
	else
	{
		std::cout << "ERROR::IMAGE::LOAD_FAILED" << std::endl;
	}
}

Texture::~Texture()
{
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
}

void Texture::bindOnSlot(int slot = 0) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
}