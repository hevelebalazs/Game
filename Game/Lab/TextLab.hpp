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
	I8 *lines[] =
	{
		"LMB",
		"Use time: 0.5 sec",
		"Hit in a direction, dealing 30 damage",
		"to the first enemy hit.",
		"Generates 20 Energy if it hits anything."
	};
	I32 line_n = sizeof(lines) / sizeof(char*);

	Bitmap *bitmap = &canvas->bitmap;
	I32 tooltip_left = (bitmap->width / 2) - (TooltipWidth / 2);
	I32 tooltip_top = (bitmap->height / 2) - (GetTooltipHeight(line_n) / 2);
	DrawBitmapTooltip(bitmap, lines, line_n, canvas->glyph_data, tooltip_top, tooltip_left);
}

static void
func TextLabUpdate(Canvas *canvas)
{
	V4 background_color = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, background_color);

	V4 top_left_color     = MakeColor(1.0f, 0.0f, 0.0f);
	V4 top_right_color    = MakeColor(0.0f, 1.0f, 0.0f);
	V4 bottom_left_color  = MakeColor(0.0f, 0.0f, 1.0f);
	V4 bottom_right_color = MakeColor(1.0f, 0.0f, 0.0f);

	Bitmap *bitmap = &canvas->bitmap;
	U32 *pixel = bitmap->memory;
	for(I32 row = 0; row < bitmap->height; row++)
	{
		R32 y_ratio = R32(row) / R32(bitmap->height - 1);

		V4 left_color = InterpolateColors(top_left_color, y_ratio, bottom_left_color);
		V4 right_color = InterpolateColors(top_right_color, y_ratio, bottom_right_color);

		for(I32 col = 0; col < bitmap->width; col++)
		{
			R32 x_ratio = R32(col) / R32(bitmap->width - 1);

			V4 color = InterpolateColors(left_color, x_ratio, right_color);
			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	Camera *camera = canvas->camera;
	R32 base_line_y = 0.5f * (CameraTopSide(camera) + CameraBottomSide(camera));
	R32 base_line_left = CameraLeftSide(camera);
	R32 base_line_right = CameraRightSide(camera);

	V4 text_color = MakeColor(0.0f, 0.0f, 0.0f);

	DrawTestToolTip(canvas);
}