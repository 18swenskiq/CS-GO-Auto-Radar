#include <stdint.h>
#include <stdlib.h>
#include <string.h> 

#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

#define __max(a,b)            (((a) > (b)) ? (a) : (b))
#define __min(a,b)            (((a) < (b)) ? (a) : (b))
#pragma once

#pragma pack(push, 1)
struct DDS_PIXELFORMAT {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

typedef struct {
	uint32_t           dwSize;
	uint32_t           dwFlags;
	uint32_t           dwHeight;
	uint32_t           dwWidth;
	uint32_t           dwPitchOrLinearSize;
	uint32_t           dwDepth;
	uint32_t           dwMipMapCount;
	uint32_t           dwReserved1[11];
	DDS_PIXELFORMAT	   ddspf;
	uint32_t           dwCaps;
	uint32_t           dwCaps2;
	uint32_t           dwCaps3;
	uint32_t           dwCaps4;
	uint32_t           dwReserved2;
} DDS_HEADER;
#pragma pack(pop)

enum IMG {
	MODE_RGB888,
	MODE_RGBA8888,
	MODE_DXT1,
	MODE_DXT5
};

UINT32 SwapEndian(UINT32 val)
{
	return (val << 24) | ((val << 8) & 0x00ff0000) |
		((val >> 8) & 0x0000ff00) | (val >> 24);
}

#define DDSD_CAPS 0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH 0x4
#define DDSD_PITCH 0x8
#define DDSD_PIXELFORMAT 0x1000
#define DDSD_MIPMAPCOUNT 0x20000
#define DDSD_LINEARSIZE 0x80000
#define DDSD_DEPTH 0x800000

#define DDPF_ALPHAPIXELS 0x1
#define DDPF_ALPHA 0x2
#define DDPF_FOURCC 0x4
#define DDPF_RGB 0x40
#define DDPF_YUV 0x200
#define DDPF_LUMINANCE 0x20000

#define DDSCAPS_COMPLEX 0x8
#define DDSCAPS_MIPMAP 0x400000
#define DDSCAPS_TEXTURE 0x1000

#define BLOCK_SIZE_DXT1 8
#define BLOCK_SIZE_DXT5 16

#define BBP_RGB888 24
#define BBP_RGBA8888 32

#define DDS_HEADER_SIZE 124
#define DDS_HEADER_PFSIZE 32
#define DDS_MAGICNUM 0x20534444;

/*
imageData:	Pointer to image data
compressedSize: Pointer to final data size
w: image width
h: image height
mode: compression mode to use
*/
uint8_t* compressImageDXT1(uint8_t* buf_RGB, uint32_t w, uint32_t h, uint32_t* cSize) {
	*cSize = ((w / 4) * (h / 4)) * BLOCK_SIZE_DXT1;

	//Create output buffer
	uint8_t* outBuffer = (uint8_t*)malloc(*cSize);

	int blocks_x = w / 4;
	int blocks_y = h / 4;

	std::cout << "Compressing DXT1 from RGB buffer\n";

	//Fill
	for (int y = 0; y < blocks_y; y++){
		for (int x = 0; x < blocks_x; x++){

			int blockindex = x + (y * blocks_x);
			int globalX = x * 4;
			int globalY = y * 4;
			
			uint8_t* src = new uint8_t[64]; //Create source RGBA buffer
			for (int _y = 0; _y < 4; _y++) {
				for (int _x = 0; _x < 4; _x++) {
					src[(_x + (_y * 4)) * 4 + 0] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 0];
					src[(_x + (_y * 4)) * 4 + 1] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 1];
					src[(_x + (_y * 4)) * 4 + 2] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 2];
					src[(_x + (_y * 4)) * 4 + 3] = 0xFF;
				}
			}

			stb_compress_dxt_block((unsigned char*)outBuffer + (blockindex * BLOCK_SIZE_DXT1), src, 0, STB_DXT_HIGHQUAL);

			free(src);
		}
	}

	return outBuffer;
}

/*
imageData:	Pointer to image data
compressedSize: Pointer to final data size
w: image width
h: image height
mode: compression mode to use
*/
uint8_t* compressImageDXT5(uint8_t* buf_RGB, uint32_t w, uint32_t h, uint32_t* cSize) {
	*cSize = ((w / 4) * (h / 4)) * BLOCK_SIZE_DXT5;

	//Create output buffer
	uint8_t* outBuffer = (uint8_t*)malloc(*cSize);

	int blocks_x = w / 4;
	int blocks_y = h / 4;

	std::cout << "Compressing DXT1 from RGB buffer\n";

	//Fill
	for (int y = 0; y < blocks_y; y++) {
		for (int x = 0; x < blocks_x; x++) {

			int blockindex = x + (y * blocks_x);
			int globalX = x * 4;
			int globalY = y * 4;

			uint8_t* src = new uint8_t[64]; //Create source RGBA buffer
			for (int _y = 0; _y < 4; _y++) {
				for (int _x = 0; _x < 4; _x++) {
					src[(_x + (_y * 4)) * 4 + 0] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 0];
					src[(_x + (_y * 4)) * 4 + 1] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 1];
					src[(_x + (_y * 4)) * 4 + 2] = buf_RGB[(globalX + _x + ((h - (globalY + _y)) * w)) * 3 + 2];
					src[(_x + (_y * 4)) * 4 + 3] = 0xFF;
				}
			}

			stb_compress_dxt_block((unsigned char*)outBuffer + (blockindex * BLOCK_SIZE_DXT5), src, 1, STB_DXT_HIGHQUAL);

			free(src);
		}
	}

	return outBuffer;
}

bool dds_write(uint8_t* imageData, const char* filename, uint32_t w, uint32_t h, IMG mode) {
	DDS_HEADER header = DDS_HEADER();
	header.dwSize = DDS_HEADER_SIZE;
	header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	header.dwHeight = h;
	header.dwWidth = w;

	header.ddspf.dwSize = DDS_HEADER_PFSIZE;

	int final_image_size = 0;

	switch (mode) {
	case IMG::MODE_DXT1:
		header.dwPitchOrLinearSize = SwapEndian(__max(1, ((w + 3) / 4)) * BLOCK_SIZE_DXT1);
		header.ddspf.dwFlags |= DDPF_FOURCC;
		header.ddspf.dwFourCC = SwapEndian((uint32_t)'DXT1');
		header.dwFlags |= DDSD_LINEARSIZE;
		
		break;
	case IMG::MODE_DXT5:
		header.dwPitchOrLinearSize = SwapEndian(__max(1, ((w + 3) / 4)) * BLOCK_SIZE_DXT5);
		header.ddspf.dwFlags |= DDPF_FOURCC;
		header.ddspf.dwFlags |= DDPF_ALPHA;
		header.ddspf.dwFourCC = SwapEndian((uint32_t)'DXT5');
		header.dwFlags |= DDSD_LINEARSIZE;

		break;
	case IMG::MODE_RGB888:
		header.dwPitchOrLinearSize = w * (BBP_RGB888 / 8);
		final_image_size = w * h * (BBP_RGB888 / 8);
		
		header.ddspf.dwFlags |= DDPF_RGB;
		header.dwFlags |= DDSD_PITCH;
		header.ddspf.dwRGBBitCount = BBP_RGB888;
		header.ddspf.dwRBitMask = SwapEndian(0xff000000);
		header.ddspf.dwGBitMask = SwapEndian(0x00ff0000);
		header.ddspf.dwBBitMask = SwapEndian(0x0000ff00);
		break;
	case IMG::MODE_RGBA8888:
		header.dwPitchOrLinearSize = w * (BBP_RGBA8888 / 8);
		final_image_size = w * h * (BBP_RGBA8888 / 8);
		
		header.ddspf.dwFlags |= DDPF_RGB;
		header.dwFlags |= DDSD_PITCH;
		header.ddspf.dwFlags |= DDPF_ALPHA;
		header.ddspf.dwRGBBitCount = BBP_RGBA8888;
		header.ddspf.dwRBitMask = SwapEndian(0xff000000);
		header.ddspf.dwGBitMask = SwapEndian(0x00ff0000);
		header.ddspf.dwBBitMask = SwapEndian(0x0000ff00);
		header.ddspf.dwABitMask = SwapEndian(0x000000ff);
		throw new std::exception("RGBA8888 Not implemented");
		break;
	
	default: return false; //Mode not supported
	}

	header.dwMipMapCount = 0;
	header.dwCaps = DDSCAPS_TEXTURE;

	// Magic number
	uint32_t magic = DDS_MAGICNUM;
	std::fstream output;
	output.open(filename, std::ios::out | std::ios::binary);

	output.write((char*)&magic, sizeof(uint32_t));
	output.write((char*)&header, DDS_HEADER_SIZE);

	if (mode == IMG::MODE_DXT1)
	{
		uint32_t size;
		uint8_t* outputBuffer = compressImageDXT1(imageData, w, h, &size);
		output.write((char*)outputBuffer, size);
	}
	else if (mode == IMG::MODE_DXT5) {
		uint32_t size;
		uint8_t* outputBuffer = compressImageDXT5(imageData, w, h, &size);
		output.write((char*)outputBuffer, size);
	}
	else
	{
		output.write((char*)imageData, final_image_size);
	}

	output.close();
	return true;
}