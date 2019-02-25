#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLFWUtil.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Shader.hpp"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

//Class for rendering fonts
class TextFont
{
private:
	static Shader* shader; //Font shader
	static unsigned int textureID; //Font image
	static unsigned int textureBackgroundID; //Font background image

	std::string text;

	unsigned int VAO;
	unsigned int VBO;
	int elementCount;

public:
	static void init() { //Setup shader and texture loading etc.
		std::cout << "Initializing fonts" << std::endl;


		//Load shader
		TextFont::shader = new Shader("shaders/textfont.vs", "shaders/textfont.fs");

		//Load font texture
		glGenTextures(1, &TextFont::textureID);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //1 byte per pixel

											   //Load texture using stb_image
		int width, height, nrChannels;
		unsigned char* data = stbi_load("fonts/dina-r.png", &width, &height, &nrChannels, 1);
		if (data){
			glBindTexture(GL_TEXTURE_2D, TextFont::textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			//glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			stbi_image_free(data);
		}
		else{
			std::cout << "ERROR::FONT::LOAD_FAILED" << std::endl;
		}

		glGenTextures(1, &TextFont::textureBackgroundID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //1 byte per pixel

		unsigned char* data_background = stbi_load("fonts/dina-r-background.png", &width, &height, &nrChannels, 1);
		if (data) {
			glBindTexture(GL_TEXTURE_2D, TextFont::textureBackgroundID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			//glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			stbi_image_free(data);
		}
		else {
			std::cout << "ERROR::FONT::LOAD_FAILED" << std::endl;
		}
	}

	glm::vec2 screenPosition;
	glm::vec3 color;
	float alpha;
	glm::vec2 size;

	void SetText(std::string text, glm::vec2 screenPos, glm::vec2 size) {
		this->text = text;
		this->screenPosition = screenPos;
		this->size = size;

		//Do generation

		//Clear old mesh
		if (this->VAO != NULL) {
			glDeleteVertexArrays(1, &this->VAO);
			glDeleteBuffers(1, &this->VBO);
		}

		std::vector<float> verts;

		float current_x = 0;
		float current_y = 0;

		//Create vertex arrays
		float charwidth = 1.0f / 94.0f;

		for (int i = 0; i < text.length(); i++) {
			char c = text[i];

			float toffset = (float)(c - 0x21) * charwidth;

			if (!(c < 0x21 || c > 0x7E)) { //Check if out of bounds
				std::vector<float> character = {

					current_x,			current_y,				toffset,				1.0f,
					current_x + 7.0f,	current_y + 12.0f,		toffset + charwidth,	0.0f,
					current_x,			current_y + 12.0f,		toffset,				0.0f,

					current_x,			current_y,				toffset,				1.0f,
					current_x + 7.0f,	current_y,				toffset + charwidth,	1.0f,
					current_x + 7.0f,	current_y + 12.0f,		toffset + charwidth,	0.0f

				};

				//Append character vert data
				verts.insert(verts.end(), character.begin(), character.end());
			}

			if (c == 0x0A) {
				current_x = 0.0f;
				current_y -= 15.0f;
			}
			else current_x += 7; //Font character width
		}

		this->elementCount = verts.size() / 4; //XYUV

		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_STATIC_DRAW);

		glBindVertexArray(this->VAO);

		//position attribute
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void SetText(std::string s) {
		this->SetText(s, this->screenPosition, this->size);
	}

	void Draw() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_CULL_FACE);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		
		//glDepthMask(0);

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, glm::vec3((screenPosition.x - 0.5f) * 2.0f, -(screenPosition.y - 0.5f)*2.0f, 0));
		model = glm::scale(model, glm::vec3(this->size.x, this->size.y, 0.0f));

		this->shader->use();
		this->shader->setVec3("color", this->color);
		this->shader->setFloat("alpha", this->alpha);
		this->shader->setMatrix("model", model);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextFont::textureID); //bind texture
		this->shader->setInt("text", 0);

		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}

	void DrawWithBackground() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_CULL_FACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);

		//glDepthMask(0);
		glDisable(GL_DEPTH_TEST);

		glm::mat4 model = glm::mat4();
		model = glm::translate(model, glm::vec3((screenPosition.x - 0.5f) * 2.0f, -(screenPosition.y - 0.5f)*2.0f, 0));
		model = glm::scale(model, glm::vec3(this->size.x, this->size.y, 0.0f));

		//bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextFont::textureBackgroundID);

		this->shader->use();
		
		this->shader->setVec3("color", glm::vec3(0, 0, 0));
		this->shader->setFloat("alpha", this->alpha);
		this->shader->setMatrix("model", model);

		this->shader->setInt("text", 0);

		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);

		//Draw foreground
		this->shader->setVec3("color", this->color);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextFont::textureID);

		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, this->elementCount);
	}

	std::string GetText() {
		return this->text;
	}

	TextFont(std::string text) {
		this->SetText(text, glm::vec2(0, 0), glm::vec2(1.0f, 1.0f));
	}

	~TextFont() {
		//Clear old mesh
		if (this->VAO != NULL) {
			glDeleteVertexArrays(1, &this->VAO);
			glDeleteBuffers(1, &this->VBO);
		}
	}
};

unsigned int TextFont::textureID;
unsigned int TextFont::textureBackgroundID;
Shader* TextFont::shader;