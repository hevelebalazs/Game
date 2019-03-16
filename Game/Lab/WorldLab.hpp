#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"

#define WorldLabRandomTableN 16384
#define WorldLabLevelSwitchRatio 0.67f
#define WorldLabMaxValue (1.0f / (1.0f - WorldLabLevelSwitchRatio))

struct GridBitmap
{
	Int32 row;
	Int32 col;
	Bitmap bitmap;
};

struct WorldLabState
{
	Bool32 running;
	Canvas canvas;
	Camera camera;
	
	Real32 randomTable[WorldLabRandomTableN];

	GridBitmap bitmaps[9];
	Int32 bitmapN;

	Int32 gridLevel;
};
WorldLabState gWorldLabState;

static void func InitRandomTable(WorldLabState* labState)
{
	for (Int32 i = 0; i < WorldLabRandomTableN; ++i)
	{
		labState->randomTable[i] = RandomBetween(-1.0f, 0.0f);
	}
}

static void func WorldLabResize(WorldLabState* labState, Int32 width, Int32 height)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	ResizeBitmap (bitmap, width, height);
}

static Real32 func SmoothRatio(Real32 x)
{
	Real32 result = x * x * (3.0f - 2.0f * x);
	return result;
}

static Real32 func GetRandomValueFromTable(WorldLabState* labState, Int32 index)
{
	Assert(index >= 0);
	Real32 value = labState->randomTable[index % WorldLabRandomTableN];
	return value;
}

static Real32 func GetGridPointValue(WorldLabState* labState, Int32 gridLevel, Int32 row, Int32 col)
{
	Int32 gridN = (1 << gridLevel);
	Assert(IsIntBetween(row, 0, gridN));
	Assert(IsIntBetween(col, 0, gridN));

	Int32 firstIndexOnLevel = 0;
	for(Int32 i = 0; i < gridLevel; ++i)
	{
		firstIndexOnLevel += IntSquare ((1 << i) + 1);
	}

	Real32 value = GetRandomValueFromTable(labState, firstIndexOnLevel + (gridN + 1) * row + col);
	return value;
}

static Real32 func GetPointValue(WorldLabState* labState, Int32 gridLevel, Real32 x, Real32 y)
{
	Int32 gridN = (1 << gridLevel);
	Assert(IsBetween(x, 0.0f, 1.0f));
	Assert(IsBetween(y, 0.0f, 1.0f));

	Int32 left = Floor(x * gridN);
	Int32 right = left + 1;
	Int32 top = Floor(y * gridN);
	Int32 bottom = top + 1;

	Real32 ratioX = SmoothRatio(Fraction(x * gridN));
	Real32 ratioY = SmoothRatio(Fraction(y * gridN));

	Real32 topLeftValue     = GetGridPointValue(labState, gridLevel, top, left);
	Real32 topRightValue    = GetGridPointValue(labState, gridLevel, top, right);
	Real32 bottomLeftValue  = GetGridPointValue(labState, gridLevel, bottom, left);
	Real32 bottomRightValue = GetGridPointValue(labState, gridLevel, bottom, right);

	Real32 topValue    = Lerp(topLeftValue, ratioX, topRightValue);
	Real32 bottomValue = Lerp(bottomLeftValue, ratioX, bottomRightValue);

	Real32 value = Lerp(topValue, ratioY, bottomValue);

	return value;
}

static void func GenerateGridBitmap(WorldLabState* labState, Int32 gridLevel, Int32 row, Int32 col, Bitmap* bitmap)
{
	ResizeBitmap(bitmap, 512, 512);

	// TODO: Use a memory arena!
	Real32* values = new Real32[bitmap->width * bitmap->height];
	Int32 valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; ++row)
	{
		for(Int32 col = 0; col < bitmap->width; ++col)
		{
			values[valueIndex] = 0.0f;
			valueIndex++;
		}
	}

	Real32 gridWidth = 1.0f / (1 << gridLevel);
	Real32 left = col * gridWidth;
	Real32 right = (col + 1) * gridWidth;
	Real32 top = row * gridWidth;
	Real32 bottom = (row + 1) * gridWidth;

	Real32 valueRatio = 1.0f;
	for(Int32 level = 2; level <= 8; level++)
	{
		Vec4 color1 = MakeColor(0.0f, 0.0f, 0.0f);
		Vec4 color2 = MakeColor(1.0f, 1.0f, 1.0f);

		Int32 valueIndex = 0;
		for(Int32 row = 0; row < bitmap->height; row++)
		{
			for(Int32 col = 0; col < bitmap->width; col++)
			{
				Real32 x = left + ((Real32)col / (Real32)bitmap->width) * (right - left);
				Real32 y = top + ((Real32)row / (Real32)bitmap->height) * (bottom - top);
				Real32 value = GetPointValue(labState, level, x, y);

				values[valueIndex] += value * valueRatio;
				valueIndex++;
			}
		}

		valueRatio *= WorldLabLevelSwitchRatio;
	}

	valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 oldValue = values[valueIndex];
			Real32 newValue = SmoothRatio((oldValue / WorldLabMaxValue) + 1.0f) * 0.5f;
			Assert(IsBetween(newValue, 0.0f, 1.0f));
			newValue = SmoothRatio(newValue);
			values[valueIndex] = newValue;
			valueIndex++;
		}
	}

	UInt32* pixel = bitmap->memory;
	Vec4 darkBlue = MakeColor(0.0f, 0.0f, 0.3f);
	Vec4 lightBlue = MakeColor(0.0f, 0.0f, 0.8f);
	Vec4 darkGreen = MakeColor(0.0f, 0.3f, 0.0f);
	Vec4 lightGreen = MakeColor(0.0f, 0.8f, 0.0f);
	Vec4 lightBrown = MakeColor(0.3f, 0.4f, 0.0f);
	Vec4 darkBrown = MakeColor(0.2f, 0.3f, 0.0f);
	Vec4 lightWhite = MakeColor(0.8f, 0.8f, 0.8f);
	Vec4 darkWhite = MakeColor(0.3f, 0.3f, 0.3f);

	valueIndex = 0;
	for(Int32 row = 0; row < bitmap->height; row++)
	{
		for(Int32 col = 0; col < bitmap->width; col++)
		{
			Real32 value = values[valueIndex];
			valueIndex++;
			Assert(IsBetween(value, 0.0f, 1.0f));

			Vec4 color = {};

			Real32 seaLevel = 0.18f;
			Real32 hillLevel = 0.35f;
			Real32 mountainLevel = 0.4f;
			Real32 topLevel = 0.5f;

			if(IsBetween(value, 0.0f, seaLevel))
			{
				color = InterpolateColors(darkBlue, (value - 0.0f) / (seaLevel - 0.0f), lightBlue);
			}
			else if(IsBetween(value, seaLevel, hillLevel))
			{
				color = InterpolateColors(lightGreen, (value - seaLevel) / (hillLevel - seaLevel), darkGreen);
			}
			else if(IsBetween(value, hillLevel, mountainLevel))
			{
				color = InterpolateColors(darkBrown, (value - hillLevel) / (mountainLevel - hillLevel), lightBrown);
			}
			else if(IsBetween(value, mountainLevel, topLevel))
			{
				color = InterpolateColors(darkWhite, (value - mountainLevel) / (topLevel - mountainLevel), lightWhite);
			}
			else
			{
				DebugBreak();
			}

			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	delete[] values;
}

static void func LoadGridBitmap(WorldLabState* labState, Int32 row, Int32 col)
{
	Assert(labState->bitmapN < 9);
	GridBitmap* gridBitmap = &labState->bitmaps[labState->bitmapN];
	labState->bitmapN++;
	gridBitmap->row = row;
	gridBitmap->col = col;

	Bitmap* bitmap = &gridBitmap->bitmap;
	GenerateGridBitmap(labState, labState->gridLevel, row, col, bitmap);
}

static void func WorldLabInit(WorldLabState* labState, Int32 width, Int32 height)
{
	InitRandom();

	labState->running = true;
	WorldLabResize(labState, width, height);

	InitRandomTable(labState);
	labState->bitmapN = 0;
	labState->gridLevel = 0;
	LoadGridBitmap(labState, 0, 0);

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;
	camera->unitInPixels = 1.0f;
	camera->center.x = 0.0f;
	camera->center.y = 0.0f;
	camera->screenPixelSize.x = (Real32)width;
	camera->screenPixelSize.y = (Real32)height;
	canvas->camera = camera;
}

static void func WorldLabBlit(WorldLabState* labState, HDC context, RECT rect)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;

	BITMAPINFO bitmapInfo = GetBitmapInfo(bitmap);
	StretchDIBits(context,
				  0, 0, bitmap->width, bitmap->height,
				  0, 0, width, height,
				  bitmap->memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static LRESULT CALLBACK WorldLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	WorldLabState* labState = &gWorldLabState;
	switch(message)
	{
		case WM_SIZE:
		{
			RECT rect = {};
			GetClientRect(window, &rect);
			Int32 width = rect.right - rect.left;
			Int32 height = rect.bottom - rect.top;
			WorldLabResize(labState, width, height);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT rect = {};
			GetClientRect(window, &rect);

			WorldLabBlit(labState, context, rect);

			EndPaint(window, &paint);
			break;
		}
		case WM_KEYUP:
		{
			WPARAM keyCode = wparam;
			switch(keyCode)
			{
				case 'W':
				{
					labState->camera.unitInPixels /= 0.9f;
					break;
				}
				case 'S':
				{
					labState->camera.unitInPixels *= 0.9f;
					break;
				}
			}

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

static void func WorldLabUpdate(WorldLabState* labState, Vec2 mouse)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Real32 gridSize = 500.0f;
	Real32 gridLeft   = -gridSize * 0.5f;
	Real32 gridRight  = +gridSize * 0.5f;
	Real32 gridTop    = -gridSize * 0.5f;
	Real32 gridBottom = +gridSize * 0.5f;

	Real32 gridCenterX = 0.0f;
	Real32 gridCenterY = 0.0f;

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;

	Vec4 bitmapBorderColor = MakeColor(1.0f, 1.0f, 0.0f);

	for(Int32 i = 0; i < labState->bitmapN; i++)
	{
		GridBitmap* gridBitmap = &labState->bitmaps[i];
		Real32 gridCellSize = (gridSize) / (1 << labState->gridLevel);

		Real32 left   = gridLeft + gridCellSize * gridBitmap->col;
		Real32 right  = left + gridCellSize;
		Real32 top    = gridTop + gridCellSize * gridBitmap->row;
		Real32 bottom = top + gridCellSize;

		DrawStretchedBitmap(canvas, &gridBitmap->bitmap, left, right, top, bottom);
		DrawRectLRTBOutline(canvas, left, right, top, bottom, bitmapBorderColor);
	}

	Real32 screenPixelSize = 600.0f;
	Real32 halfScreenSize = (screenPixelSize * 0.5f) / camera->unitInPixels;

	Real32 centerX = (CameraLeftSide(camera) + CameraRightSide(camera)) * 0.5f;
	Real32 centerY = (CameraTopSide(camera) + CameraBottomSide(camera)) * 0.5f;
	Real32 left   = centerX - halfScreenSize;
	Real32 right  = centerX + halfScreenSize;
	Real32 top    = centerY - halfScreenSize;
	Real32 bottom = centerY + halfScreenSize;

	Vec4 screenBorderColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawRectLRTBOutline(canvas, left, right, top, bottom, screenBorderColor);

	Real32 screenUnitSize = screenPixelSize / camera->unitInPixels;
	Real32 bitmapUnitSize = (gridRight - gridLeft) / (1 << labState->gridLevel);

	Bool32 reloadBitmaps = false;
	if(2.0f * bitmapUnitSize < screenUnitSize)
	{
		if(labState->gridLevel > 0)
		{
			labState->gridLevel--;
			reloadBitmaps = true;
		}
	} 
	else if(bitmapUnitSize > screenUnitSize)
	{
		labState->gridLevel++;
		reloadBitmaps = true;
	}

	if(reloadBitmaps)
	{
		Real32 screenLeft   = -screenUnitSize * 0.5f;
		Real32 screenRight  = +screenUnitSize * 0.5f;
		Real32 screenTop    = -screenUnitSize * 0.5f;
		Real32 screenBottom = +screenUnitSize * 0.5f;

		Real32 leftRatio   = (screenLeft - gridLeft) / gridSize;
		Real32 rightRatio  = (screenRight - gridLeft) / gridSize;
		Real32 topRatio    = (screenTop - gridTop) / gridSize;
		Real32 bottomRatio = (screenBottom - gridTop) / gridSize;

		Int32 gridN = (1 << labState->gridLevel);
		Int32 leftCol   = Floor(leftRatio * gridN);
		Int32 rightCol  = Floor(rightRatio * gridN);
		Int32 topRow    = Floor(topRatio * gridN);
		Int32 bottomRow = Floor(bottomRatio * gridN);

		leftCol   = IntMax2(leftCol, 0);
		rightCol  = IntMin2(rightCol, gridN - 1);
		topRow    = IntMax2(topRow, 0);
		bottomRow = IntMin2(bottomRow, gridN - 1);

		Int32 bitmapN = (rightCol - leftCol + 1) * (bottomRow - topRow + 1);
		Assert (bitmapN <= 9);

		labState->bitmapN = 0;
		for(Int32 row = topRow; row <= bottomRow; row++)
		{
			for(Int32 col = leftCol; col <= rightCol; col++)
			{
				LoadGridBitmap (labState, row, col);
			}
		}
	}
}

static void func WorldLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = WorldLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "WorldLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"WorldLab",
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

	WorldLabState* labState = &gWorldLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;
	WorldLabInit(labState, width, height);

	MSG message = {};
	while(labState->running)
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		Vec2 mouse = GetMousePosition(&labState->camera, window);
		WorldLabUpdate(labState, mouse);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		WorldLabBlit(labState, context, rect);
		ReleaseDC(window, context);
	}
}

// [TODO: Make map zoomable!]
// TODO: Optimize bitmap generation for speed!
// TODO: Make water-ground edges look better!
// TODO: Remove inline functions from project!
// TODO: Pass canvas by address in project!