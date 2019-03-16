#include <Windows.h>
#include <vector>
#include <string>

namespace fs
{
	std::vector<std::string> getFilesInDirectory(std::string folder) {
		std::vector<std::string> names;
		std::string search_path = folder + "/*.*";
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					names.push_back(folder + "\\" + fd.cFileName);
				}
			} while (::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
		for (auto && s : names) s = s.substr(s.size());

		return names;
	}

	std::vector<std::string> getFilesInDirectoryRecursive(std::string folder, std::vector<std::string>* vec = NULL) {
		std::vector<std::string>* v = vec;

		std::vector<std::string> names;
		if (v == NULL) // First iteration
			v = &names;

		std::string search_path = folder + "/*.*";
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					v->push_back(folder + "\\" + fd.cFileName);
				}
				else {
					if ((std::string(fd.cFileName) != ".") && (std::string(fd.cFileName) != ".."))
						getFilesInDirectoryRecursive(folder + "\\" + fd.cFileName, v);
				}
			} while (::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}

		for (auto && s : names) s = s.substr(folder.size());

		return *v;
	}

	void copyFile(std::string src, std::string dst) {
		std::ifstream _src(src, std::ios::binary);
		std::ofstream _dst(dst, std::ios::binary);

		_dst << _src.rdbuf();
	}


	bool dirExists(const std::string& dirName_in) {
		DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory!

		return false;    // this is not a directory!
	}

	std::string getDirName(std::string f) {
		return f.substr(0, f.size() - split(sutil::ReplaceAll(f, "/", "\\"), '\\').back().size());
	}
}