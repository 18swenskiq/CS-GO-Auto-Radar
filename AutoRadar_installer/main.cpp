#include "../MCDV/vdf.hpp"
#include "../MCDV/wc.hpp"
#include "../MCDV/util.h"

#include <windows.h>
#include <ShlObj_core.h>
#include <iostream>
#include <io.h>
#include <fstream>

#include "ConColor.h"

#include "FileSystemHelper.h"

std::string steam_install_path = "C:\\Program Files (x86)\\Steam\\";
std::string csgo_sdk_bin_path = "";

int exit() {
	cc::reset();
	system("PAUSE");
	return -1;
}

int main(){
	cc::setup();

	/* Load install configuration */
	std::ifstream ifs_vinfo("versioninfo.vdf");
	if (!ifs_vinfo) {
		cc::error();
		std::cout << "versioninfo.vdf not found!!!" << std::endl;
		return exit();
	}

	cc::info();
	
	std::string vinfo_str((std::istreambuf_iterator<char>(ifs_vinfo)), std::istreambuf_iterator<char>());
	kv::FileData vinfo(vinfo_str);
	kv::DataBlock vinfodata = vinfo.headNode.SubBlocks[0];

	cc::fancy(); std::cout << "Installing version: " << vinfodata.Values["version"] << "\n";

#pragma region sdk_detect
	/* Get steam installation path */

	cc::info(); std::cout << "Getting steam installation path from windows registry\n";

	HKEY hKey = NULL;
	char buffer[1024];

	bool regReadSuccess = true;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Valve\\Steam", NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS){
		DWORD size;
		if (RegQueryValueEx(hKey, "SteamPath", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS){
			steam_install_path = buffer;
			steam_install_path += "\\";
		}
		else regReadSuccess = false;
	}
	else regReadSuccess = false;

	RegCloseKey(hKey);

	if (!regReadSuccess) {
		cc::warning();
		std::cout << "Failed to read registry key: 'Software\\Valve\\Steam\\SteamPath'\nDefaulting to C:\\Program Files (x86)\\Steam\\ installation...\n";
	}

	cc::info();
	std::cout << "Reading steam library folders\n";

	/* Read library folders file */

	std::vector<std::string> libraryFolders;
	libraryFolders.push_back(steam_install_path + "steammapps\\common\\");

	std::ifstream ifs(steam_install_path + "steamapps\\libraryfolders.vdf");
	if (!ifs) {
		std::cout << "Libraryfolders.vdf not found. Skipping search...\n" << std::endl;
	}
	else {
		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		kv::FileData libFolders(str);

		kv::DataBlock* libFoldersDB = libFolders.headNode.GetFirstByName("\"LibraryFolders\"");

		if (libFoldersDB != NULL) {
			int index = 0;
			while (libFoldersDB->Values.count(std::to_string(++index))) libraryFolders.push_back(libFoldersDB->Values[std::to_string(index)] + "\\steamapps\\common\\");
		}
	}

	if (libraryFolders.size() == 0) std::cout << "No library folders found, defaulting to steamapps common folder...\n";

	/* Scan for csgo sdk installations */

	std::cout << "Scanning for SDK installation\n";

	for (auto && folder : libraryFolders) {
		if (_access_s((folder + "Counter-Strike Global Offensive\\bin\\SDKLauncher.exe").c_str(), 0) == 0) {
			csgo_sdk_bin_path = folder + "Counter-Strike Global Offensive\\bin\\";
		}
	}

	if (csgo_sdk_bin_path == "") {
		cc::error();
		std::cout << "Failed to find CS:GO SDK bin.\n";
		return exit();
	}

#pragma endregion

#pragma region copyfiles

	/* Start doing the heavy work */
	std::cout << "Copying files\n________________________________________________________\n\n";
	
	// Copy folders
	for (auto && folder : vinfodata.GetAllByName("folder")){
		std::string source_folder = kv::tryGetStringValue(folder.Values, "path");
		std::string target_folder = kv::tryGetStringValue(folder.Values, "target");

		if ((source_folder == "") || (target_folder == "")) {
			cc::warning(); std::cout << "Missing source/destination paths. Skipping...\n";
			continue;
		}

		cc::info();
		std::cout << "Copying folder: " << source_folder << "\n";

		source_folder = source_folder + "\\";
		target_folder = csgo_sdk_bin_path + target_folder + "\\";

		std::vector<std::string> files = fs::getFilesInDirectoryRecursive(source_folder);

		for (auto && f : files) {
			std::string fDstFolder = target_folder + fs::getDirName(f);

			if (_access_s(fDstFolder.c_str(), 0)) {
				std::cout << "mkdr  " << fs::getDirName(f) << "\n";
				SHCreateDirectoryExA(NULL, fDstFolder.c_str(), NULL);
			}

			std::cout <<     "copy  " << f << "\n";

			fs::copyFile(source_folder + f, target_folder + f);
		}

		std::cout << "\n";
	}

#pragma endregion

#pragma region CommandSequences

	// Install command sequences
	std::cout << "Installing command sequences\n________________________________________________________\n\n";

	for (auto && seqH : vinfodata.GetAllByName("CommandSequenceFile")) {
		std::string target_file = kv::tryGetStringValue(seqH.Values, "target");

		if (target_file == "") {
			cc::warning(); std::cout << "Missing target file. Skipping CommandSeq...\n";
			continue;
		}

		cc::info();

		target_file = csgo_sdk_bin_path + target_file;

		fs::copyFile(target_file, target_file + ".bak");

		wc::filedata WcFile(target_file);

		std::cout << "Writing to: " << kv::tryGetStringValue(seqH.Values, "target") << "\n";

		for (auto && seqS : seqH.GetAllByName("Sequence")) {
			std::cout << "Installing sequence:  " << kv::tryGetStringValue(seqS.Values, "name", "error-name") << "\n";
			std::string name = kv::tryGetStringValue(seqS.Values, "name", "error-name");

			for (auto && seq : WcFile.sequences) {
				if (strstr(seq.name, name.c_str()) != NULL) {
					seq.write_enable = false;
				}
			}

			wc::Sequence seq_new = wc::Sequence();
			strcpy_s(seq_new.name, name.c_str());

			for (auto && cmd : seqS.GetAllByName("Command"))
			{
				wc::Command command_build = wc::Command();
				command_build.is_enabled = kv::tryGetValue(cmd.Values, "is_enabled", 0);
				command_build.special = kv::tryGetValue(cmd.Values, "special", 0);
				command_build.is_long_filename = kv::tryGetValue(cmd.Values, "is_long_filename", 0);
				command_build.ensure_check = kv::tryGetValue(cmd.Values, "ensure_check", 0);
				command_build.use_proc_win = kv::tryGetValue(cmd.Values, "use_proc_win", 0);
				command_build.no_wait = kv::tryGetValue(cmd.Values, "no_wait", 0);;

				std::string executable = kv::tryGetStringValue(cmd.Values, "executable", "error-executable");

				strcpy_s(command_build.executable, executable.c_str());

				std::string args = kv::tryGetStringValue(cmd.Values, "args", "error-args");
				strcpy_s(command_build.args, args.c_str());

				seq_new.commands.push_back(command_build);
			}

			WcFile.sequences.push_back(seq_new);
		}

		WcFile.serialize(target_file);
	}

#pragma endregion

#pragma region FGDEntries

	// Install custom entities
	std::cout << "\nInstalling custom entity entries\n________________________________________________________\n\n";

	std::ifstream ifs_gameconfig(csgo_sdk_bin_path + "GameConfig.txt");
	if (!ifs_gameconfig) {
		cc::error();
		std::cout << "GameConfig.txt not found. custom entites will not be installed...\n" << std::endl;
	}
	else {
		cc::info();
		std::cout << "Adding GameConfig.cfg FGD entries\n";
		fs::copyFile(csgo_sdk_bin_path + "GameConfig.txt", csgo_sdk_bin_path + "GameConfig.txt.bak");

		std::string str_gameconfig((std::istreambuf_iterator<char>(ifs_gameconfig)), std::istreambuf_iterator<char>());
		kv::FileData gameConfig(str_gameconfig);

		kv::DataBlock* hammerBlock = gameConfig.headNode.SubBlocks[0].GetFirstByName("\"Games\"")->GetFirstByName("\"Counter-Strike: Global Offensive\"")->GetFirstByName("\"Hammer\"");
		int freeIndex = -1;

		for (auto && newEntry : vinfodata.GetAllByName("HammerVGDRegistry")){
			// Check if entry exists
			int i = -1;
			bool matched = false;
			while (hammerBlock->Values.count("GameData" + std::to_string(++i))) {
				if (csgo_sdk_bin_path + newEntry.Values["source"] == hammerBlock->Values["GameData" + std::to_string(i)]) {
					matched = true;
					break;
				}
			}

			if (matched) continue;

			while (hammerBlock->Values.count("GameData" + std::to_string(++freeIndex)));
			hammerBlock->Values.insert({ "GameData" + std::to_string(freeIndex), csgo_sdk_bin_path + newEntry.Values["source"]});
		}

		std::cout << "Saving GameConfig.cfg\n";
		std::ofstream out(std::string(csgo_sdk_bin_path + "GameConfig.txt").c_str());
		gameConfig.headNode.SubBlocks[0].Serialize(out);
	}

#pragma endregion

	cc::success(); std::cout << "\nCompleted setup!\n";
	
	/* Small wait to auto close */
	for (int i = 10; i > 0; i--) {
		cc::info();
		std::cout << "Closing in " << i << " seconds...\r";
		Sleep(1000);
	}

	cc::reset();
	std::cout << "\n";
	return 0;
}