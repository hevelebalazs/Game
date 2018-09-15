#pragma once

#include "Draw.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Texture 
{
	I32 side; // NOTE: power of two
	I32 logSide;
	U32* memory;
};

inline Texture CopyTexture(Texture* texture) 
{
	Texture result = {};
	result.side = texture->side;
	result.logSide = texture->logSide;
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32* resultPixel = result.memory;
	U32* texturePixel = texture->memory;
	// TODO: use a memcpy here?
	for (I32 row = 0; row < result.side; ++row) 
	{
		for (I32 col = 0; col < result.side; ++col) 
		{
			*resultPixel = *texturePixel;
			resultPixel++;
			texturePixel++;
		}
	}

	return result;
}

inline void Swap2(U32* i1, U32* i2)
{
	U32 tmp = *i1;
	*i1 = *i2;
	*i2 = tmp;
}

inline void Swap4(U32* i1, U32* i2, U32* i3, U32* i4)
{
	U32 tmp = *i1;
	*i1 = *i2;
	*i2 = *i3;
	*i3 = *i4;
	*i4 = tmp;
}

inline U32* TextureAddress(Texture* texture, I32 row, I32 col)
{
	U32* result = texture->memory + (row * texture->side) + col;
	return result;
}

inline U32 TextureValue(Texture* texture, I32 row, I32 col)
{
	U32 result = *TextureAddress(texture, row, col);
	return result;
}

inline void RotateTextureUpsideDown(Texture* texture)
{
	I32 side = texture->side;
	I32 halfSide = (side / 2);

	for (I32 row = 0; row < halfSide; ++row) 
	{
		for (I32 col = 0; col < side; ++col) 
		{
			U32* top = TextureAddress(texture, row, col);
			U32* bottom = TextureAddress(texture, side - 1 - row, side - 1 - col);
			Swap2(top, bottom);
		}
	}
}

inline void RotateTextureLeft(Texture* texture)
{
	I32 side = texture->side;
	I32 halfSide = (side / 2);
	for (I32 row = 0; row < halfSide; ++row) 
	{
		for (I32 col = 0; col < halfSide; ++col) 
		{
			U32* topLeft  = TextureAddress(texture, row, col);
			U32* topRight = TextureAddress(texture, col, side - 1 - row);
			U32* bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			U32* bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, topRight, bottomRight, bottomLeft);
		}
	}
}

inline void RotateTextureRight(Texture* texture)
{
	I32 side = texture->side;
	I32 halfSide = (side / 2);
	for (I32 row = 0; row < halfSide; ++row) 
	{
		for (I32 col = 0; col < halfSide; ++col) 
		{
			U32* topLeft  = TextureAddress(texture, row, col);
			U32* topRight = TextureAddress(texture, col, side - 1 - row);
			U32* bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			U32* bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, bottomLeft, bottomRight, topRight);
		}
	}
}

inline Texture RoofTexture(I32 logSide) 
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	I32 tileWidth = 16;
	I32 tileHeight = 32;

	U32* pixel = result.memory;

	for (I32 row = 0; row < result.side; ++row) 
	{
		for (I32 col = 0; col < result.side; ++col) 
		{
			I32 red = 0;
			I32 green = 0;
			I32 blue = 0;

			I32 tileLeft = tileWidth * (col / tileWidth);
			I32 tileRight = tileLeft + tileWidth;
			I32 leftDist = col - tileLeft;

			I32 tileTop = tileHeight * (row / tileHeight);
			I32 topDist = row - tileTop;

			I32 height = IntMin2(col - tileLeft, tileRight - col);
			I32 tileBottom = tileTop + height;

			B32 onSide = (col % tileWidth == 0);
			B32 onBottom = (row == tileBottom);

			B32 onTileSide = (onSide || onBottom);

			if (!onTileSide)
				red = IntRandom(100, 150);

			*pixel = (red << 16) | (green << 8) | (blue << 0);
			pixel++;
		}
	}

	return result;
}

inline Texture GrassTexture(I32 logSide, MemArena* tmpArena)
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32* pixel = result.memory;
	for (I32 row = 0; row < result.side; ++row) 
	{
		for (I32 col = 0; col < result.side; ++col) 
		{
			*pixel = 0;
			pixel++;
		}
	}

	F32 multiplier = 0.8f;
	I32 gridN = 2;
	F32 opacity = 1.0f - multiplier;
	while (gridN <= 256) 
	{
		F32* gridValues = ArenaPushArray(tmpArena, F32, (gridN + 1) * (gridN + 1));
		F32* gridValue = gridValues;
		for (I32 row = 0; row <= gridN; ++row) 
		{
			for (I32 col = 0; col <= gridN; ++col) 
			{
				if (row == gridN)
					*gridValue = *(gridValues + (gridN + 1) * 0 + col);
				else if (col == gridN)
					*gridValue = *(gridValues + (gridN + 1) * row + 0);
				else
					*gridValue = RandomBetween(0.0f, opacity);

				gridValue++;
			}
		}

		I32 gridWidth = (result.side / gridN);

		U32* pixel = result.memory;
		for (I32 row = 0; row < result.side; ++row) 
		{
			I32 y0 = (row / gridWidth);
			I32 y1 = (y0 + 1);
			F32 yr = ((F32)(row % gridWidth) / (F32)gridWidth);

			for (I32 col = 0; col < result.side; ++col) 
			{
				I32 x0 = (col / gridWidth);
				I32 x1 = (x0 + 1);
				F32 xr = ((F32)(col % gridWidth) / (F32)gridWidth);

				F32 topLeft     = *(gridValues + y0 * (gridN + 1) + x0);
				F32 topRight    = *(gridValues + y0 * (gridN + 1) + x1);
				F32 bottomLeft  = *(gridValues + y1 * (gridN + 1) + x0);
				F32 bottomRight = *(gridValues + y1 * (gridN + 1) + x1);

				F32 top    = Lerp(topLeft, xr, topRight);
				F32 bottom = Lerp(bottomLeft, xr, bottomRight);
				F32 value  = Lerp(top, yr, bottom);

				V4 oldColor = GetColorFromColorCode(*pixel);
				V4 newColor = MakeColor(0.0f, value, 0.0f);

				V4 color = AddColors(oldColor, newColor);
				*pixel = GetColorCode(color);
				pixel++;
			}
		}

		ArenaPopTo(tmpArena, gridValues);

		gridN *= 2;
		opacity *= multiplier;
	}

	pixel = result.memory;
	for (I32 row = 0; row < result.side; ++row) 
	{
		for (I32 col = 0; col < result.side; ++col) 
		{
			V4 color = GetColorFromColorCode(*pixel);
			F32 green = RandomBetween(color.green * 0.9f, color.green * 1.0f);
			V4 newColor = MakeColor(0.0f, green, 0.0f);
			*pixel = GetColorCode(newColor);
			pixel++;
		}
	}

	return result;
}

inline Texture RandomGreyTexture(I32 logSide, I32 minRatio, I32 maxRatio)
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32* pixel = result.memory;
	for (I32 row = 0; row < result.side; ++row) 
	{
		for (I32 col = 0; col < result.side; ++col) 
		{
			I32 greyRatio = IntRandom(minRatio, maxRatio);

			*pixel = (greyRatio << 16) | (greyRatio << 8) | (greyRatio << 0); 
			pixel++;
		}
	}

	return result;
}

inline U32 TextureColorCodeInt(Texture texture, I32 row, I32 col)
{
	U32 result = *(texture.memory + (row << texture.logSide) + (col));
	return result;
}

inline V4 TextureColorInt(Texture texture, I32 row, I32 col)
{
	I32 colorCode = TextureColorCodeInt(texture, row, col);
	V4 result = GetColorFromColorCode(colorCode);
	return result;
}

inline V4 TextureColor(Texture texture, F32 x, F32 y)
{
	I32 row = ((I32)y & (texture.side - 1));
	I32 col = ((I32)x & (texture.side - 1));
	V4 color = TextureColorInt(texture, row, col);
	return color;
}

inline U32 TextureColorCode(Texture texture, F32 x, F32 y)
{
	V4 color = TextureColor(texture, x, y);
	U32 colorCode = GetColorCode(color);
	return colorCode;
}

inline U32 ColorCodeLerp(U32 colorCode1, U8 ratio, U32 colorCode2)
{
	struct ColorCode 
	{
		union 
		{
			U32 u;
			struct 
			{
				U8 r, g, b;
			};
		};
	};

	ColorCode code1 = {};
	ColorCode code2 = {};
	code1.u = colorCode1;
	code2.u = colorCode2;

	U8 r1 = (255 - ratio);
	U8 r2 = ratio;

	ColorCode result = {};
	/*
	result.r = (U8)((code1.r * r1) + (code2.r * r2));
	result.g = (U8)((code1.g * r1) + (code2.g * r2));
	result.b = (U8)((code1.b * r1) + (code2.b * r2));
	*/
	result.r = (U8)((((U16)code1.r * r1) + (U16)(code2.r * r2)) >> 8);
	result.g = (U8)((((U16)code1.g * r1) + (U16)(code2.g * r2)) >> 8);
	result.b = (U8)((((U16)code1.b * r1) + (U16)(code2.b * r2)) >> 8);

	return result.u;
}

inline U32 TextureColorCode(Texture texture, I32 x, U8 subx, I32 y, U8 suby)
{
	I32 row1 = y;
	I32 row2 = (row1 + 1) & (texture.side - 1);
	I32 col1 = x;
	I32 col2 = (col1 + 1) & (texture.side - 1);

	U32 code11 = TextureColorCodeInt(texture, row1, col1);
	U32 code12 = TextureColorCodeInt(texture, row1, col2);
	U32 code1 = ColorCodeLerp(code11, subx, code12);

	U32 code21 = TextureColorCodeInt(texture, row2, col1);
	U32 code22 = TextureColorCodeInt(texture, row2, col2);
	U32 code2 = ColorCodeLerp(code21, subx, code22);

	U32 code = ColorCodeLerp(code1, suby, code2);
	return code;
}