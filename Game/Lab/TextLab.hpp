#pragma once

#include <Windows.h>

#include "../Draw.hpp"
#include "../Text.hpp"
#include "../UserInput.hpp"

static void
func TextLabInit(Canvas* canvas)
{
	canvas->glyphData = GetGlobalGlyphData();
	canvas->camera->unitInPixels = 2.0f;
}

static void
func DrawTestToolTip(Canvas* canvas)
{
	Int8* lines[] =
	{
		"LMB",
		"Use time: 0.5 sec",
		"Hit in a direction, dealing 30 damage",
		"to the first enemy hit.",
		"Generates 20 Energy if it hits anything."
	};
	Int32 lineN = sizeof(lines) / sizeof(char*);

	Bitmap* bitmap = &canvas->bitmap;
	Int32 tooltipLeft = (bitmap->width / 2) - (TooltipWidth / 2);
	Int32 tooltipTop = (bitmap->height / 2) - (GetTooltipHeight(lineN) / 2);
	DrawBitmapTooltip(bitmap, lines, lineN, canvas->glyphData, tooltipTop, tooltipLeft);
}

static void
func TextLabUpdate(Canvas* canvas)
{
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	Vec4 topLeftColor     = MakeColor(1.0f, 0.0f, 0.0f);
	Vec4 topRightColor    = MakeColor(0.0f, 1.0f, 0.0f);
	Vec4 bottomLeftColor  = MakeColor(0.0f, 0.0f, 1.0f);
	Vec4 bottomRightColor = MakeColor(1.0f, 0.0f, 0.0f);

	Bitmap* bitmap = &canvas->bitmap;
	UInt32* pixel = bitmap->memory;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		Real32 yRatio = Real32(row) / Real32(bitmap->height - 1);

		Vec4 leftColor = InterpolateColors(topLeftColor, yRatio, bottomLeftColor);
		Vec4 rightColor = InterpolateColors(topRightColor, yRatio, bottomRightColor);

		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 xRatio = Real32(col) / Real32(bitmap->width - 1);

			Vec4 color = InterpolateColors(leftColor, xRatio, rightColor);
			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	Camera* camera = canvas->camera;
	Real32 baseLineY = 0.5f * (CameraTopSide(camera) + CameraBottomSide(camera));
	Real32 baseLineLeft = CameraLeftSide(camera);
	Real32 baseLineRight = CameraRightSide(camera);

	Vec4 textColor = MakeColor(0.0f, 0.0f, 0.0f);

	DrawTestToolTip(canvas);
}