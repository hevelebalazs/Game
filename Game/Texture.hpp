#pragma once

#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Texture 
{
	Int32 side; // NOTE: power of two
	Int32 log_side;
	UInt32 *memory;
};

static Texture
func CopyTexture(Texture *texture) 
{
	Texture result = {};
	result.side = texture->side;
	result.log_side = texture->log_side;
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	UInt32 *result_pixel = result.memory;
	UInt32 *texture_pixel = texture->memory;
	// TODO: use a memcpy here?
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			*result_pixel = *texture_pixel;
			result_pixel++;
			texture_pixel++;
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
	Int32 half_side = (side / 2);

	for(Int32 row = 0; row < half_side; row++) 
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
	Int32 half_side = (side / 2);
	for(Int32 row = 0; row < half_side; row++) 
	{
		for(Int32 col = 0; col < half_side; col++) 
		{
			UInt32 *top_left  = TextureAddress(texture, row, col);
			UInt32 *top_right = TextureAddress(texture, col, side - 1 - row);
			UInt32 *bottom_right = TextureAddress(texture, side - 1 - row, side - 1 - col);
			UInt32 *bottom_left = TextureAddress(texture, side - 1 - col, row);

			Swap4(top_left, top_right, bottom_right, bottom_left);
		}
	}
}

static void
func RotateTextureRight(Texture *texture)
{
	Int32 side = texture->side;
	Int32 half_side = (side / 2);
	for(Int32 row = 0; row < half_side; row++) 
	{
		for(Int32 col = 0; col < half_side; col++) 
		{
			UInt32 *top_left  = TextureAddress(texture, row, col);
			UInt32 *top_right = TextureAddress(texture, col, side - 1 - row);
			UInt32 *bottom_right = TextureAddress(texture, side - 1 - row, side - 1 - col);
			UInt32 *bottom_left = TextureAddress(texture, side - 1 - col, row);

			Swap4(top_left, bottom_left, bottom_right, top_right);
		}
	}
}

static Texture
func RoofTexture(Int32 log_side)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	Int32 tile_width = 16;
	Int32 tile_height = 32;

	UInt32 *pixel = result.memory;

	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Int32 red = 0;
			Int32 green = 0;
			Int32 blue = 0;

			Int32 tile_left = tile_width * (col / tile_width);
			Int32 tile_right = tile_left + tile_width;
			Int32 left_dist = col - tile_left;

			Int32 tile_top = tile_height * (row / tile_height);
			Int32 top_dist = row - tile_top;

			Int32 height = IntMin2(col - tile_left, tile_right - col);
			Int32 tile_bottom = tile_top + height;

			Bool32 on_side = (col % tile_width == 0);
			Bool32 on_bottom = (row == tile_bottom);

			Bool32 on_tile_side = (on_side || on_bottom);

			if(!on_tile_side)
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
func GrassTexture(Int32 log_side, MemArena *tmp_arena)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
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
	Int32 grid_n = 2;
	Real32 opacity = 1.0f - multiplier;
	while(grid_n <= 256) 
	{
		Real32 *grid_values = ArenaAllocArray(tmp_arena, Real32, (grid_n + 1) * (grid_n + 1));
		Real32 *grid_value = grid_values;
		for(Int32 row = 0; row <= grid_n; row++) 
		{
			for(Int32 col = 0; col <= grid_n; col++) 
			{
				if(row == grid_n)
				{
					*grid_value = *(grid_values + (grid_n + 1) * 0 + col);
				}
				else if(col == grid_n)
				{
					*grid_value = *(grid_values + (grid_n + 1) * row + 0);
				}
				else
				{
					*grid_value = RandomBetween(0.0f, opacity);
				}

				grid_value++;
			}
		}

		Int32 grid_width = (result.side / grid_n);

		UInt32 *pixel = result.memory;
		for(Int32 row = 0; row < result.side; row++) 
		{
			Int32 y0 = (row / grid_width);
			Int32 y1 = (y0 + 1);
			Real32 yr = ((Real32)(row % grid_width) / (Real32)grid_width);

			for(Int32 col = 0; col < result.side; col++) 
			{
				Int32 x0 = (col / grid_width);
				Int32 x1 = (x0 + 1);
				Real32 xr = ((Real32)(col % grid_width) / (Real32)grid_width);

				Real32 top_left     = *(grid_values + y0 * (grid_n + 1) + x0);
				Real32 top_right    = *(grid_values + y0 * (grid_n + 1) + x1);
				Real32 bottom_left  = *(grid_values + y1 * (grid_n + 1) + x0);
				Real32 bottom_right = *(grid_values + y1 * (grid_n + 1) + x1);

				Real32 top    = Lerp(top_left, xr, top_right);
				Real32 bottom = Lerp(bottom_left, xr, bottom_right);
				Real32 value  = Lerp(top, yr, bottom);

				Vec4 old_color = GetColorFromColorCode(*pixel);
				Vec4 new_color = MakeColor(0.0f, value, 0.0f);

				Vec4 color = AddColors(old_color, new_color);
				*pixel = GetColorCode(color);
				pixel++;
			}
		}

		ArenaPopTo(tmp_arena, grid_values);

		grid_n *= 2;
		opacity *= multiplier;
	}

	pixel = result.memory;
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Vec4 color = GetColorFromColorCode(*pixel);
			Real32 green = RandomBetween(color.green * 0.9f, color.green * 1.0f);
			Vec4 new_color = MakeColor(0.0f, green, 0.0f);
			*pixel = GetColorCode(new_color);
			pixel++;
		}
	}

	return result;
}

static Texture
func RandomGreyTexture(Int32 log_side, Int32 min_ratio, Int32 max_ratio)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
	// TODO: use a memory arena
	result.memory = new UInt32[result.side * result.side];

	UInt32 *pixel = result.memory;
	for(Int32 row = 0; row < result.side; row++) 
	{
		for(Int32 col = 0; col < result.side; col++) 
		{
			Int32 grey_ratio = IntRandom(min_ratio, max_ratio);

			*pixel = (grey_ratio << 16) | (grey_ratio << 8) | (grey_ratio << 0); 
			pixel++;
		}
	}

	return result;
}

static UInt32
func TextureColorCodeInt(Texture texture, Int32 row, Int32 col)
{
	UInt32 result = *(texture.memory + (row << texture.log_side) + (col));
	return result;
}

static Vec4
func TextureColorInt(Texture texture, Int32 row, Int32 col)
{
	Int32 color_code = TextureColorCodeInt(texture, row, col);
	Vec4 result = GetColorFromColorCode(color_code);
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
	UInt32 color_code = GetColorCode(color);
	return color_code;
}

static UInt32
func ColorCodeLerp(UInt32 color_code1, UInt8 ratio, UInt32 color_code2)
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
	code1.u = color_code1;
	code2.u = color_code2;

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
