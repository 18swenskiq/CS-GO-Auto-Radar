#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <regex>

#include <chrono>

#include "Util.h"

//#define _USE_REGEX

namespace kv
{
	const std::regex reg_kv("(\"([^=\"]*)\")|([^=\\s]+)");

	template<typename T>
	T tryGetValue(std::map<std::string, std::string> map, const char* key, T defaultValue) {
		if (!map.count(key)) return defaultValue;
		return static_cast<T>(::atof(map[key].c_str()));
	}

	std::string tryGetStringValue(std::map<std::string, std::string> map, const char* key, std::string defaultValue = "") {
		if (!map.count(key)) return defaultValue;
		return map[key];
	}

	class DataBlock
	{
	public:
		std::string name = "";
		std::vector<DataBlock> SubBlocks;
		std::map<std::string, std::string> Values;

		DataBlock() {}

		DataBlock(std::istringstream* stream, std::string name = "", void* progress_callback = NULL) {
			this->name = sutil::trim(name);

			std::string line, prev = "";
			while (std::getline(*stream, line)) {
				if(progress_callback != NULL) util::CastFunctionPtr(progress_callback); //Increment line counter

				line = split(line, "//")[0];

				if (line.find("{") != std::string::npos) {
					std::string pname = prev;
					prev.erase(std::remove(prev.begin(), prev.end(), '"'), prev.end());
					this->SubBlocks.push_back(DataBlock(stream, pname, progress_callback));
					continue;
				}
				if (line.find("}") != std::string::npos) {
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
#endif 
#ifdef _USE_REGEX
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

		void Serialize(std::ofstream& stream, int depth = 0)
		{
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
				this->SubBlocks[i].Serialize(stream, depth + 1);

			if (depth >= 0)
				stream << indenta << "}" << std::endl;
		}

		//Scan for sub block with name
		DataBlock* GetFirstByName(std::string _name) {
			for (int i = 0; i < this->SubBlocks.size(); i++) {
				if (_name == this->SubBlocks[i].name)
					return &this->SubBlocks[i];
			}

			return NULL;
		}

		//Gets all sub blocks by type
		std::vector<DataBlock> GetAllByName(std::string _name) {
			std::vector<DataBlock> c;

			for (int i = 0; i < this->SubBlocks.size(); i++) {
				if (_name == this->SubBlocks[i].name)
					c.push_back(this->SubBlocks[i]);
			}

			return c;
		}
	};

	class FileData
	{
	public:
		DataBlock headNode;

		FileData(std::string filestring, void* progress_callback = NULL)
		{
			std::istringstream sr(filestring);

			auto start = std::chrono::high_resolution_clock::now();

			this->headNode = DataBlock(&sr, "", progress_callback);


			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
			std::cout << "KV Read time: " << milliseconds << "ms" << std::endl;
		}

		FileData()
		{

		}

		~FileData() {}
	};
}