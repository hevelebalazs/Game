#pragma once

#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Texture 
{
	Int32 side; // NOTE: power of two
	Int32 logSide;
	UInt32 *memory;
};

static Texture
func CopyTexture(Texture *texture) 
{
	Texture result = {};
	result.side = texture->side;
	result.logSide = texture->logSide;
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	UInt32 *resultPixel = result.memory;
	UInt32 *texturePixel = texture->memory;
	// TODO: use a memcpy here?
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			*resultPixel = *texturePixel;
			resultPixel++;
			texturePixel++;
		}
	}

	return result;
}

static void
func Swap2(UInt32 *i1, UInt32 *i2)
{
	UInt32 tmp = *i1;
	*i1 = *i2;
	*i2 = tmp;
}

static void
func Swap4(UInt32 *i1, UInt32 *i2, UInt32 *i3, UInt32 *i4)
{
	UInt32 tmp = *i1;
	*i1 = *i2;
	*i2 = *i3;
	*i3 = *i4;
	*i4 = tmp;
}

static UInt32 *
func TextureAddress(Texture *texture, Int32 row, Int32 col)
{
	UInt32 *result = texture->memory + (row * texture->side) + col;
	return result;
}

static UInt32
func TextureValue(Texture *texture, Int32 row, Int32 col)
{
	UInt32 result = *TextureAddress(texture, row, col);
	return result;
}

static void
func RotateTextureUpsideDown(Texture *texture)
{
	Int32 side = texture->side;
	Int32 halfSide = (side / 2);

	for(Int32 row = 0; row < halfSide; row++) 
	{
		for(Int32 col = 0; col < side; col++) 
		{
			UInt32 *top = TextureAddress(texture, row, col);
			UInt32 *bottom = TextureAddress(texture, side - 1 - row, side - 1 - col);
			Swap2(top, bottom);
		}
	}
}

static void
func RotateTextureLeft(Texture *texture)
{
	Int32 side = texture->side;
	Int32 halfSide = (side / 2);
	for(Int32 row = 0; row < halfSide; row++) 
	{
		for(Int32 col = 0; col < halfSide; col++) 
		{
			UInt32 *topLeft  = TextureAddress(texture, row, col);
			UInt32 *topRight = TextureAddress(texture, col, side - 1 - row);
			UInt32 *bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			UInt32 *bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, topRight, bottomRight, bottomLeft);
		}
	}
}

static void
func RotateTextureRight(Texture *texture)
{
	Int32 side = texture->side;
	Int32 halfSide = (side / 2);
	for(Int32 row = 0; row < halfSide; row++) 
	{
		for(Int32 col = 0; col < halfSide; col++) 
		{
			UInt32 *topLeft  = TextureAddress(texture, row, col);
			UInt32 *topRight = TextureAddress(texture, col, side - 1 - row);
			UInt32 *bottomRight = TextureAddress(texture, side - 1 - row, side - 1 - col);
			UInt32 *bottomLeft = TextureAddress(texture, side - 1 - col, row);

			Swap4(topLeft, bottomLeft, bottomRight, topRight);
		}
	}
}

static Texture
func RoofTexture(Int32 logSide) 
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	Int32 tileWidth = 16;
	Int32 tileHeight = 32;

	UInt32* pixel = result.memory;

	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Int32 red = 0;
			Int32 green = 0;
			Int32 blue = 0;

			Int32 tileLeft = tileWidth * (col / tileWidth);
			Int32 tileRight = tileLeft + tileWidth;
			Int32 leftDist = col - tileLeft;

			Int32 tileTop = tileHeight * (row / tileHeight);
			Int32 topDist = row - tileTop;

			Int32 height = IntMin2(col - tileLeft, tileRight - col);
			Int32 tileBottom = tileTop + height;

			Bool32 onSide = (col % tileWidth == 0);
			Bool32 onBottom = (row == tileBottom);

			Bool32 onTileSide = (onSide || onBottom);

			if(!onTileSide)
			{
				red = IntRandom(100, 150);
			}

			*pixel = (red << 16) | (green << 8) | (blue << 0);
			pixel++;
		}
	}

	return result;
}

static Texture
func GrassTexture(Int32 logSide, MemArena *tmpArena)
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	UInt32 *pixel = result.memory;
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			*pixel = 0;
			pixel++;
		}
	}

	Real32 multiplier = 0.8f;
	Int32 gridN = 2;
	Real32 opacity = 1.0f - multiplier;
	while(gridN <= 256) 
	{
		Real32 *gridValues = ArenaAllocArray(tmpArena, Real32, (gridN + 1) * (gridN + 1));
		Real32 *gridValue = gridValues;
		for(Int32 row = 0; row <= gridN; row++) 
		{
			for(Int32 col = 0; col <= gridN; col++) 
			{
				if(row == gridN)
				{
					*gridValue = *(gridValues + (gridN + 1) * 0 + col);
				}
				else if(col == gridN)
				{
					*gridValue = *(gridValues + (gridN + 1) * row + 0);
				}
				else
				{
					*gridValue = RandomBetween(0.0f, opacity);
				}

				gridValue++;
			}
		}

		Int32 gridWidth = (result.side / gridN);

		UInt32 *pixel = result.memory;
		for(Int32 row = 0; row < result.side; row++) 
		{
			Int32 y0 = (row / gridWidth);
			Int32 y1 = (y0 + 1);
			Real32 yr = ((Real32)(row % gridWidth) / (Real32)gridWidth);

			for(Int32 col = 0; col < result.side; col++) 
			{
				Int32 x0 = (col / gridWidth);
				Int32 x1 = (x0 + 1);
				Real32 xr = ((Real32)(col % gridWidth) / (Real32)gridWidth);

				Real32 topLeft     = *(gridValues + y0 * (gridN + 1) + x0);
				Real32 topRight    = *(gridValues + y0 * (gridN + 1) + x1);
				Real32 bottomLeft  = *(gridValues + y1 * (gridN + 1) + x0);
				Real32 bottomRight = *(gridValues + y1 * (gridN + 1) + x1);

				Real32 top    = Lerp(topLeft, xr, topRight);
				Real32 bottom = Lerp(bottomLeft, xr, bottomRight);
				Real32 value  = Lerp(top, yr, bottom);

				Vec4 oldColor = GetColorFromColorCode(*pixel);
				Vec4 newColor = MakeColor(0.0f, value, 0.0f);

				Vec4 color = AddColors(oldColor, newColor);
				*pixel = GetColorCode(color);
				pixel++;
			}
		}

		ArenaPopTo(tmpArena, gridValues);

		gridN *= 2;
		opacity *= multiplier;
	}

	pixel = result.memory;
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Vec4 color = GetColorFromColorCode(*pixel);
			Real32 green = RandomBetween(color.green * 0.9f, color.green * 1.0f);
			Vec4 newColor = MakeColor(0.0f, green, 0.0f);
			*pixel = GetColorCode(newColor);
			pixel++;
		}
	}

	return result;
}

static Texture
func RandomGreyTexture(Int32 logSide, Int32 minRatio, Int32 maxRatio)
{
	Texture result = {};
	result.logSide = logSide;
	result.side = (1 << logSide);
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	UInt32 *pixel = result.memory;
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Int32 greyRatio = IntRandom(minRatio, maxRatio);

			*pixel = (greyRatio << 16) | (greyRatio << 8) | (greyRatio << 0); 
			pixel++;
		}
	}

	return result;
}

static UInt32
func TextureColorCodeInt(Texture texture, Int32 row, Int32 col)
{
	UInt32 result = *(texture.memory + (row << texture.logSide) + (col));
	return result;
}

static Vec4
func TextureColorInt(Texture texture, Int32 row, Int32 col)
{
	Int32 colorCode = TextureColorCodeInt(texture, row, col);
	Vec4 result = GetColorFromColorCode(colorCode);
	return result;
}

static Vec4
func TextureColor(Texture texture, Real32 x, Real32 y)
{
	Int32 row = ((Int32)y & (texture.side - 1));
	Int32 col = ((Int32)x & (texture.side - 1));
	Vec4 color = TextureColorInt(texture, row, col);
	return color;
}

static UInt32
func TextureColorCode(Texture texture, Real32 x, Real32 y)
{
	Vec4 color = TextureColor(texture, x, y);
	UInt32 colorCode = GetColorCode(color);
	return colorCode;
}

static UInt32
func ColorCodeLerp(UInt32 colorCode1, UInt8 ratio, UInt32 colorCode2)
{
	struct ColorCode 
	{
		union 
		{
			UInt32 u;
			struct 
			{
				UInt8 r, g, b;
			};
		};
	};

	ColorCode code1 = {};
	ColorCode code2 = {};
	code1.u = colorCode1;
	code2.u = colorCode2;

	UInt8 r1 = (255 - ratio);
	UInt8 r2 = ratio;

	ColorCode result = {};
	/*
	result.r = (UInt8)((code1.r * r1) + (code2.r * r2));
	result.g = (UInt8)((code1.g * r1) + (code2.g * r2));
	result.b = (UInt8)((code1.b * r1) + (code2.b * r2));
	*/
	result.r = (UInt8)((((UInt16)code1.r * r1) + (UInt16)(code2.r * r2)) >> 8);
	result.g = (UInt8)((((UInt16)code1.g * r1) + (UInt16)(code2.g * r2)) >> 8);
	result.b = (UInt8)((((UInt16)code1.b * r1) + (UInt16)(code2.b * r2)) >> 8);

	return result.u;
}

static UInt32
func TextureColorCode(Texture texture, Int32 x, UInt8 subx, Int32 y, UInt8 suby)
{
	Int32 row1 = y;
	Int32 row2 = (row1 + 1) & (texture.side - 1);
	Int32 col1 = x;
	Int32 col2 = (col1 + 1) & (texture.side - 1);

	UInt32 code11 = TextureColorCodeInt(texture, row1, col1);
	UInt32 code12 = TextureColorCodeInt(texture, row1, col2);
	UInt32 code1 = ColorCodeLerp(code11, subx, code12);

	UInt32 code21 = TextureColorCodeInt(texture, row2, col1);
	UInt32 code22 = TextureColorCodeInt(texture, row2, col2);
	UInt32 code2 = ColorCodeLerp(code21, subx, code22);

	UInt32 code = ColorCodeLerp(code1, suby, code2);
	return code;
}
