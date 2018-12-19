#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Draw.hpp"
#include "../Text.hpp"

struct TextLabState
{
	Camera camera;
	Canvas canvas;

	B32 running;
};
static TextLabState gTextLabState;

static void TextLabResize(TextLabState* labState, I32 width, I32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 2.0f;
}

static void TextLabBlit(Canvas canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas.bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo(&bitmap);
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void TextLabInit(TextLabState* labState, I32 windowWidth, I32 windowHeight)
{
	labState->running = true;
	TextLabResize(labState, windowWidth, windowHeight);

	labState->canvas.glyphData = GetGlobalGlyphData();
}

static void DrawToolTip(Canvas canvas)
{
	GlyphData* glyphData = canvas.glyphData;
	Assert(glyphData != 0);

	Camera* camera = canvas.camera;
	Assert(camera->unitInPixels > 0.0f);
	F32 width = 300.0f / camera->unitInPixels;
	F32 centerX = (CameraLeftSide(camera) + CameraRightSide(camera)) * 0.5f;
	F32 centerY = (CameraTopSide(camera) + CameraBottomSide(camera)) * 0.5f;
	F32 left = centerX - width * 0.5f;
	F32 right = centerX + width * 0.5f;

	F32 padding = 5.0f / camera->unitInPixels;
	F32 topPadding = 8.0f / camera->unitInPixels;
	
	char* lines[] =
	{
		"LMB",
		"Use time: 0.5 sec",
		"Hit in a direction, dealing 30 damage",
		"to the first enemy hit.",
		"Generates 20 Energy if it hits anything."
	};
	I32 lineN = sizeof(lines) / sizeof(char*);

	F32 tooltipHeight = topPadding + lineN * (TextHeightInPixels / camera->unitInPixels) + padding;
	F32 top = centerY - tooltipHeight * 0.5f;
	F32 bottom = centerY + tooltipHeight * 0.5f;

	V4 toolTipColor = MakeColor(0.0f, 0.0f, 0.0f);
	DrawRect(canvas, left, right, top, bottom, toolTipColor);

	V4 toolTipBorderColor = MakeColor(1.0f, 1.0f, 1.0f);
	DrawRectOutline(canvas, left, right, top, bottom, toolTipBorderColor);

	V4 titleColor = MakeColor(1.0f, 1.0f, 0.0f);
	V4 normalColor = MakeColor(1.0f, 1.0f, 1.0f);
	F32 baseLineY = top + topPadding + TextPixelsAboveBaseLine / camera->unitInPixels;
	F32 textLeft = left + padding;
	for (I32 i = 0; i < lineN; ++i)
	{
		V4 color = (i == 0) ? titleColor : normalColor;
		char* line = lines[i];
		DrawTextLine(canvas, line, baseLineY, textLeft, color);
		baseLineY += TextHeightInPixels / camera->unitInPixels;
	}
}

static void TextLabUpdate(TextLabState* labState)
{
	Canvas canvas = labState->canvas;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, backgroundColor);

	V4 topLeftColor = MakeColor(1.0f, 0.0f, 0.0f);
	V4 topRightColor = MakeColor(0.0f, 1.0f, 0.0f);
	V4 bottomLeftColor = MakeColor(0.0f, 0.0f, 1.0f);
	V4 bottomRightColor = MakeColor(1.0f, 0.0f, 0.0f);

	Bitmap* bitmap = &canvas.bitmap;
	U32* pixel = bitmap->memory;
	for (I32 row = 0; row < bitmap->height; ++row)
	{
		F32 yRatio = F32(row) / F32(bitmap->height - 1);

		V4 leftColor = InterpolateColors(topLeftColor, yRatio, bottomLeftColor);
		V4 rightColor = InterpolateColors(topRightColor, yRatio, bottomRightColor);

		for (I32 col = 0; col < bitmap->width; ++col)
		{
			F32 xRatio = F32(col) / F32(bitmap->width - 1);

			V4 color = InterpolateColors(leftColor, xRatio, rightColor);
			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	Camera* camera = canvas.camera;
	F32 baseLineY = 0.5f * (CameraTopSide(camera) + CameraBottomSide(camera));
	F32 baseLineLeft = CameraLeftSide(camera);
	F32 baseLineRight = CameraRightSide(camera);

	V4 textColor = MakeColor(0.0f, 0.0f, 0.0f);

	DrawToolTip(canvas);
}

static LRESULT CALLBACK TextLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	TextLabState* labState = &gTextLabState;
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			TextLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			TextLabBlit(labState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
		{
			labState->running = false;
			break;
		}
		default:
		{
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

static void TextLab(HINSTANCE instance)
{
	TextLabState* labState = &gTextLabState;

	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = TextLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "TextLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"TextLab",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		instance,
		0
	);
	Assert(window != 0);

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	TextLabInit(labState, width, height);

	MSG message = {};
	while (labState->running)
	{
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		TextLabUpdate(labState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		TextLabBlit(labState->canvas, context, rect);
		ReleaseDC(window, context); 
	}
}

// TODO: Pass Canvas by pointer!
// TODO: Separate UI rendering from game rendering?