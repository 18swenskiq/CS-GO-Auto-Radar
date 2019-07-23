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

#include "..\MCDV\loguru.hpp"
#include "..\MCDV\loguru.cpp"

std::string steam_install_path = "C:\\Program Files (x86)\\Steam\\";
std::string csgo_sdk_bin_path = "";

/*

// Find steam installation ___________________________________________________________________________

if argc > 0:																							// overridden input
	RETURN SDK_BIN

if CheckExist ([DRIVE]:/program files (x86)/steam/) 'steam.exe':										// normal drive letter installs
	steam_path = ^
else: if CheckExist (<reg='Software\Valve\Steam\SteamPath'>/) 'steam.exe':								// check from registry if not found
	steam_path = ^
else: RETURN FAIL																						// can't find steam installation.

CheckExist (steam_path steamapps/common/Counter-Strike Global Offensive/bin/) 'hammer.exe':				// check steam install's regular /common/ folder
	RETURN SDK_BIN

CheckExist '__ steamapps/libraryfolders.vdf'															// check if steam library folder vdf exists
	FOREACH lib_folder :: libraryfolders.vdf:															// iterate the library folders
		CheckExist (lib_folder steamapps/common/Counter-Strike Global Offensive/bin/) 'hammer.exe':		// check for hammer
			RETURN SDK_BIN

RETURN FAIL
*/

int exit() {
	cc::reset();
	system("PAUSE");
	return -1;
}

inline bool checkFileExist(const char* path) {
	return (_access_s(path, 0) == 0);
}

bool get_steam_install_from_winreg(std::string* s) {
	HKEY hKey = NULL;
	char buffer[1024];

	bool regReadSuccess = true;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Valve\\Steam", NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
		DWORD size;
		if (RegQueryValueEx(hKey, "SteamPath", NULL, NULL, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
			steam_install_path = buffer;
			steam_install_path += "\\";
		}
		else regReadSuccess = false;
	}
	else regReadSuccess = false;

	RegCloseKey(hKey);

	if (regReadSuccess)
		*s = buffer;

	return regReadSuccess;
}

std::vector<std::string> get_library_folders_from_vdf(std::string vdf) {
	std::vector<std::string> libraryFolders;

	std::ifstream ifs(vdf);
	if (!ifs) return libraryFolders; //ifs failed to open.

	else {
		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		kv::FileData libFolders(str);

		kv::DataBlock* libFoldersDB = libFolders.headNode->GetFirstByName("\"LibraryFolders\"");

		if (libFoldersDB != NULL) {
			int index = 0;
			while (libFoldersDB->Values.count(std::to_string(++index))) libraryFolders.push_back(libFoldersDB->Values[std::to_string(index)] + "/steamapps/common/");
		}
	}

	return libraryFolders;
}

/* Work through all possible locations of steam / csgo. */
std::string detect_csgo_bin(std::string steam_drive_letter = "c") {
	std::string steam_path = "";

	try {
		if (checkFileExist((steam_drive_letter + ":/program files (x86)/steam/steam.exe").c_str()))
			steam_path = steam_drive_letter + ":/program files (x86)/steam/";

		else if (get_steam_install_from_winreg(&steam_path))
			if (checkFileExist((steam_path + "/steam.exe").c_str()))
				steam_path = steam_path + "/";
			else throw std::exception((	"1) [" + steam_drive_letter + ":/program files (x86)/steam/steam.exe] not found\n2) Steam registry key pointed to wrong location.").c_str());
		else throw std::exception((		"1) [" + steam_drive_letter + ":/program files (x86)/steam/steam.exe] not found\n2) Failed to open registry for steam path.").c_str());

		// steam_path assumed to be correct from here.

		// check steam apps folder
		if (checkFileExist((steam_path + "steamapps/common/Counter-Strike Global Offensive/bin/hammer.exe").c_str()))
			return steam_path + "steamapps/common/Counter-Strike Global Offensive/bin/";

		// check if library folder thingy exists
		if (checkFileExist((steam_path + "steamapps/libraryfolders.vdf").c_str())) {
			for (auto && folder : get_library_folders_from_vdf(steam_path + "steamapps/libraryfolders.vdf"))
				if (checkFileExist((folder + "Counter-Strike Global Offensive/bin/hammer.exe").c_str()))
					return sutil::ReplaceAll(folder, "\\\\", "/") + "Counter-Strike Global Offensive/bin/";

			throw std::exception(("1) [" + steam_path + "steamapps/common/Counter-Strike Global Offensive/bin/hammer.exe] not found\n2) libraryfolders.vdf search failed.").c_str());
		}

		// unable to find hammer using any methods.
		throw std::exception(("1) [" + steam_path + "steamapps/common/Counter-Strike Global Offensive/bin/hammer.exe] not found\n2) libraryfolders.vdf not found.").c_str());
	}
	catch (std::exception e) {
		cc::error(); std::cout << "\nFailed to automatically detect /bin/ folder...\n\n";
		cc::info();  std::cout << "Reasons:\n====================================================================\n";
		cc::reset(); std::cout << e.what();
		cc::info();  std::cout << "\n====================================================================\n\n";
	}

	return "";
}

int main(int argc, const char** argv) {
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
	kv::DataBlock* vinfodata = vinfo.headNode->SubBlocks[0];

	cc::fancy(); std::cout << "Installing version: " << vinfodata->Values["version"] << "\n";
	cc::reset();

	// Overide install
	if (argc > 1) {
		csgo_sdk_bin_path = std::string(argv[1]) + "\\";
		goto IL_PRE_INSTALL;
	}
	else {
		csgo_sdk_bin_path = detect_csgo_bin();
	}

	if (csgo_sdk_bin_path == "") {
		cc::reset(); std::cout << "Having issues?\nRun this installer from command prompt with the first argument being the path to your Counter Strike bin folder\n(the same folder as where hammer.exe is located)\n";
		cc::reset(); std::cout << " > ";
		cc::info();  std::cout << "AutoRadar_installer.exe ";
		cc::reset(); std::cout << "\"";
		cc::info();  std::cout << "..."; 
		cc::reset(); std::cout << "Counter-Strike Global Offensive/bin/\"\n";
		exit();
	};

	std::cout << "Evaluated bin_path to: " << csgo_sdk_bin_path << "\n";

IL_PRE_INSTALL:

	/* Check files that are required for install exist */
	if (!checkFileExist((csgo_sdk_bin_path + "GameConfig.txt").c_str())){
		cc::error(); std::cout << "Required file missing: \'" << csgo_sdk_bin_path + "GameConfig.txt\'\n";
		exit();
	}

#pragma region copyfiles

	/* Start doing the heavy work */
	std::cout << "Copying files\n________________________________________________________\n\n";
	
	// Copy folders
	for (auto && folder : vinfodata->GetAllByName("folder")){
		std::string source_folder = kv::tryGetStringValue(folder->Values, "path");
		std::string target_folder = kv::tryGetStringValue(folder->Values, "target");

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

	for (auto && seqH : vinfodata->GetAllByName("CommandSequenceFile")) {
		std::string target_file = kv::tryGetStringValue(seqH->Values, "target");

		if (target_file == "") {
			cc::warning(); std::cout << "Missing target file. Skipping CommandSeq...\n";
			continue;
		}

		cc::info();

		target_file = csgo_sdk_bin_path + target_file;

		if (!checkFileExist(target_file.c_str())) {
			cc::warning(); std::cout << "CmdSeq.wc missing, command sequences won't be installed!\nCheck manual_install.txt (# Command Sequences)\n"; cc::info();
			continue;
		}

		fs::copyFile(target_file, target_file + ".bak");

		wc::filedata WcFile(target_file);

		std::cout << "Writing to: " << kv::tryGetStringValue(seqH->Values, "target") << "\n";

		for (auto && seqS : seqH->GetAllByName("Sequence")) {
			std::cout << "Installing sequence:  " << kv::tryGetStringValue(seqS->Values, "name", "error-name") << "\n";
			std::string name = kv::tryGetStringValue(seqS->Values, "name", "error-name");

			for (auto && seq : WcFile.sequences) {
				if (strstr(seq.name, name.c_str()) != NULL) {
					seq.write_enable = false;
				}
			}

			wc::Sequence seq_new = wc::Sequence();
			strcpy_s(seq_new.name, name.c_str());

			for (auto && cmd : seqS->GetAllByName("Command")){
				wc::Command command_build = wc::Command();
				command_build.is_enabled = kv::tryGetValue(cmd->Values, "is_enabled", 0);
				command_build.special = kv::tryGetValue(cmd->Values, "special", 0);
				command_build.is_long_filename = kv::tryGetValue(cmd->Values, "is_long_filename", 0);
				command_build.ensure_check = kv::tryGetValue(cmd->Values, "ensure_check", 0);
				command_build.use_proc_win = kv::tryGetValue(cmd->Values, "use_proc_win", 0);
				command_build.no_wait = kv::tryGetValue(cmd->Values, "no_wait", 0);;

				std::string executable = kv::tryGetStringValue(cmd->Values, "executable", "error-executable");

				strcpy_s(command_build.executable, executable.c_str());

				std::string args = kv::tryGetStringValue(cmd->Values, "args", "error-args");
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

		kv::DataBlock* hammerBlock = gameConfig.headNode->SubBlocks[0]->GetFirstByName("\"Games\"")->GetFirstByName("\"Counter-Strike: Global Offensive\"")->GetFirstByName("\"Hammer\"");
		int freeIndex = -1;

		for (auto && newEntry : vinfodata->GetAllByName("HammerVGDRegistry")){
			// Check if entry exists
			int i = -1;
			bool matched = false;
			while (hammerBlock->Values.count("GameData" + std::to_string(++i))) {
				if (csgo_sdk_bin_path + newEntry->Values["source"] == hammerBlock->Values["GameData" + std::to_string(i)]) {
					matched = true;
					break;
				}
			}

			if (matched) continue;

			while (hammerBlock->Values.count("GameData" + std::to_string(++freeIndex)));
			hammerBlock->Values.insert({ "GameData" + std::to_string(freeIndex), csgo_sdk_bin_path + newEntry->Values["source"]});
		}

		std::cout << "Saving GameConfig.cfg\n";
		std::ofstream out(std::string(csgo_sdk_bin_path + "GameConfig.txt").c_str());
		gameConfig.headNode->SubBlocks[0]->Serialize(out);
	}

#pragma endregion

	cc::success(); std::cout << "\nCompleted setup!\n";
	exit();
	return 0;
}