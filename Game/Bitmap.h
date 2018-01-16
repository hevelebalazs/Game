#pragma once

#include <windows.h>
#include "Math.h"
#include "Point.h"

struct Color {
	float red;
	float green;
	float blue;
};

struct Bitmap {
	int width;
	int height;
	void* memory;
	// TODO: get rid of this Windows specific thing
	BITMAPINFO info;
};

struct Texture {
	int width;
	int height;
	unsigned int* memory;
};

#define BitmapBytesPerPixel 4

Color RandomColor();
inline Texture RandomGreyTexture(int width, int height, int minRatio, int maxRatio) {
	Texture result = {};
	result.width = width;
	result.height = height;
	// TODO: use a memory arena
	result.memory = new unsigned int[width * height];

	unsigned int* pixel = result.memory;
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			int greyRatio = IntRandom(minRatio, maxRatio);

			*pixel = (greyRatio << 16) | (greyRatio << 8) | (greyRatio << 0); 
			pixel++;
		}
	}

	return result;
}

inline unsigned int TextureColor(Texture texture, int row, int col) {
	// unsigned int result = *(texture.memory + row * texture.width + col);
	// TODO: it is assumed that texture.width is 256 and texture.height is 256
	unsigned int result = *(texture.memory + (row << 8) + (col));
	return result;
}