#pragma once

#include "Math.hpp"
#include "Memory.hpp"
#include "Renderer.hpp"

struct Texture {
	int side; // NOTE: power of two
	int logSide;
	unsigned int* memory;
};

inline Texture CopyTexture(Texture* texture) {
	Texture result = {};
	result.side = texture->side;
	result.logSide = texture->logSide;
	// TODO: use a memory arena
	result.memory = new unsigned int[result.side * result.side];

	unsigned int* resultPixel = result.memory;
	unsigned int* texturePixel = texture->memory;
	// TODO: use a memcpy here?
	for (int row = 0; row < result.side; ++row) {
		for (int col = 0; col < result.side; ++col) {
			*resultPixel = *texturePixel;
			resultPixel++;
			texturePixel++;
		}
	}

	return result;
}

inline void Swap2(unsigned int* i1, unsigned int* i2) {
	unsigned int tmp = *i1;
	*i1 = *i2;
	*i2 = tmp;
}

inline void Swap4(unsigned int* i1, unsigned int* i2, unsigned int* i3, unsigned int* i4) {
	unsigned int tmp = *i1;
	*i1 = *i2;
	*i2 = *i3;
	*i3 = *i4;
	*i4 = tmp;
}

inline unsigned int* TextureAddress(Texture* texture, int row, int col) {
	unsigned int* result = texture->memory + (row * texture->side) + col;
	return result;
}

inline unsigned int TextureValue(Texture* texture, int row, int col) {
	unsigned int result = *TextureAddress(texture, row, col);
	return result;
}

inline void RotateTextureUpsideDown(Texture* texture) {
	int side = texture->side;
	int halfSide = (side / 2);

	for (int row = 0; row < halfSide; ++row) {
		for (int col = 0; col < side; ++col) {
			unsigned int* top = TextureAddress(texture, row, col);
			unsigned int* bottom = TextureAddress(texture, side - 1 - row, side - 1 - col);
			Swap2(top, bottom);
		}
	}
}

inline void RotateTextureLeft(Texture* texture) {
	int side = texture->side;
	int halfSide = (side / 2);
	for (int row = 0; row < halfSide; ++row) {
		for (int col = 0; col < halfSide; ++col) {
			unsigned int* topLeft  = TextureAddress(texture, row, col);
			unsigned int* topRight = TextureAddress(texture, col, side - 1 - row);
			unsigned int* bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			unsigned int* bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, topRight, bottomRight, bottomLeft);
		}
	}
}

inline void RotateTextureRight(Texture* texture) {
	int side = texture->side;
	int halfSide = (side / 2);
	for (int row = 0; row < halfSide; ++row) {
		for (int col = 0; col < halfSide; ++col) {
			unsigned int* topLeft  = TextureAddress(texture, row, col);
			unsigned int* topRight = TextureAddress(texture, col, side - 1 - row);
			unsigned int* bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			unsigned int* bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, bottomLeft, bottomRight, topRight);
		}
	}
}

inline Texture RoofTexture(int logSide) {
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new unsigned int[result.side * result.side];

	int tileWidth = 16;
	int tileHeight = 32;

	unsigned int* pixel = result.memory;

	for (int row = 0; row < result.side; ++row) {
		for (int col = 0; col < result.side; ++col) {
			int red = 0;
			int green = 0;
			int blue = 0;

			int tileLeft = tileWidth * (col / tileWidth);
			int tileRight = tileLeft + tileWidth;
			int leftDist = col - tileLeft;

			int tileTop = tileHeight * (row / tileHeight);
			int topDist = row - tileTop;

			int height = IntMin2(col - tileLeft, tileRight - col);
			int tileBottom = tileTop + height;

			bool onSide = (col % tileWidth == 0);
			bool onBottom = (row == tileBottom);

			bool onTileSide = (onSide || onBottom);

			if (!onTileSide)
				red = IntRandom(100, 150);

			*pixel = (red << 16) | (green << 8) | (blue << 0);
			pixel++;
		}
	}

	return result;
}

inline Texture GrassTexture(int logSide, MemArena* tmpArena) {
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new unsigned int[result.side * result.side];

	unsigned int* pixel = result.memory;
	for (int row = 0; row < result.side; ++row) {
		for (int col = 0; col < result.side; ++col) {
			*pixel = 0;
			pixel++;
		}
	}

	float multiplier = 0.8f;
	int gridN = 2;
	float opacity = 1.0f - multiplier;
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

		int gridWidth = (result.side / gridN);

		unsigned int* pixel = result.memory;
		for (int row = 0; row < result.side; ++row) {
			int y0 = (row / gridWidth);
			int y1 = (y0 + 1);
			float yr = ((float)(row % gridWidth) / (float)gridWidth);

			for (int col = 0; col < result.side; ++col) {
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
		opacity *= multiplier;
	}

	pixel = result.memory;
	for (int row = 0; row < result.side; ++row) {
		for (int col = 0; col < result.side; ++col) {
			Color color = ColorFromCode(*pixel);
			float green = RandomBetween(color.green * 0.9f, color.green * 1.0f);
			Color newColor = Color{0.0f, green, 0.0f};
			*pixel = ColorCode(newColor);
			pixel++;
		}
	}

	return result;
}

inline Texture RandomGreyTexture(int logSide, int minRatio, int maxRatio) {
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new unsigned int[result.side * result.side];

	unsigned int* pixel = result.memory;
	for (int row = 0; row < result.side; ++row) {
		for (int col = 0; col < result.side; ++col) {
			int greyRatio = IntRandom(minRatio, maxRatio);

			*pixel = (greyRatio << 16) | (greyRatio << 8) | (greyRatio << 0); 
			pixel++;
		}
	}

	return result;
}

inline unsigned int TextureColorCodeInt(Texture texture, int row, int col) {
	unsigned int result = *(texture.memory + (row << texture.logSide) + (col));
	return result;
}

inline Color TextureColorInt(Texture texture, int row, int col) {
	int colorCode = TextureColorCodeInt(texture, row, col);
	Color result = ColorFromCode(colorCode);
	return result;
}

inline Color TextureColor(Texture texture, float x, float y) {
	int row1 = ((int)y    & (texture.side - 1));
	int row2 = (row1 + 1) & (texture.side - 1);
	int col1 = ((int)x    & (texture.side - 1));
	int col2 = (col1 + 1) & (texture.side - 1);
	float rowR = (x - Floor(x));
	float colR = (y - Floor(y));

	Color color11 = TextureColorInt(texture, row1, col1);
	Color color12 = TextureColorInt(texture, row1, col2);
	Color color21 = TextureColorInt(texture, row2, col1);
	Color color22 = TextureColorInt(texture, row2, col2);

	Color color1 = ColorLerp(color11, rowR, color12);
	Color color2 = ColorLerp(color21, rowR, color22);

	Color color = ColorLerp(color1, colR, color2);

	return color;
}

inline unsigned int ColorCodeLerp(unsigned int colorCode1, unsigned char ratio, unsigned int colorCode2) {
	struct ColorCode {
		union {
			unsigned int u;
			struct {
				unsigned char r, g, b;
			};
		};
	};

	ColorCode code1 = {};
	ColorCode code2 = {};
	code1.u = colorCode1;
	code2.u = colorCode2;

	unsigned char r1 = (255 - ratio);
	unsigned char r2 = ratio;

	ColorCode result = {};
	/*
	result.r = (unsigned char)((code1.r * r1) + (code2.r * r2));
	result.g = (unsigned char)((code1.g * r1) + (code2.g * r2));
	result.b = (unsigned char)((code1.b * r1) + (code2.b * r2));
	*/
	result.r = (unsigned char)((((unsigned short)code1.r * r1) + (unsigned short)(code2.r * r2)) >> 8);
	result.g = (unsigned char)((((unsigned short)code1.g * r1) + (unsigned short)(code2.g * r2)) >> 8);
	result.b = (unsigned char)((((unsigned short)code1.b * r1) + (unsigned short)(code2.b * r2)) >> 8);

	return result.u;
}

inline unsigned int TextureColorCode(Texture texture, int x, unsigned char subx, int y, unsigned char suby) {
	int row1 = y;
	int row2 = (row1 + 1) & (texture.side - 1);
	int col1 = x;
	int col2 = (col1 + 1) & (texture.side - 1);

	unsigned int code11 = TextureColorCodeInt(texture, row1, col1);
	unsigned int code12 = TextureColorCodeInt(texture, row1, col2);
	unsigned int code1 = ColorCodeLerp(code11, subx, code12);

	unsigned int code21 = TextureColorCodeInt(texture, row2, col1);
	unsigned int code22 = TextureColorCodeInt(texture, row2, col2);
	unsigned int code2 = ColorCodeLerp(code21, subx, code22);

	unsigned int code = ColorCodeLerp(code1, suby, code2);
	return code;
}