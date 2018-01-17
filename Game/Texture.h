#pragma once

#include "Math.h"
#include "Memory.h"
#include "Renderer.h"

struct Texture {
	int width;
	int height;
	unsigned int* memory;
};

inline Texture GrassTexture(int width, int height, MemArena* tmpArena) {
	Texture result = {};
	result.width = width;
	result.height = height;
	// TODO: use a memory arena
	result.memory = new unsigned int[width * height];

	Color baseColor = Color{0.0f, 0.5f, 0.0f};
	unsigned int baseCode = ColorCode(baseColor);

	unsigned int* pixel = result.memory;
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			*pixel = baseCode;
			pixel++;
		}
	}

	int gridN = 16;
	float opacity = 0.25f;
	while (gridN <= 256) {
		float* gridValues = ArenaPushArray(tmpArena, float, (gridN + 1) * (gridN + 1));
		float* gridValue = gridValues;
		for (int row = 0; row <= gridN; ++row) {
			for (int col = 0; col <= gridN; ++col) {
				if (row == gridN)
					*gridValue = *(gridValues + (gridN + 1) * 0 + col);
				else if (col == gridN)
					*gridValue = *(gridValues + (gridN + 1) * row + 0);
				else
					*gridValue = RandomBetween(0.0f, opacity);

				gridValue++;
			}
		}

		// TODO: it is assumed that width == height here
		//       is it always the case?
		//       if it is, pass only one parameter
		int gridWidth = (width / gridN);

		unsigned int* pixel = result.memory;
		for (int row = 0; row < height; ++row) {
			int y0 = (row / gridWidth);
			int y1 = (y0 + 1);
			float yr = ((float)(row % gridWidth) / (float)gridWidth);

			for (int col = 0; col < width; ++col) {
				int x0 = (col / gridWidth);
				int x1 = (x0 + 1);
				float xr = ((float)(col % gridWidth) / (float)gridWidth);

				float topLeft     = *(gridValues + y0 * (gridN + 1) + x0);
				float topRight    = *(gridValues + y0 * (gridN + 1) + x1);
				float bottomLeft  = *(gridValues + y1 * (gridN + 1) + x0);
				float bottomRight = *(gridValues + y1 * (gridN + 1) + x1);

				float top    = Lerp(topLeft, xr, topRight);
				float bottom = Lerp(bottomLeft, xr, bottomRight);
				float value  = Lerp(top, yr, bottom);

				Color oldColor = ColorFromCode(*pixel);
				Color newColor = Color{0.0f, value, 0.0f};

				Color color = ColorAdd(oldColor, newColor);
				*pixel = ColorCode(color);
				pixel++;
			}
		}

		ArenaPopTo(tmpArena, gridValues);

		gridN *= 2;
		opacity *= 0.5f;
	}

	return result;
}

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
	// unsigned int resultCode = *(texture.memory + row * texture.width + col);
	// TODO: it is assumed that texture.width is 256 and texture.height is 256
	unsigned int result = *(texture.memory + (row << 8) + (col));
	return result;
}