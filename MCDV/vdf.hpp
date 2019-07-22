#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <regex>

#include <chrono>

#include "Util.h"

#define _USE_REGEX

namespace kv{
	//const std::regex reg_kv("(\"([^=\"]*)\")|([^=\\s]+)");
	const std::regex reg_kv(R"vv("(.*?)"|([^\s]+))vv");

	template<typename T>
	T tryGetValue(std::map<std::string, std::string> map, const char* key, T defaultValue) {
		if (!map.count(key)) return defaultValue;
		return static_cast<T>(::atof(map[key].c_str()));
	}

	std::string tryGetStringValue(std::map<std::string, std::string> map, const char* key, std::string defaultValue = "") {
		if (!map.count(key)) return defaultValue;
		return map[key];
	}

	/* Create list from map and key */
	std::vector<std::string> getList(std::map<std::string, std::string> map, const char* key) {
		std::vector<std::string> list;

		int vc = -1;
		while (map.count(key + (++vc > 0 ? std::to_string(vc) : ""))) list.push_back(map[key + (vc > 0 ? std::to_string(vc) : "")]);

		return list;
	}

	class DataBlock{
	public:
		std::string name = "";
		std::vector<DataBlock*> SubBlocks;
		std::map<std::string, std::string> Values;

		// One node up. Should only by null if its head node.
		DataBlock* parent = NULL;

		DataBlock() {}

		// Init node with name
		DataBlock(std::istringstream* stream, const std::string& name = "", DataBlock* _parent = NULL): parent(_parent) {
			this->name = sutil::trim(name);

			std::string line, prev = "";
			while (std::getline(*stream, line)) {
				line = split(line, "//")[0];

				if (sutil::get_unquoted_material(line).find("{") != std::string::npos) {
					std::string pname = prev;
					prev.erase(std::remove(prev.begin(), prev.end(), '"'), prev.end());
					this->SubBlocks.push_back(new DataBlock(stream, pname, this));
					continue;
				}
				if (sutil::get_unquoted_material(line).find("}") != std::string::npos) {
					return;
				}

#ifndef _USE_REGEX
				// Regex is so fucking slow in debug mode its unreal
				// Rather have it mess up than take 10 hours

				std::vector<std::string> s1 = split(line, '"');
				std::vector<std::string> strings;

				if (s1.size() >= 3)
				{
					strings.push_back(s1[1]);
					strings.push_back(s1[3]);
				}
#else
				std::vector<std::string> strings = sutil::regexmulti(line, reg_kv);
#endif

				for (int i = 0; i < strings.size(); i++) {
					strings[i] = sutil::removeChar(strings[i], '"');
				}

				if (strings.size() == 2) {
					// Fix for multiply defined key-values (THANKS VALVE APPRECIATE THAT)
					std::string keyname = strings[0];
					int i = -1;
					while (this->Values.count((++i > 0 ? keyname + std::to_string(i) : keyname)));

					this->Values.insert({ i > 0 ? keyname + std::to_string(i) : keyname, strings[1] });
				}

				prev = line;
			}
		}

		// Take this node and write it and all other sub-nodes into filestream
		void Serialize(std::ofstream& stream, int depth = 0){
			//Build indentation levels
			std::string indenta = "";
			for (int i = 0; i < depth; i++)
				indenta += "\t";
			std::string indentb = indenta + "\t";

			if (depth >= 0)
				stream << indenta << this->name << std::endl << indenta << "{" << std::endl;

			//Write kvs
			for (auto const& x : this->Values)
				stream << indentb << "\"" << x.first << "\" \"" << x.second << "\"" << std::endl;

			//Write subdata recursively
			for (int i = 0; i < this->SubBlocks.size(); i++) 
				this->SubBlocks[i]->Serialize(stream, depth + 1);

			if (depth >= 0)
				stream << indenta << "}" << std::endl;
		}

		// Gets the first data block by name. Returns null if not found
		DataBlock* GetFirstByName(const std::string& _name) {
			for (auto&& i: this->SubBlocks)
				if (i->name == _name)
					return i;
			return NULL;
		}

		// Gets all datablocks by name. Returns a list of pointers to matches
		std::vector<DataBlock*> GetAllByName(const std::string& _name) {
			std::vector<DataBlock*> c;

			for (auto&& i: this->SubBlocks) 
				if (i->name == _name)
					c.push_back(i);

			return c;
		}

		// Append a child node (copy-construct)
		void AddNode(const DataBlock& node) {
			DataBlock* block = new DataBlock(node);

			block->parent = this;
			this->SubBlocks.push_back(block);
		}

		// Destructor; delete child datablocks (recursive fuck-off delete)
		~DataBlock() {
			for(auto&& i: this->SubBlocks) delete i;
		}
	};

	// Wrapper to set everything up.
	class FileData
	{
	public:
		DataBlock* headNode;

		FileData(std::string filestring)
		{
			std::istringstream sr(filestring);

			auto start = std::chrono::high_resolution_clock::now();

			this->headNode = new DataBlock(&sr, "");

			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			std::cout << "KV Read time: " << milliseconds << "ms" << std::endl;
		}

		FileData()
		{

		}

		~FileData() { delete this->headNode; }
	};
}