#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Draw.hpp"
#include "../Text.hpp"

struct TextLabState
{
	Camera camera;
	Canvas canvas;

	Bool32 running;
};
static TextLabState gTextLabState;

static void func TextLabResize(TextLabState* labState, Int32 width, Int32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 2.0f;
}

static void func TextLabBlit(Canvas* canvas, HDC context, RECT rect)
{
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas->bitmap;
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

static void func TextLabInit(TextLabState* labState, Int32 windowWidth, Int32 windowHeight)
{
	labState->running = true;
	TextLabResize(labState, windowWidth, windowHeight);

	labState->canvas.glyphData = GetGlobalGlyphData();
}

static void func DrawTestToolTip(Canvas* canvas)
{
	char* lines[] =
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

static void func TextLabUpdate(TextLabState* labState)
{
	Canvas* canvas = &labState->canvas;
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

static LRESULT CALLBACK func TextLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	TextLabState* labState = &gTextLabState;
	LRESULT result = 0;

	switch(message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			Int32 width = clientRect.right - clientRect.left;
			Int32 height = clientRect.bottom - clientRect.top;
			TextLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			TextLabBlit(&labState->canvas, context, clientRect);

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

static void func TextLab(HINSTANCE instance)
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
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;
	TextLabInit(labState, width, height);

	MSG message = {};
	while(labState->running)
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		TextLabUpdate(labState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		TextLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context); 
	}
}

// TODO: Canvas.bitmap should be a pointer?