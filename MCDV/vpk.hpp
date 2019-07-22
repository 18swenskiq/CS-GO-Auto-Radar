#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "util.h"

#pragma pack(push, 1)
namespace vpk
{
	std::string get_sz(std::ifstream* stream) {
		std::string out = "";
		while (true) {
			char c;
			stream->read(&c, 1);
			if (c == 0x00) break;
			out += c;
		}
		return out;
	}

	struct Header_v2
	{
		unsigned int Signature = 0x55aa1234;
		unsigned int Version = 2;

		// The size, in bytes, of the directory tree
		unsigned int TreeSize;

		// How many bytes of file content are stored in this VPK file (0 in CSGO)
		unsigned int FileDataSectionSize;

		// The size, in bytes, of the section containing MD5 checksums for external archive content
		unsigned int ArchiveMD5SectionSize;

		// The size, in bytes, of the section containing MD5 checksums for content in this file (should always be 48)
		unsigned int OtherMD5SectionSize;

		// The size, in bytes, of the section containing the public key and signature. This is either 0 (CSGO & The Ship) or 296 (HL2, HL2:DM, HL2:EP1, HL2:EP2, HL2:LC, TF2, DOD:S & CS:S)
		unsigned int SignatureSectionSize;
	};

	struct VPKDirectoryEntry
	{
		unsigned int CRC; // A 32bit CRC of the file's data.
		unsigned short PreloadBytes; // The number of bytes contained in the index file.

									 // A zero based index of the archive this file's data is contained in.
									 // If 0x7fff, the data follows the directory.
		unsigned short ArchiveIndex;

		// If ArchiveIndex is 0x7fff, the offset of the file data relative to the end of the directory (see the header for more details).
		// Otherwise, the offset of the data from the start of the specified archive.
		unsigned int EntryOffset;

		// If zero, the entire file is stored in the preload data.
		// Otherwise, the number of bytes stored starting at EntryOffset.
		unsigned int EntryLength;

		unsigned short Terminator = 0xffff;
	};

	struct vEntry
	{
		VPKDirectoryEntry entryInfo;
		std::string entryString;
	};

	class index {
	public:
		Header_v2 header;
		std::vector<vEntry> entries;

		index(std::string path) {
			//Create main file handle
			std::ifstream reader(path, std::ios::in | std::ios::binary);

			if (!reader) {
				throw std::exception("VPK::LOAD Failed"); return;
			}

			//Read header
			reader.read((char*)&this->header, sizeof(this->header));

			std::cout << "Version: " << this->header.Version << "\n";
			std::cout << "TreeSize: " << this->header.TreeSize << "\n";

			//std::ofstream f;
			//f.open("vpk.txt");

			while (true) {
				std::string extension = get_sz(&reader);
				std::cout << "   *." << extension << "\n";
				if (extension == "") break;

				while (true) {
					std::string folder = get_sz(&reader);
					if (folder == "") break;

					while (true) {
						std::string filename = get_sz(&reader);
						if (filename == "") break;

						if(this->header.Version > 0)
						if (reader.tellg() > this->header.TreeSize + sizeof(Header_v2)) goto IL_EXIT; // Get out of f

						vEntry entry;
						reader.read((char*)&entry.entryInfo, sizeof(VPKDirectoryEntry));

						if (entry.entryInfo.PreloadBytes) {
							char* arr = new char[entry.entryInfo.PreloadBytes];
							reader.read(arr, entry.entryInfo.PreloadBytes);
							delete arr;
						}

						entry.entryString = folder + "/" + filename + "." + extension;
						//f << folder + "/" + filename + "." + extension << "\n";

						this->entries.push_back(entry);
					}
				}
			}

		IL_EXIT:

			//f.close();

			std::cout << "Done reading\n";
			std::cout << this->entries.size() << " entries read\n";

			reader.close();
		}

		vEntry* find(std::string name) {
			// All files in vpk are stored in lowercase.
			std::string search = sutil::to_lower(name);
			for (auto && v : this->entries) {
				if (v.entryString == search) {
					return &v;
				}
			}

			return NULL;
		}
	};
}
#pragma pack(pop)