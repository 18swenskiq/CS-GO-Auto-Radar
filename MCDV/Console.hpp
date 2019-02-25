#include "GLFWUtil.hpp"
#include <string>
#include <map>
#include <GLFW\glfw3.h>

#include "util.h"

#include "TextFont.hpp"

enum MSG_STATUS {
	INFO = 0,
	SUCCESS,
	ERR
};

std::map<MSG_STATUS, glm::vec3> MSG_STATUS_COLOR {
	{ MSG_STATUS::INFO, glm::vec3(0.75f, 0.75f, 0.75f) },
	{ MSG_STATUS::SUCCESS, glm::vec3(0.2f, 1.0f, 0.2f) },
	{ MSG_STATUS::ERR, glm::vec3(1.0f, 0.2f, 0.2f) }
};

class Console{
private:
	std::map<std::string, int*> cVarBook = {};
	std::map<std::string, void*> cCmdBook = {};
	std::string current_input_buffer = "";

	TextFont* ui_input_buffer;
	TextFont* ui_last_result;

	util_keyHandler* keyHandler;
public:
	bool isEnabled = false;

	Console(util_keyHandler* keyhandler, int* window_width, int* window_height) { //Todo; set private references to width/height
		this->keyHandler = keyhandler;

		//Setup
		this->ui_input_buffer = new TextFont(">> ");
		this->ui_input_buffer->size = glm::vec2(1.0f / *window_width, 1.0f / *window_height) * 2.0f;
		this->ui_input_buffer->alpha = 1.0f;
		this->ui_input_buffer->color = glm::vec3(1.0f, 1.0f, 1.0f);
		this->ui_input_buffer->screenPosition = glm::vec2(0, (1.0f / *window_height) * 30.0f);

		//Console reply
		this->ui_last_result = new TextFont("...");
		this->ui_last_result->size = glm::vec2(1.0f / *window_width, 1.0f / *window_height) * 2.0f;
		this->ui_last_result->alpha = 1.0f;
		this->ui_last_result->color = glm::vec3(0.75f, 0.75f, 0.75f);
		this->ui_last_result->screenPosition = glm::vec2(0, (1.0f / *window_height) * 15.0f);
	}

	Console() {

	}

	void RegisterCVar(std::string name, int* variableAddr) {
		this->cVarBook.insert({ name, variableAddr });

		std::cout << "Registered CVAR: " << name << " at address: 0x" <<std::hex << variableAddr << std::endl;
	}

	void RegisterCmd(std::string name, void* addr) {
		this->cCmdBook.insert({ name, addr });
		std::cout << "Registered CCMD: " << name << " at address: 0x" << std::hex << addr << std::endl;
	}

	void CallCmd(std::string name) {
		((void(*)(void)) this->cCmdBook[name])(); //Call assigned CMD->function
	}

	void FeedBack(std::string s, MSG_STATUS status = MSG_STATUS::INFO) {
		std::cout << "Con :: " << s << std::endl;
		this->ui_last_result->color = MSG_STATUS_COLOR[status]; // Assign correct color to output text
		this->ui_last_result->SetText(s);
	}

	void parseRead() {
		std::vector<std::string> parts = split(this->current_input_buffer);

		if (parts.size() > 0) {
			if (this->cVarBook.count(parts[0]) > 0) {
				//Fond in CVARs

				if (parts.size() > 1) { //edit cvar
					*this->cVarBook[parts[0]] = std::stoi(parts[1]);
					this->FeedBack("Var set: " + std::string(parts[0]));
				}
				else {
					this->FeedBack((std::string(parts[0]) + " = " + std::to_string(*this->cVarBook[parts[0]])));
				}
			}

			else if (this->cCmdBook.count(parts[0]) > 0) {
				this->CallCmd(parts[0]);
				if(parts[0] != "HELP")
					this->FeedBack(std::string("Called: *") + parts[0] + std::string( " @ 0x") + to_string<unsigned int>((unsigned int)this->cCmdBook[parts[0]], std::hex));
			}

			else {
				this->FeedBack("No registered command registered for '" + parts[0] + "'", MSG_STATUS::ERR);
			}
		}
	}

	void handleKeysTick() {
		if (!this->isEnabled) return;

		bool modified = false;
		for (int i = 44; i < 94; i++) { //Check every key starting from space
			if (this->keyHandler->getKeyDown(i)) {
				this->current_input_buffer += (char)i;
				modified = true;
			}
		}

		//And space bar...
		if (this->keyHandler->getKeyDown(32)) {
			this->current_input_buffer += (char)32;
			modified = true;
		}

		if (this->keyHandler->getKeyDown(GLFW_KEY_BACKSPACE)) {
			if (this->current_input_buffer.size() > 0) {
				this->current_input_buffer.pop_back();
				modified = true;
			}
		}

		if (this->keyHandler->getKeyDown(GLFW_KEY_ENTER)) {
			this->parseRead();
			this->current_input_buffer = "";
			modified = true;
		}

		if (modified) {
			this->ui_input_buffer->SetText(">> " + this->current_input_buffer);
		}
	}

	void draw() {
		if (!this->isEnabled) return;
		this->ui_last_result->DrawWithBackground();
		this->ui_input_buffer->DrawWithBackground();
	}
};