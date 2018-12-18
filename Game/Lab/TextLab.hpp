#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Draw.hpp"
#include "../Text.hpp"

struct TextLabState
{
	Camera camera;
	Canvas canvas;

	GlyphData* glyphData;

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
	camera->unitInPixels = 1.0f;
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

	labState->glyphData = GetGlobalGlyphData();
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

	V4 textColor = MakeColor(0.0f, 1.0f, 1.0f);
	DrawTextLine(canvas, "The quick brown fox jumps over the lazy dog. 1234567890", baseLineY, baseLineLeft, labState->glyphData, textColor);
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