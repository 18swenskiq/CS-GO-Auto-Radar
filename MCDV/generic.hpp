#pragma once
#include <vector>

namespace bsp {
#pragma pack(push, 1)
struct lumpHeader
{
	int lumpOffset;
	int lumpLength;
	int version;
	char fourCC[4];
};
#pragma pack(pop)

/* Reads back a vector of lumps, can be used for basic lump types */
template<typename T>
std::vector<T> readLumpGeneric(std::ifstream* reader, bsp::lumpHeader lumpinfo)
{
	//Calculate lump count
	int numData = lumpinfo.lumpLength / sizeof(T);
	std::vector<T> ret;

	//Jump to that position
	reader->seekg(lumpinfo.lumpOffset);
	for (int i = 0; i < numData; i++) {
		T var;
		reader->read((char*)&var, sizeof(var));
		ret.push_back(var);
	}

	return ret;
}
}