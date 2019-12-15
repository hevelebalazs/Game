#pragma once

#include <Windows.h>

#include "../Draw.hpp"
#include "../Text.hpp"
#include "../UserInput.hpp"

static void
func TextLabInit(Canvas *canvas)
{
	canvas->glyph_data = GetGlobalGlyphData();
	canvas->camera->unit_in_pixels = 2.0f;
}

static void
func DrawTestToolTip(Canvas *canvas)
{
	Int8 *lines[] =
	{
		"LMB",
		"Use time: 0.5 sec",
		"Hit in a direction, dealing 30 damage",
		"to the first enemy hit.",
		"Generates 20 Energy if it hits anything."
	};
	Int32 line_n = sizeof(lines) / sizeof(char*);

	Bitmap *bitmap = &canvas->bitmap;
	Int32 tooltip_left = (bitmap->width / 2) - (TooltipWidth / 2);
	Int32 tooltip_top = (bitmap->height / 2) - (GetTooltipHeight(line_n) / 2);
	DrawBitmapTooltip(bitmap, lines, line_n, canvas->glyph_data, tooltip_top, tooltip_left);
}

static void
func TextLabUpdate(Canvas *canvas)
{
	Vec4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, background_color);

	Vec4 top_left_color     = MakeColor(1.0f, 0.0f, 0.0f);
	Vec4 top_right_color    = MakeColor(0.0f, 1.0f, 0.0f);
	Vec4 bottom_left_color  = MakeColor(0.0f, 0.0f, 1.0f);
	Vec4 bottom_right_color = MakeColor(1.0f, 0.0f, 0.0f);

	Bitmap *bitmap = &canvas->bitmap;
	UInt32 *pixel = bitmap->memory;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		Real32 y_ratio = Real32(row) / Real32(bitmap->height - 1);

		Vec4 left_color = InterpolateColors(top_left_color, y_ratio, bottom_left_color);
		Vec4 right_color = InterpolateColors(top_right_color, y_ratio, bottom_right_color);

		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 x_ratio = Real32(col) / Real32(bitmap->width - 1);

			Vec4 color = InterpolateColors(left_color, x_ratio, right_color);
			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	Camera *camera = canvas->camera;
	Real32 base_line_y = 0.5f * (CameraTopSide(camera) + CameraBottomSide(camera));
	Real32 base_line_left = CameraLeftSide(camera);
	Real32 base_line_right = CameraRightSide(camera);

	Vec4 text_color = MakeColor(0.0f, 0.0f, 0.0f);

	DrawTestToolTip(canvas);
}