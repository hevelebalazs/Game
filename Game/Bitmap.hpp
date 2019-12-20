#pragma once

#include <windows.h>

#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "String.hpp"
#include "Text.hpp"
#include "Type.hpp"

struct Bitmap
{
	I32 width;
	I32 height;
	U32 *memory;
};

#define BitmapBytesPerPixel 4

static BITMAPINFO
func GetBitmapInfo(Bitmap *bitmap)
{
	BITMAPINFO info = {};
	BITMAPINFOHEADER* header = &info.bmiHeader;
	header->biSize = sizeof(*header);
	header->biWidth = bitmap->width;
	header->biHeight = -bitmap->height;
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;
	return info;
}

static void
func ResizeBitmap(Bitmap *bitmap, I32 width, I32 height)
{
	if(bitmap->memory)
	{
		delete bitmap->memory;
	}
    
	bitmap->width = width;
	bitmap->height = height;
    
	I32 bitmap_memory_size = (bitmap->width * bitmap->height);
	bitmap->memory = new U32[bitmap_memory_size];
}

static V4
func MakeColor(R32 red, R32 green, R32 blue)
{
	V4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = 1.0f;
	return color;
}

static U32
func GetColorCode(V4 color)
{
	U8 alpha = (U8)(color.alpha * 255);
	U8 red   = (U8)(color.red * 255);
	U8 green = (U8)(color.green * 255);
	U8 blue  = (U8)(color.blue * 255);
    
	U32 color_code = (alpha << 24) + (red << 16) + (green << 8) + (blue);
	return color_code;
}

static U32
func MakeColorCode(R32 red, R32 green, R32 blue)
{
	V4 color = MakeColor(red, green, blue);
	U32 color_code = GetColorCode(color);
	return color_code;
}

static V4
func GetRandomColor()
{
	R32 r = RandomBetween(0.0f, 1.0f);
	R32 g = RandomBetween(0.0f, 1.0f);
	R32 b = RandomBetween(0.0f, 1.0f);
	V4 random_color = MakeColor(r, g, b);
	return random_color;
}

static U32
func GetRandomColorCode()
{
	V4 color = GetRandomColor ();
	I32 color_code = GetColorCode(color);
	return color_code;
}

static V4
func MakeAlphaColor(R32 red, R32 green, R32 blue, R32 alpha)
{
	V4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	return color;
}

static U32
func MakeAlphaColorCode(R32 red, R32 green, R32 blue, R32 alpha)
{
	V4 color = MakeAlphaColor(red, green, blue, alpha);
	U32 colorCode = GetColorCode(color);
	return colorCode;
}

static V4
func GetColorFromColorCode(U32 color_code)
{
	V4 color = {};
	color.alpha = (R32)((color_code & 0xff000000) >> 24) * (1.0f / 255.0f);
	color.red   = (R32)((color_code & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (R32)((color_code & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (R32)((color_code & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

static V4
func AddColors(V4 color1, V4 color2)
{
	V4 result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

static V4
func MixColors(V4 base_color, V4 color_to_add)
{
	V4 result = {};
	Assert(IsBetween(base_color.alpha, 0.0f, 1.0f));
	Assert(IsBetween(color_to_add.alpha, 0.0f, 1.0f));
	result.alpha = base_color.alpha + (1.0f - base_color.alpha) * color_to_add.alpha;
	Assert(IsBetween(result.alpha, 0.0f, 1.0f));
	result.red = color_to_add.red + (base_color.red * (1.0f - color_to_add.alpha));
	result.green = color_to_add.green + (base_color.green * (1.0f - color_to_add.alpha));
	result.blue = color_to_add.blue + (base_color.blue * (1.0f - color_to_add.alpha));
	return result;
}

static U32
func MixColorCodes(U32 base_color_code, U32 color_code_to_add)
{
	V4 base_color = GetColorFromColorCode(base_color_code);
	V4 color_to_add = GetColorFromColorCode(color_code_to_add);
	V4 result_color = MixColors(base_color, color_to_add);
	U32 result_color_code = GetColorCode(result_color);
	return result_color_code;
}

static V4
func InterpolateColors(V4 color1, R32 ratio, V4 color2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	V4 result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}

static U32 *
func GetBitmapPixelAddress(Bitmap *bitmap, I32 row, I32 col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	U32 *pixel_address = bitmap->memory + row * bitmap->width + col;
	return pixel_address;
}

static V4
func GetBitmapPixelColor(Bitmap *bitmap, I32 row, I32 col)
{
	U32 *address = GetBitmapPixelAddress(bitmap, row, col);
	U32 color_code = *address;
	V4 color = GetColorFromColorCode(color_code);
	return color;
}

static U32
func GetClosestBitmapColorCode(Bitmap *bitmap, R32 height_ratio, R32 width_ratio)
{
	I32 row = Floor(height_ratio * (bitmap->height - 1));
	I32 col = Floor(width_ratio * (bitmap->width - 1));
	Assert(IsIntBetween(row, 0, bitmap->height - 1));
	Assert(IsIntBetween(col, 0, bitmap->width - 1));
	U32 *address = GetBitmapPixelAddress(bitmap, row, col);
	U32 color_code = *address;
	return color_code;
}

static B32
func IsValidBitmapPixel(Bitmap *bitmap, I32 row, I32 col)
{
	B32 is_valid_row = IsIntBetween(row, 0, bitmap->height - 1);
	B32 is_valid_col = IsIntBetween(col, 0, bitmap->width - 1);
	B32 is_valid = (is_valid_row && is_valid_col);
	return is_valid;
}

static void
func PaintBitmapPixel(Bitmap *bitmap, I32 row, I32 col, U32 color_code)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
    
	U32 *pixel_address = GetBitmapPixelAddress(bitmap, row, col);
	*pixel_address = color_code;
}

static void
func MixBitmapPixel(Bitmap *bitmap, I32 row, I32 col, V4 new_color)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
	V4 old_color = GetBitmapPixelColor(bitmap, row, col);
	V4 mixed_color = MixColors(old_color, new_color);
	PaintBitmapPixel(bitmap, row, col, GetColorCode(mixed_color));
}

struct BresenhamData 
{
	I32 row1, col1;
	I32 row2, col2;
	I32 row_abs, col_abs;
	I32 row_add, col_add;
	I32 error1, error2;
};

static BresenhamData
func InitBresenham(I32 row1, I32 col1, I32 row2, I32 col2)
{
	BresenhamData data = {};
	data.col1 = col1;
	data.row1 = row1;
	data.col2 = col2;
	data.row2 = row2;
    
	data.row_abs = IntAbs(data.row1 - data.row2);
	data.col_abs = IntAbs(data.col1 - data.col2);
    
	data.col_add = 1;
	if(data.col1 > data.col2)
	{
		data.col_add = -1;
	}
    
	data.row_add = 1;
	if(data.row1 > data.row2)
	{
		data.row_add = -1;
	}
    
	data.error1 = 0;
	if(data.col_abs > data.row_abs)
	{
		data.error1 = data.col_abs / 2;
	}
	else
	{
		data.error1 = -data.row_abs / 2;
	}
    
	data.error2 = 0;
	return data;
}

static void
func AdvanceBresenham(BresenhamData *data)
{
	data->error2 = data->error1;
	if(data->error2 > -data->col_abs) 
	{
		data->error1 -= data->row_abs;
		data->col1 += data->col_add;
	}
    
	if(data->error2 < data->row_abs) 
	{
		data->error1 += data->col_abs;
		data->row1 += data->row_add;
	}
}

static void
func DrawBitmapBresenhamLine(Bitmap *bitmap, 
							 I32 row1, I32 col1,
							 I32 row2, I32 col2, 
							 V4 color)
{
	U32 color_code = GetColorCode(color);
	BresenhamData data = InitBresenham(row1, col1, row2, col2);
	while(1) 
	{
		if(IsValidBitmapPixel(bitmap, data.row1, data.col1))
		{
			PaintBitmapPixel(bitmap, data.row1, data.col1, color_code);
		}
        
		if (data.row1 == data.row2 && data.col1 == data.col2)
		{
			break;
		}
        
		AdvanceBresenham(&data);
	}
}

static void
func FillBitmapWithColor(Bitmap *bitmap, V4 color)
{
	U32 color_code = GetColorCode(color);
	U32 *pixel = bitmap->memory;
	for(I32 row = 0; row < bitmap->height; ++row) 
	{
		for(I32 col = 0; col < bitmap->width; ++col) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

static void
func CopyBitmap(Bitmap *from_bitmap, Bitmap *to_bitmap, I32 to_left, I32 to_top)
{
	I32 to_right = to_left + from_bitmap->width - 1;
	I32 to_bottom = to_top + from_bitmap->height - 1;
    
	I32 from_left = 0;
	if(to_left < 0)
	{
		from_left = -to_left;
	}
    
	I32 from_right = from_bitmap->width - 1;
	if(to_right > to_bitmap->width - 1)
	{
		from_right -= (to_right - (to_bitmap->width - 1));
	}
    
	I32 from_top = 0;
	if(to_top < 0)
	{
		from_top = -to_top;
	}
    
	I32 from_bottom = from_bitmap->height - 1;
	if(to_bottom > to_bitmap->height - 1)
	{
		from_bottom -= (to_bottom - (to_bitmap->height - 1));
	}
    
	for(I32 from_row = from_top; from_row <= from_bottom; ++from_row)
	{
		for(I32 from_col = from_left; from_col <= from_right; ++from_col)
		{
			I32 to_row = to_top + from_row;
			I32 to_col = to_left + from_col;
            
			V4 from_color = GetBitmapPixelColor(from_bitmap, from_row, from_col);
			V4 to_color = GetBitmapPixelColor(to_bitmap, to_row, to_col);
			V4 mixed_color = MixColors(to_color, from_color);
			*GetBitmapPixelAddress(to_bitmap, to_row, to_col) = GetColorCode(mixed_color);
		}
	}
}

static void
func DrawBitmapPolyOutline(Bitmap *bitmap, I32 poly_n, I32 *poly_col_row, V4 color)
{
	for(I32 i = 0; i < poly_n; i++) 
	{
		I32 next = (i < poly_n - 1) ? (i + 1) : (0);
		I32 row1 = poly_col_row[2 * i];
		I32 col1 = poly_col_row[2 * i + 1];
		I32 row2 = poly_col_row[2 * next];
		I32 col2 = poly_col_row[2 * next + 1];
		DrawBitmapBresenhamLine(bitmap, row1, col1, row2, col2, color);
	}
}

static IntRect
func GetBitmapBounds(Bitmap *bitmap)
{
	IntRect bounds = {};
	bounds.left   = 0;
	bounds.right  = (bitmap->width - 1);
	bounds.top    = 0;
	bounds.bottom = (bitmap->height - 1);
	return bounds;
}

static void
func DrawBitmapRect(Bitmap *bitmap, IntRect rect, V4 color)
{
	IntRect bitmap_bounds = GetBitmapBounds(bitmap);
	IntRect draw_rect = GetIntRectIntersection(rect, bitmap_bounds);

	U32 color_code = GetColorCode(color);
	for(I32 col = draw_rect.left; col <= draw_rect.right; col++)
	{
		for(I32 row = draw_rect.top; row <= draw_rect.bottom; row++)
		{
			U32 *pixel_address = GetBitmapPixelAddress(bitmap, row, col);
			*pixel_address = color_code;
		}
	}
}

static void
func DrawBitmapRectOutline(Bitmap *bitmap, IntRect rect, V4 color)
{
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.left, rect.top, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.right, rect.bottom, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.right, rect.bottom, rect.left, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.left, rect.top, rect.left, color);
}

static void
func DrawBitmapGlyph(Bitmap *bitmap, Glyph *glyph, I32 x, I32 base_line_y, V4 color)
{
	I32 start_col = x + (I32)glyph->offset_x;
	I32 start_row = base_line_y + (I32)glyph->offset_y;
    
	for(I32 row = 0; row < 32; row++)
	{
		for(I32 col = 0; col < 32; col++)
		{
			U8 alpha_value = glyph->alpha[row][col];
			if(alpha_value == 0)
			{
				continue;
			}
            
			R32 alpha = (R32)alpha_value / 255.0f;
			V4 new_color = {};
			new_color.red   = color.red * alpha;
			new_color.green = color.green * alpha;
			new_color.blue  = color.blue * alpha;
			new_color.alpha = alpha;
            
			I32 to_row = start_row + row;
			I32 to_col = start_col + col;
            
			if(IsValidBitmapPixel(bitmap, to_row, to_col))
			{
				MixBitmapPixel(bitmap, to_row, to_col, new_color);
			}
		}
	}
}

static void
func DrawBitmapTextLine(Bitmap *bitmap, I8 *text, GlyphData *glyph_data, 
						I32 left, I32 base_line_y, V4 color)
{
	Assert(glyph_data);
	R32 textX = (R32)left;
	R32 textY = (R32)base_line_y;
    
	for(I32 i = 0; text[i]; i++)
	{
		U8 c = text[i];
		if(c == '\n')
		{
			continue;
		}
        
		Glyph *glyph = &glyph_data->glyphs[c];
        
		R32 right = textX + glyph->advance_x;
		U8 nextC = text[i + 1];
		if(nextC > 0)
		{
			right += glyph_data->kerning_table[c][nextC];
		}
        
		DrawBitmapGlyph(bitmap, glyph, (I32)textX, (I32)textY, color);
		textX = right;
	}
}

static R32
func GetTextPixelWidth(I8 *text, GlyphData *glyph_data)
{
	Assert(glyph_data != 0);
    
	R32 width = 0.0f;
	for(I32 i = 0; text[i]; i++)
	{
		U8 c = text[i];
		Glyph *glyph = &glyph_data->glyphs[c];
		width += glyph->advance_x;
		
		U8 next_c = text[i + 1];
		if(next_c > 0)
		{
			width += glyph_data->kerning_table[c][next_c];
		}
	}
    
	return width;
}

static void
func DrawBitmapTextLineCentered(Bitmap *bitmap, I8 *text, GlyphData *glyph_data,
								IntRect rect, V4 color)
{
	I32 height = TextHeightInPixels;
	I32 width = (I32)GetTextPixelWidth(text, glyph_data);
	I32 left = (rect.left + rect.right) / 2 - width / 2;
	I32 top = (rect.bottom + rect.top) / 2 - height / 2;
	I32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

static void
func DrawBitmapTextLineTopLeft(Bitmap *bitmap, I8 *text, GlyphData *glyph_data, 
							   I32 left, I32 top, V4 color)
{
	I32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

static void
func DrawBitmapTextLineTopRight(Bitmap *bitmap, I8 *text, GlyphData *glyph_data,
								I32 right, I32 top, V4 color)
{
	I32 width = (I32)GetTextPixelWidth(text, glyph_data);
	I32 left = right - width;
	I32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

#define TooltipWidth 300
#define TooltipPadding 3
#define TooltipTopPadding 5

static I32
func GetTooltipHeight(I32 line_n)
{
	I32 height = TooltipTopPadding + line_n * (TextHeightInPixels) + TooltipPadding;
	return height;
}

static void
func DrawBitmapStringTooltip(Bitmap *bitmap, String string, GlyphData *glyph_data, 
							 I32 top, I32 left)
{
	I32 line_n = GetNumberOfLines(string);
    
	Assert(glyph_data != 0);

	IntRect rect = {};
	rect.left = left;
	rect.top = top;
    
	rect.right = left + TooltipWidth;
	I32 height = GetTooltipHeight(line_n);
	rect.bottom = top + height;
    
	V4 tooltip_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltip_color);
    
	V4 tooltip_border_color = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltip_border_color);
    
	V4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 normal_color = MakeColor(1.0f, 1.0f, 1.0f);
	I32 base_line_y = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	I32 text_left = left + TooltipPadding;
    
	R32 text_x = (R32)text_left;
	I32 line_index = 0;
    
	for(I32 i = 0; i < string.used_size; ++i)
	{
		U8 c = string.buffer[i];
		Assert(c != 0);
        
		if(c == '\n')
		{
			line_index++;
			base_line_y += TextHeightInPixels;
			text_x = (R32)text_left;
		}
		else
		{
			Glyph *glyph = &glyph_data->glyphs[c];
            
			R32 right = text_x + glyph->advance_x;
			U8 next_c = string.buffer[i + 1];
			if(next_c > 0)
			{
				right += glyph_data->kerning_table[c][next_c];
			}
            
			V4 color = (line_index == 0) ? title_color : normal_color;
			DrawBitmapGlyph(bitmap, glyph, I32(text_x), I32(base_line_y), color);
			text_x = right;
		}
        
		Assert(text_x <= rect.right);
	}
}

static void
func DrawBitmapStringTooltipBottom(Bitmap *bitmap, String string, GlyphData *glyph_data, 
								   I32 bottom, I32 left)
{
	I32 line_n = GetNumberOfLines(string);
	I32 top = bottom - GetTooltipHeight(line_n);
	DrawBitmapStringTooltip(bitmap, string, glyph_data, top, left);
}

static void
func DrawBitmapStringTooltipBottomRight(Bitmap *bitmap, String string, GlyphData *glyph_data, 
										I32 bottom, I32 right)
{
	I32 line_n = GetNumberOfLines(string);
	I32 top = bottom - GetTooltipHeight(line_n);
	I32 left = right - TooltipWidth;
	DrawBitmapStringTooltip(bitmap, string, glyph_data, top, left);
}

static void
func DrawBitmapTooltip(Bitmap *bitmap, I8 **lines, I32 line_n, GlyphData *glyph_data, 
					   I32 top, I32 left)
{
	Assert(glyph_data != 0);
    
	IntRect rect = {};
	rect.left = left;
	rect.top = top;
	rect.right = left + TooltipWidth;
	I32 height = GetTooltipHeight(line_n);
	rect.bottom = top + height;
    
	V4 tooltip_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltip_color);
    
	V4 tooltip_border_color = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltip_border_color);
    
	V4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
	V4 normal_color = MakeColor(1.0f, 1.0f, 1.0f);
	I32 base_line_y = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	I32 text_left = left + TooltipPadding;
	for(I32 i = 0; i < line_n; i++)
	{
		V4 color = (i == 0) ? title_color : normal_color;
		I8* line = lines[i];
		DrawBitmapTextLine(bitmap, line, glyph_data, text_left, base_line_y, color);
		base_line_y += TextHeightInPixels;
	}
}

static void
func DrawBitmapTooltipBottom(Bitmap *bitmap, I8 **lines, I32 line_n, 
							 GlyphData *glyph_data, I32 bottom, I32 left)
{
	I32 top = bottom - GetTooltipHeight(line_n);
	DrawBitmapTooltip(bitmap, lines, line_n, glyph_data, top, left);
}

static void
func DrawBitmapTooltipBottomRight(Bitmap *bitmap, I8 **lines, I32 line_n, 
								  GlyphData *glyph_data, I32 bottom, I32 right)
{
	I32 top = bottom - GetTooltipHeight(line_n);
	I32 left = right - TooltipWidth;
	DrawBitmapTooltip(bitmap, lines, line_n, glyph_data, top, left);
}