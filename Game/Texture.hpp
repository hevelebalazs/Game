#pragma once

#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Texture 
{
	I32 side; // NOTE: power of two
	I32 log_side;
	U32 *memory;
};

static Texture
func CopyTexture(Texture *texture) 
{
	Texture result = {};
	result.side = texture->side;
	result.log_side = texture->log_side;
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32 *result_pixel = result.memory;
	U32 *texture_pixel = texture->memory;
	// TODO: use a memcpy here?
	for(I32 row = 0; row < result.side; row++) 
	{
		for(I32 col = 0; col < result.side; col++) 
		{
			*result_pixel = *texture_pixel;
			result_pixel++;
			texture_pixel++;
		}
	}

	return result;
}

static void
func Swap2(U32 *i1, U32 *i2)
{
	U32 tmp = *i1;
	*i1 = *i2;
	*i2 = tmp;
}

static void
func Swap4(U32 *i1, U32 *i2, U32 *i3, U32 *i4)
{
	U32 tmp = *i1;
	*i1 = *i2;
	*i2 = *i3;
	*i3 = *i4;
	*i4 = tmp;
}

static U32 *
func TextureAddress(Texture *texture, I32 row, I32 col)
{
	U32 *result = texture->memory + (row * texture->side) + col;
	return result;
}

static U32
func TextureValue(Texture *texture, I32 row, I32 col)
{
	U32 result = *TextureAddress(texture, row, col);
	return result;
}

static void
func RotateTextureUpsideDown(Texture *texture)
{
	I32 side = texture->side;
	I32 half_side = (side / 2);

	for(I32 row = 0; row < half_side; row++) 
	{
		for(I32 col = 0; col < side; col++) 
		{
			U32 *top = TextureAddress(texture, row, col);
			U32 *bottom = TextureAddress(texture, side - 1 - row, side - 1 - col);
			Swap2(top, bottom);
		}
	}
}

static void
func RotateTextureLeft(Texture *texture)
{
	I32 side = texture->side;
	I32 half_side = (side / 2);
	for(I32 row = 0; row < half_side; row++) 
	{
		for(I32 col = 0; col < half_side; col++) 
		{
			U32 *top_left  = TextureAddress(texture, row, col);
			U32 *top_right = TextureAddress(texture, col, side - 1 - row);
			U32 *bottom_right = TextureAddress(texture, side - 1 - row, side - 1 - col);
			U32 *bottom_left = TextureAddress(texture, side - 1 - col, row);

			Swap4(top_left, top_right, bottom_right, bottom_left);
		}
	}
}

static void
func RotateTextureRight(Texture *texture)
{
	I32 side = texture->side;
	I32 half_side = (side / 2);
	for(I32 row = 0; row < half_side; row++) 
	{
		for(I32 col = 0; col < half_side; col++) 
		{
			U32 *top_left  = TextureAddress(texture, row, col);
			U32 *top_right = TextureAddress(texture, col, side - 1 - row);
			U32 *bottom_right = TextureAddress(texture, side - 1 - row, side - 1 - col);
			U32 *bottom_left = TextureAddress(texture, side - 1 - col, row);

			Swap4(top_left, bottom_left, bottom_right, top_right);
		}
	}
}

static Texture
func RoofTexture(I32 log_side)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	I32 tile_width = 16;
	I32 tile_height = 32;

	U32 *pixel = result.memory;

	for(I32 row = 0; row < result.side; row++) 
	{
		for(I32 col = 0; col < result.side; col++) 
		{
			I32 red = 0;
			I32 green = 0;
			I32 blue = 0;

			I32 tile_left = tile_width * (col / tile_width);
			I32 tile_right = tile_left + tile_width;
			I32 left_dist = col - tile_left;

			I32 tile_top = tile_height * (row / tile_height);
			I32 top_dist = row - tile_top;

			I32 height = IntMin2(col - tile_left, tile_right - col);
			I32 tile_bottom = tile_top + height;

			B32 on_side = (col % tile_width == 0);
			B32 on_bottom = (row == tile_bottom);

			B32 on_tile_side = (on_side || on_bottom);

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
func GrassTexture(I32 log_side, MemArena *tmp_arena)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32 *pixel = result.memory;
	for(I32 row = 0; row < result.side; row++) 
	{
		for(I32 col = 0; col < result.side; col++) 
		{
			*pixel = 0;
			pixel++;
		}
	}

	R32 multiplier = 0.8f;
	I32 grid_n = 2;
	R32 opacity = 1.0f - multiplier;
	while(grid_n <= 256) 
	{
		R32 *grid_values = ArenaAllocArray(tmp_arena, R32, (grid_n + 1) * (grid_n + 1));
		R32 *grid_value = grid_values;
		for(I32 row = 0; row <= grid_n; row++) 
		{
			for(I32 col = 0; col <= grid_n; col++) 
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

		I32 grid_width = (result.side / grid_n);

		U32 *pixel = result.memory;
		for(I32 row = 0; row < result.side; row++) 
		{
			I32 y0 = (row / grid_width);
			I32 y1 = (y0 + 1);
			R32 yr = ((R32)(row % grid_width) / (R32)grid_width);

			for(I32 col = 0; col < result.side; col++) 
			{
				I32 x0 = (col / grid_width);
				I32 x1 = (x0 + 1);
				R32 xr = ((R32)(col % grid_width) / (R32)grid_width);

				R32 top_left     = *(grid_values + y0 * (grid_n + 1) + x0);
				R32 top_right    = *(grid_values + y0 * (grid_n + 1) + x1);
				R32 bottom_left  = *(grid_values + y1 * (grid_n + 1) + x0);
				R32 bottom_right = *(grid_values + y1 * (grid_n + 1) + x1);

				R32 top    = Lerp(top_left, xr, top_right);
				R32 bottom = Lerp(bottom_left, xr, bottom_right);
				R32 value  = Lerp(top, yr, bottom);

				V4 old_color = GetColorFromColorCode(*pixel);
				V4 new_color = MakeColor(0.0f, value, 0.0f);

				V4 color = AddColors(old_color, new_color);
				*pixel = GetColorCode(color);
				pixel++;
			}
		}

		ArenaPopTo(tmp_arena, grid_values);

		grid_n *= 2;
		opacity *= multiplier;
	}

	pixel = result.memory;
	for(I32 row = 0; row < result.side; row++) 
	{
		for(I32 col = 0; col < result.side; col++) 
		{
			V4 color = GetColorFromColorCode(*pixel);
			R32 green = RandomBetween(color.green * 0.9f, color.green * 1.0f);
			V4 new_color = MakeColor(0.0f, green, 0.0f);
			*pixel = GetColorCode(new_color);
			pixel++;
		}
	}

	return result;
}

static Texture
func RandomGreyTexture(I32 log_side, I32 min_ratio, I32 max_ratio)
{
	Texture result = {};
	result.log_side = log_side;
	result.side = (1 << log_side);
	// TODO: use a memory arena
	result.memory = new U32[result.side * result.side];

	U32 *pixel = result.memory;
	for(I32 row = 0; row < result.side; row++) 
	{
		for(I32 col = 0; col < result.side; col++) 
		{
			I32 grey_ratio = IntRandom(min_ratio, max_ratio);

			*pixel = (grey_ratio << 16) | (grey_ratio << 8) | (grey_ratio << 0); 
			pixel++;
		}
	}

	return result;
}

static U32
func TextureColorCodeInt(Texture texture, I32 row, I32 col)
{
	U32 result = *(texture.memory + (row << texture.log_side) + (col));
	return result;
}

static V4
func TextureColorInt(Texture texture, I32 row, I32 col)
{
	I32 color_code = TextureColorCodeInt(texture, row, col);
	V4 result = GetColorFromColorCode(color_code);
	return result;
}

static V4
func TextureColor(Texture texture, R32 x, R32 y)
{
	I32 row = ((I32)y & (texture.side - 1));
	I32 col = ((I32)x & (texture.side - 1));
	V4 color = TextureColorInt(texture, row, col);
	return color;
}

static U32
func TextureColorCode(Texture texture, R32 x, R32 y)
{
	V4 color = TextureColor(texture, x, y);
	U32 color_code = GetColorCode(color);
	return color_code;
}

static U32
func ColorCodeLerp(U32 color_code1, U8 ratio, U32 color_code2)
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
	code1.u = color_code1;
	code2.u = color_code2;

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

static U32
func TextureColorCode(Texture texture, I32 x, U8 subx, I32 y, U8 suby)
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
