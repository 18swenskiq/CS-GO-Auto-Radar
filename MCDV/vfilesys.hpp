#pragma once
#include <string>
#include "vpk.hpp"
#include "vdf.hpp"
#include "vvd.hpp"
#include "vtx.hpp"
#include "../AutoRadar_installer/FileSystemHelper.h"
#include "loguru.hpp"

class vfilesys : public util::verboseControl {
public:
	// Cached items
	vpk::index* vpkIndex;

	// Paths
	std::string dir_gamedir;  // Where the gameinfo.txt is (and where we should output to)
	std::string dir_exedir;   // Counter-Strike Global Offensive directory
	std::string dir_bin;      // exedir + /bin/ folder

	std::vector<std::string> searchPaths; // List of paths to search for stuff (these are all absolute)

	/* Create a file system helper from game info */
	vfilesys(std::string gameinfo, std::string exedir = ""){
		LOG_SCOPE_FUNCTION(1);

		if (!fs::checkFileExist(gameinfo.c_str())) throw std::exception("gameinfo.txt not found");

		LOG_F(1, "Loading gameinfo.txt");

		// Load gameinfo
		std::ifstream ifs(gameinfo);
		if (!ifs) {
			throw std::exception("File read error");
		}

		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		kv::FileData kv_gameinfo = kv::FileData(str);

		// Set gamedir
		this->dir_gamedir = fs::getDirName(gameinfo);

		// Look for csgo.exe to set exedir
		std::string dir_upOne = this->dir_gamedir + "../"; // go up to csgo.exe (or at least it should be here)

		std::cout << dir_upOne << "\n";

		if (exedir != "") 
			if(fs::checkFileExist((exedir + "/csgo.exe").c_str())) this->dir_exedir = exedir + "/";
			else throw std::exception("Specified exedir was incorrect");

		else if (fs::checkFileExist((dir_upOne + "csgo.exe").c_str())) this->dir_exedir = dir_upOne;
		else throw std::exception("Can't find csgo.exe");

		// Set bindir
		this->dir_bin = this->dir_exedir + "bin/";

		// Collect search paths from gameinfo.txt
		for (auto && path : kv::getList( kv_gameinfo.headNode->GetFirstByName("\"GameInfo\"")->GetFirstByName("FileSystem")->GetFirstByName("SearchPaths")->Values, "Game")) {
			std::string _path = "";
			if (path.find(':') != path.npos) _path = path + "/"; //this path is abs
			else _path = this->dir_exedir + path + "/"; // relative path to exedir
			_path = sutil::ReplaceAll(_path, "\\", "//");

			if (fs::checkFileExist(_path.c_str())) this->searchPaths.push_back(_path);
		}

		this->searchPaths.push_back(this->dir_gamedir);

		// Look for pak01_dir.vpk in all search paths
		for (auto && sp : this->searchPaths) {
			if (fs::checkFileExist((sp + "pak01_dir.vpk").c_str())) {
				this->vpkIndex = new vpk::index(sp + "pak01_dir.vpk");
				goto IL_FOUND;
			}
		}

		std::cout << "Warning: could not find pak01_dir.vpk...\n";

	IL_FOUND:
		std::cout << "Finished setting up filesystem.\n";
	}

	/* Create a file handle on an existing resource file. Could be from vpk, could be from custom. Returns null if not found */
	template<typename T>
	T* get_resource_handle(std::string relpath) {
		// Order of importantness:
		// 0) VPK file (actual game files)
		// 1) anything in csgo folders

		if (this->vpkIndex != NULL) {
			vpk::vEntry* vEntry = this->vpkIndex->find(relpath);

			if (vEntry != NULL) {
				std::string pakDir = this->dir_exedir + "csgo/pak01_" + sutil::pad0(std::to_string(vEntry->entryInfo.ArchiveIndex), 3) + ".vpk";

				std::ifstream pkHandle(pakDir, std::ios::in | std::ios::binary);

				pkHandle.seekg(vEntry->entryInfo.EntryOffset); // set that ifstream to read from the correct location
				return new T(&pkHandle);
			}
		}
		
		// Check all search paths for custom content
		for (auto && sp : this->searchPaths) {
			if (fs::checkFileExist((sp + relpath).c_str())) {
				std::ifstream pkHandle(sp + relpath, std::ios::in | std::ios::binary);
				return new T(&pkHandle);
			}
		}

		return NULL;
	}

	/* Generate a path to a file inside the gamedir. Optionally automatically create new directories (shell). */
	std::string create_output_filepath(std::string relpath, bool mkdr = false, bool verbose = true) {
		this->use_verbose = verbose;

		std::string fullpath = this->dir_gamedir + relpath;

		if (mkdr) {
			if (!fs::checkFileExist(fs::getDirName(fullpath).c_str())) this->debug("MKDIR: ", fs::getDirName(fullpath));
			fs::mkdr(fs::getDirName(fullpath).c_str());
		}

		return fullpath;
	}
};