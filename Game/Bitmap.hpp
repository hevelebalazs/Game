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
	Int32 width;
	Int32 height;
	UInt32 *memory;
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
func ResizeBitmap(Bitmap *bitmap, Int32 width, Int32 height)
{
	if(bitmap->memory)
	{
		delete bitmap->memory;
	}
    
	bitmap->width = width;
	bitmap->height = height;
    
	Int32 bitmap_memory_size = (bitmap->width * bitmap->height);
	bitmap->memory = new UInt32[bitmap_memory_size];
}

static Vec4
func MakeColor(Real32 red, Real32 green, Real32 blue)
{
	Vec4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = 1.0f;
	return color;
}

static UInt32
func GetColorCode(Vec4 color)
{
	UInt8 alpha = (UInt8)(color.alpha * 255);
	UInt8 red   = (UInt8)(color.red * 255);
	UInt8 green = (UInt8)(color.green * 255);
	UInt8 blue  = (UInt8)(color.blue * 255);
    
	UInt32 color_code = (alpha << 24) + (red << 16) + (green << 8) + (blue);
	return color_code;
}

static UInt32
func MakeColorCode(Real32 red, Real32 green, Real32 blue)
{
	Vec4 color = MakeColor(red, green, blue);
	UInt32 color_code = GetColorCode(color);
	return color_code;
}

static Vec4
func GetRandomColor()
{
	Real32 r = RandomBetween(0.0f, 1.0f);
	Real32 g = RandomBetween(0.0f, 1.0f);
	Real32 b = RandomBetween(0.0f, 1.0f);
	Vec4 random_color = MakeColor(r, g, b);
	return random_color;
}

static UInt32
func GetRandomColorCode()
{
	Vec4 color = GetRandomColor ();
	Int32 color_code = GetColorCode(color);
	return color_code;
}

static Vec4
func MakeAlphaColor(Real32 red, Real32 green, Real32 blue, Real32 alpha)
{
	Vec4 color = {};
	color.red   = red;
	color.green = green;
	color.blue  = blue;
	color.alpha = alpha;
	return color;
}

static UInt32
func MakeAlphaColorCode(Real32 red, Real32 green, Real32 blue, Real32 alpha)
{
	Vec4 color = MakeAlphaColor(red, green, blue, alpha);
	UInt32 colorCode = GetColorCode(color);
	return colorCode;
}

static Vec4
func GetColorFromColorCode(UInt32 color_code)
{
	Vec4 color = {};
	color.alpha = (Real32)((color_code & 0xff000000) >> 24) * (1.0f / 255.0f);
	color.red   = (Real32)((color_code & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (Real32)((color_code & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (Real32)((color_code & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

static Vec4
func AddColors(Vec4 color1, Vec4 color2)
{
	Vec4 result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

static Vec4
func MixColors(Vec4 base_color, Vec4 color_to_add)
{
	Vec4 result = {};
	Assert(IsBetween(base_color.alpha, 0.0f, 1.0f));
	Assert(IsBetween(color_to_add.alpha, 0.0f, 1.0f));
	result.alpha = base_color.alpha + (1.0f - base_color.alpha) * color_to_add.alpha;
	Assert(IsBetween(result.alpha, 0.0f, 1.0f));
	result.red = color_to_add.red + (base_color.red * (1.0f - color_to_add.alpha));
	result.green = color_to_add.green + (base_color.green * (1.0f - color_to_add.alpha));
	result.blue = color_to_add.blue + (base_color.blue * (1.0f - color_to_add.alpha));
	return result;
}

static UInt32
func MixColorCodes(UInt32 base_color_code, UInt32 color_code_to_add)
{
	Vec4 base_color = GetColorFromColorCode(base_color_code);
	Vec4 color_to_add = GetColorFromColorCode(color_code_to_add);
	Vec4 result_color = MixColors(base_color, color_to_add);
	UInt32 result_color_code = GetColorCode(result_color);
	return result_color_code;
}

static Vec4
func InterpolateColors(Vec4 color1, Real32 ratio, Vec4 color2)
{
	Assert(IsBetween(ratio, 0.0f, 1.0f));
	Vec4 result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}

static UInt32 *
func GetBitmapPixelAddress(Bitmap *bitmap, Int32 row, Int32 col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	UInt32 *pixel_address = bitmap->memory + row * bitmap->width + col;
	return pixel_address;
}

static Vec4
func GetBitmapPixelColor(Bitmap *bitmap, Int32 row, Int32 col)
{
	UInt32 *address = GetBitmapPixelAddress(bitmap, row, col);
	UInt32 color_code = *address;
	Vec4 color = GetColorFromColorCode(color_code);
	return color;
}

static UInt32
func GetClosestBitmapColorCode(Bitmap *bitmap, Real32 height_ratio, Real32 width_ratio)
{
	Int32 row = Floor(height_ratio * (bitmap->height - 1));
	Int32 col = Floor(width_ratio * (bitmap->width - 1));
	Assert(IsIntBetween(row, 0, bitmap->height - 1));
	Assert(IsIntBetween(col, 0, bitmap->width - 1));
	UInt32 *address = GetBitmapPixelAddress(bitmap, row, col);
	UInt32 color_code = *address;
	return color_code;
}

static Bool32
func IsValidBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col)
{
	Bool32 is_valid_row = IsIntBetween(row, 0, bitmap->height - 1);
	Bool32 is_valid_col = IsIntBetween(col, 0, bitmap->width - 1);
	Bool32 is_valid = (is_valid_row && is_valid_col);
	return is_valid;
}

static void
func PaintBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col, UInt32 color_code)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
    
	UInt32 *pixel_address = GetBitmapPixelAddress(bitmap, row, col);
	*pixel_address = color_code;
}

static void
func MixBitmapPixel(Bitmap *bitmap, Int32 row, Int32 col, Vec4 new_color)
{
	Assert(IsValidBitmapPixel(bitmap, row, col));
	Vec4 old_color = GetBitmapPixelColor(bitmap, row, col);
	Vec4 mixed_color = MixColors(old_color, new_color);
	PaintBitmapPixel(bitmap, row, col, GetColorCode(mixed_color));
}

struct BresenhamData 
{
	Int32 row1, col1;
	Int32 row2, col2;
	Int32 row_abs, col_abs;
	Int32 row_add, col_add;
	Int32 error1, error2;
};

static BresenhamData
func InitBresenham(Int32 row1, Int32 col1, Int32 row2, Int32 col2)
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
							 Int32 row1, Int32 col1,
							 Int32 row2, Int32 col2, 
							 Vec4 color)
{
	UInt32 color_code = GetColorCode(color);
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
func FillBitmapWithColor(Bitmap *bitmap, Vec4 color)
{
	UInt32 color_code = GetColorCode(color);
	UInt32 *pixel = bitmap->memory;
	for(Int32 row = 0; row < bitmap->height; ++row) 
	{
		for(Int32 col = 0; col < bitmap->width; ++col) 
		{
			*pixel = color_code;
			pixel++;
		}
	}
}

static void
func CopyBitmap(Bitmap *from_bitmap, Bitmap *to_bitmap, Int32 to_left, Int32 to_top)
{
	Int32 to_right = to_left + from_bitmap->width - 1;
	Int32 to_bottom = to_top + from_bitmap->height - 1;
    
	Int32 from_left = 0;
	if(to_left < 0)
	{
		from_left = -to_left;
	}
    
	Int32 from_right = from_bitmap->width - 1;
	if(to_right > to_bitmap->width - 1)
	{
		from_right -= (to_right - (to_bitmap->width - 1));
	}
    
	Int32 from_top = 0;
	if(to_top < 0)
	{
		from_top = -to_top;
	}
    
	Int32 from_bottom = from_bitmap->height - 1;
	if(to_bottom > to_bitmap->height - 1)
	{
		from_bottom -= (to_bottom - (to_bitmap->height - 1));
	}
    
	for(Int32 from_row = from_top; from_row <= from_bottom; ++from_row)
	{
		for(Int32 from_col = from_left; from_col <= from_right; ++from_col)
		{
			Int32 to_row = to_top + from_row;
			Int32 to_col = to_left + from_col;
            
			Vec4 from_color = GetBitmapPixelColor(from_bitmap, from_row, from_col);
			Vec4 to_color = GetBitmapPixelColor(to_bitmap, to_row, to_col);
			Vec4 mixed_color = MixColors(to_color, from_color);
			*GetBitmapPixelAddress(to_bitmap, to_row, to_col) = GetColorCode(mixed_color);
		}
	}
}

static void
func DrawBitmapPolyOutline(Bitmap *bitmap, Int32 poly_n, Int32 *poly_col_row, Vec4 color)
{
	for(Int32 i = 0; i < poly_n; i++) 
	{
		Int32 next = (i < poly_n - 1) ? (i + 1) : (0);
		Int32 row1 = poly_col_row[2 * i];
		Int32 col1 = poly_col_row[2 * i + 1];
		Int32 row2 = poly_col_row[2 * next];
		Int32 col2 = poly_col_row[2 * next + 1];
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
func DrawBitmapRect(Bitmap *bitmap, IntRect rect, Vec4 color)
{
	IntRect bitmap_bounds = GetBitmapBounds(bitmap);
	IntRect draw_rect = GetIntRectIntersection(rect, bitmap_bounds);

	UInt32 color_code = GetColorCode(color);
	for(Int32 col = draw_rect.left; col <= draw_rect.right; col++)
	{
		for(Int32 row = draw_rect.top; row <= draw_rect.bottom; row++)
		{
			UInt32 *pixel_address = GetBitmapPixelAddress(bitmap, row, col);
			*pixel_address = color_code;
		}
	}
}

static void
func DrawBitmapRectOutline(Bitmap *bitmap, IntRect rect, Vec4 color)
{
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.left, rect.top, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.top, rect.right, rect.bottom, rect.right, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.right, rect.bottom, rect.left, color);
	DrawBitmapBresenhamLine(bitmap, rect.bottom, rect.left, rect.top, rect.left, color);
}

static void
func DrawBitmapGlyph(Bitmap *bitmap, Glyph *glyph, Int32 x, Int32 base_line_y, Vec4 color)
{
	Int32 start_col = x + (Int32)glyph->offset_x;
	Int32 start_row = base_line_y + (Int32)glyph->offset_y;
    
	for(Int32 row = 0; row < 32; row++)
	{
		for(Int32 col = 0; col < 32; col++)
		{
			UInt8 alpha_value = glyph->alpha[row][col];
			if(alpha_value == 0)
			{
				continue;
			}
            
			Real32 alpha = (Real32)alpha_value / 255.0f;
			Vec4 new_color = {};
			new_color.red   = color.red * alpha;
			new_color.green = color.green * alpha;
			new_color.blue  = color.blue * alpha;
			new_color.alpha = alpha;
            
			Int32 to_row = start_row + row;
			Int32 to_col = start_col + col;
            
			if(IsValidBitmapPixel(bitmap, to_row, to_col))
			{
				MixBitmapPixel(bitmap, to_row, to_col, new_color);
			}
		}
	}
}

static void
func DrawBitmapTextLine(Bitmap *bitmap, Int8 *text, GlyphData *glyph_data, 
						Int32 left, Int32 base_line_y, Vec4 color)
{
	Assert(glyph_data);
	Real32 textX = (Real32)left;
	Real32 textY = (Real32)base_line_y;
    
	for(Int32 i = 0; text[i]; i++)
	{
		UInt8 c = text[i];
		if(c == '\n')
		{
			continue;
		}
        
		Glyph *glyph = &glyph_data->glyphs[c];
        
		Real32 right = textX + glyph->advance_x;
		UInt8 nextC = text[i + 1];
		if(nextC > 0)
		{
			right += glyph_data->kerning_table[c][nextC];
		}
        
		DrawBitmapGlyph(bitmap, glyph, (Int32)textX, (Int32)textY, color);
		textX = right;
	}
}

static Real32
func GetTextPixelWidth(Int8 *text, GlyphData *glyph_data)
{
	Assert(glyph_data != 0);
    
	Real32 width = 0.0f;
	for(Int32 i = 0; text[i]; i++)
	{
		UInt8 c = text[i];
		Glyph *glyph = &glyph_data->glyphs[c];
		width += glyph->advance_x;
		
		UInt8 next_c = text[i + 1];
		if(next_c > 0)
		{
			width += glyph_data->kerning_table[c][next_c];
		}
	}
    
	return width;
}

static void
func DrawBitmapTextLineCentered(Bitmap *bitmap, Int8 *text, GlyphData *glyph_data,
								IntRect rect, Vec4 color)
{
	Int32 height = TextHeightInPixels;
	Int32 width = (Int32)GetTextPixelWidth(text, glyph_data);
	Int32 left = (rect.left + rect.right) / 2 - width / 2;
	Int32 top = (rect.bottom + rect.top) / 2 - height / 2;
	Int32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

static void
func DrawBitmapTextLineTopLeft(Bitmap *bitmap, Int8 *text, GlyphData *glyph_data, 
							   Int32 left, Int32 top, Vec4 color)
{
	Int32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

static void
func DrawBitmapTextLineTopRight(Bitmap *bitmap, Int8 *text, GlyphData *glyph_data,
								Int32 right, Int32 top, Vec4 color)
{
	Int32 width = (Int32)GetTextPixelWidth(text, glyph_data);
	Int32 left = right - width;
	Int32 base_line_y = top + TextPixelsAboveBaseLine;
	DrawBitmapTextLine(bitmap, text, glyph_data, left, base_line_y, color);
}

#define TooltipWidth 300
#define TooltipPadding 3
#define TooltipTopPadding 5

static Int32
func GetTooltipHeight(Int32 line_n)
{
	Int32 height = TooltipTopPadding + line_n * (TextHeightInPixels) + TooltipPadding;
	return height;
}

static void
func DrawBitmapStringTooltip(Bitmap *bitmap, String string, GlyphData *glyph_data, 
							 Int32 top, Int32 left)
{
	Int32 line_n = GetNumberOfLines(string);
    
	Assert(glyph_data != 0);

	IntRect rect = {};
	rect.left = left;
	rect.top = top;
    
	rect.right = left + TooltipWidth;
	Int32 height = GetTooltipHeight(line_n);
	rect.bottom = top + height;
    
	Vec4 tooltip_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltip_color);
    
	Vec4 tooltip_border_color = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltip_border_color);
    
	Vec4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 normal_color = MakeColor(1.0f, 1.0f, 1.0f);
	Int32 base_line_y = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	Int32 text_left = left + TooltipPadding;
    
	Real32 text_x = (Real32)text_left;
	Int32 line_index = 0;
    
	for(Int32 i = 0; i < string.used_size; ++i)
	{
		UInt8 c = string.buffer[i];
		Assert(c != 0);
        
		if(c == '\n')
		{
			line_index++;
			base_line_y += TextHeightInPixels;
			text_x = (Real32)text_left;
		}
		else
		{
			Glyph *glyph = &glyph_data->glyphs[c];
            
			Real32 right = text_x + glyph->advance_x;
			UInt8 next_c = string.buffer[i + 1];
			if(next_c > 0)
			{
				right += glyph_data->kerning_table[c][next_c];
			}
            
			Vec4 color = (line_index == 0) ? title_color : normal_color;
			DrawBitmapGlyph(bitmap, glyph, Int32(text_x), Int32(base_line_y), color);
			text_x = right;
		}
        
		Assert(text_x <= rect.right);
	}
}

static void
func DrawBitmapStringTooltipBottom(Bitmap *bitmap, String string, GlyphData *glyph_data, 
								   Int32 bottom, Int32 left)
{
	Int32 line_n = GetNumberOfLines(string);
	Int32 top = bottom - GetTooltipHeight(line_n);
	DrawBitmapStringTooltip(bitmap, string, glyph_data, top, left);
}

static void
func DrawBitmapStringTooltipBottomRight(Bitmap *bitmap, String string, GlyphData *glyph_data, 
										Int32 bottom, Int32 right)
{
	Int32 line_n = GetNumberOfLines(string);
	Int32 top = bottom - GetTooltipHeight(line_n);
	Int32 left = right - TooltipWidth;
	DrawBitmapStringTooltip(bitmap, string, glyph_data, top, left);
}

static void
func DrawBitmapTooltip(Bitmap *bitmap, Int8 **lines, Int32 line_n, GlyphData *glyph_data, 
					   Int32 top, Int32 left)
{
	Assert(glyph_data != 0);
    
	IntRect rect = {};
	rect.left = left;
	rect.top = top;
	rect.right = left + TooltipWidth;
	Int32 height = GetTooltipHeight(line_n);
	rect.bottom = top + height;
    
	Vec4 tooltip_color = MakeColor(0.0f, 0.0f, 0.0f);
	DrawBitmapRect(bitmap, rect, tooltip_color);
    
	Vec4 tooltip_border_color = MakeColor(1.0f, 1.0f, 1.0f);
	DrawBitmapRectOutline(bitmap, rect, tooltip_border_color);
    
	Vec4 title_color = MakeColor(1.0f, 1.0f, 0.0f);
	Vec4 normal_color = MakeColor(1.0f, 1.0f, 1.0f);
	Int32 base_line_y = top + TooltipTopPadding + TextPixelsAboveBaseLine;
	Int32 text_left = left + TooltipPadding;
	for(Int32 i = 0; i < line_n; i++)
	{
		Vec4 color = (i == 0) ? title_color : normal_color;
		Int8* line = lines[i];
		DrawBitmapTextLine(bitmap, line, glyph_data, text_left, base_line_y, color);
		base_line_y += TextHeightInPixels;
	}
}

static void
func DrawBitmapTooltipBottom(Bitmap *bitmap, Int8 **lines, Int32 line_n, 
							 GlyphData *glyph_data, Int32 bottom, Int32 left)
{
	Int32 top = bottom - GetTooltipHeight(line_n);
	DrawBitmapTooltip(bitmap, lines, line_n, glyph_data, top, left);
}

static void
func DrawBitmapTooltipBottomRight(Bitmap *bitmap, Int8 **lines, Int32 line_n, 
								  GlyphData *glyph_data, Int32 bottom, Int32 right)
{
	Int32 top = bottom - GetTooltipHeight(line_n);
	Int32 left = right - TooltipWidth;
	DrawBitmapTooltip(bitmap, lines, line_n, glyph_data, top, left);
}