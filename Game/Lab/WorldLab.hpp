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
	I32 row;
	I32 col;
	Bitmap bitmap;
};

struct WorldLabState
{
	B32 running;
	Canvas canvas;
	Camera camera;
	
	F32 randomTable[WorldLabRandomTableN];

	GridBitmap bitmaps[9];
	I32 bitmapN;

	I32 gridLevel;
};
WorldLabState gWorldLabState;

static void InitRandomTable(WorldLabState* labState)
{
	for (I32 i = 0; i < WorldLabRandomTableN; ++i)
	{
		labState->randomTable[i] = RandomBetween(-1.0f, 0.0f);
	}
}

static void WorldLabResize(WorldLabState* labState, I32 width, I32 height)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	ResizeBitmap(bitmap, width, height);
}

static F32 SmoothRatio(F32 x)
{
	return x * x * (3.0f - 2.0f * x);
}

static F32 GetRandomValueFromTable(WorldLabState* labState, I32 index)
{
	Assert(index >= 0);
	F32 value = labState->randomTable[index % WorldLabRandomTableN];
	return value;
}

static F32 GetGridPointValue(WorldLabState* labState, I32 gridLevel, I32 row, I32 col)
{
	I32 gridN = (1 << gridLevel);
	Assert(IsIntBetween(row, 0, gridN));
	Assert(IsIntBetween(col, 0, gridN));

	I32 firstIndexOnLevel = 0;
	for (I32 i = 0; i < gridLevel; ++i)
	{
		firstIndexOnLevel += IntSquare((1 << i) + 1);
	}

	F32 value = GetRandomValueFromTable(labState, firstIndexOnLevel + (gridN + 1) * row + col);
	return value;
}

static F32 GetPointValue(WorldLabState* labState, I32 gridLevel, F32 x, F32 y)
{
	I32 gridN = (1 << gridLevel);
	Assert(IsBetween(x, 0.0f, 1.0f));
	Assert(IsBetween(y, 0.0f, 1.0f));

	I32 left = Floor(x * gridN);
	I32 right = left + 1;
	I32 top = Floor(y * gridN);
	I32 bottom = top + 1;

	F32 ratioX = SmoothRatio(Fraction(x * gridN));
	F32 ratioY = SmoothRatio(Fraction(y * gridN));

	F32 topLeftValue = GetGridPointValue(labState, gridLevel, top, left);
	F32 topRightValue = GetGridPointValue(labState, gridLevel, top, right);
	F32 bottomLeftValue = GetGridPointValue(labState, gridLevel, bottom, left);
	F32 bottomRightValue = GetGridPointValue(labState, gridLevel, bottom, right);

	F32 topValue = Lerp(topLeftValue, ratioX, topRightValue);
	F32 bottomValue = Lerp(bottomLeftValue, ratioX, bottomRightValue);

	F32 value = Lerp(topValue, ratioY, bottomValue);

	return value;
}

static void GenerateGridBitmap(WorldLabState* labState, I32 gridLevel, I32 row, I32 col, Bitmap* bitmap)
{
	ResizeBitmap(bitmap, 512, 512);

	// TODO: Use a memory arena!
	F32* values = new F32[(bitmap->width) * (bitmap->height)];
	I32 valueIndex = 0;
	for (I32 row = 0; row < bitmap->height; ++row)
	{
		for (I32 col = 0; col < bitmap->width; ++col)
		{
			values[valueIndex] = 0.0f;
			valueIndex++;
		}
	}

	F32 gridWidth = 1.0f / (1 << gridLevel);
	F32 left = col * gridWidth;
	F32 right = (col + 1) * gridWidth;
	F32 top = row * gridWidth;
	F32 bottom = (row + 1) * gridWidth;

	F32 valueRatio = 1.0f;
	for (I32 level = 2; level <= 8; ++level)
	{
		V4 color1 = MakeColor(0.0f, 0.0f, 0.0f);
		V4 color2 = MakeColor(1.0f, 1.0f, 1.0f);

		I32 valueIndex = 0;
		for (I32 row = 0; row < bitmap->height; ++row)
		{
			for (I32 col = 0; col < bitmap->width; ++col)
			{
				F32 x = left + (F32(col) / F32(bitmap->width)) * (right - left);
				F32 y = top + (F32(row) / F32(bitmap->height)) * (bottom - top);
				F32 value = GetPointValue(labState, level, x, y);

				values[valueIndex] += value * valueRatio;
				valueIndex++;
			}
		}

		valueRatio *= WorldLabLevelSwitchRatio;
	}

	valueIndex = 0;
	for (I32 row = 0; row < bitmap->height; ++row)
	{
		for (I32 col = 0; col < bitmap->width; ++col)
		{
			F32 oldValue = values[valueIndex];
			F32 newValue = SmoothRatio((oldValue / WorldLabMaxValue) + 1.0f) * 0.5f;
			Assert(IsBetween(newValue, 0.0f, 1.0f));
			newValue = SmoothRatio(newValue);
			values[valueIndex] = newValue;
			valueIndex++;
		}
	}

	U32* pixel = bitmap->memory;
	V4 darkBlue = MakeColor(0.0f, 0.0f, 0.3f);
	V4 lightBlue = MakeColor(0.0f, 0.0f, 0.8f);
	V4 darkGreen = MakeColor(0.0f, 0.3f, 0.0f);
	V4 lightGreen = MakeColor(0.0f, 0.8f, 0.0f);
	V4 lightBrown = MakeColor(0.3f, 0.4f, 0.0f);
	V4 darkBrown = MakeColor(0.2f, 0.3f, 0.0f);
	V4 lightWhite = MakeColor(0.8f, 0.8f, 0.8f);
	V4 darkWhite = MakeColor(0.3f, 0.3f, 0.3f);

	valueIndex = 0;
	for (int row = 0; row < bitmap->height; ++row)
	{
		for (int col = 0; col < bitmap->width; ++col)
		{
			F32 value = values[valueIndex];
			valueIndex++;
			Assert(IsBetween(value, 0.0f, 1.0f));

			V4 color = {};

			F32 seaLevel = 0.18f;
			F32 hillLevel = 0.35f;
			F32 mountainLevel = 0.4f;
			F32 topLevel = 0.5f;

			if (IsBetween(value, 0.0f, seaLevel))
			{
				color = InterpolateColors(darkBlue, (value - 0.0f) / (seaLevel - 0.0f), lightBlue);
			}
			else if (IsBetween(value, seaLevel, hillLevel))
			{
				color = InterpolateColors(lightGreen, (value - seaLevel) / (hillLevel - seaLevel), darkGreen);
			}
			else if (IsBetween(value, hillLevel, mountainLevel))
			{
				color = InterpolateColors(darkBrown, (value - hillLevel) / (mountainLevel - hillLevel), lightBrown);
			}
			else if (IsBetween(value, mountainLevel, topLevel))
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

static void LoadGridBitmap(WorldLabState* labState, I32 row, I32 col)
{
	Assert(labState->bitmapN < 9);
	GridBitmap* gridBitmap = &labState->bitmaps[labState->bitmapN];
	labState->bitmapN++;
	gridBitmap->row = row;
	gridBitmap->col = col;

	Bitmap* bitmap = &gridBitmap->bitmap;
	GenerateGridBitmap(labState, labState->gridLevel, row, col, bitmap);
}

static void WorldLabInit(WorldLabState* labState, I32 width, I32 height)
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
	camera->screenPixelSize.x = F32(width);
	camera->screenPixelSize.y = F32(height);
	canvas->camera = camera;
}

static void WorldLabBlit(WorldLabState* labState, HDC context, RECT rect)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	StretchDIBits(context,
				  0, 0, bitmap->width, bitmap->height,
				  0, 0, width, height,
				  bitmap->memory,
				  &bitmap->info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static LRESULT CALLBACK WorldLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	WorldLabState* labState = &gWorldLabState;
	switch (message)
	{
		case WM_SIZE:
		{
			RECT rect = {};
			GetClientRect(window, &rect);
			I32 width = rect.right - rect.left;
			I32 height = rect.bottom - rect.top;
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

			switch (keyCode)
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

static void WorldLabUpdate(WorldLabState* labState, V2 mouse)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	F32 gridSize = 500.0f;
	F32 gridLeft = -gridSize * 0.5f;
	F32 gridRight = gridSize * 0.5f;
	F32 gridTop = -gridSize * 0.5f;
	F32 gridBottom = gridSize * 0.5f;

	F32 gridCenterX = 0.0f;
	F32 gridCenterY = 0.0f;

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;

	V4 bitmapBorderColor = MakeColor(1.0f, 1.0f, 0.0f);

	for (I32 i = 0; i < labState->bitmapN; ++i)
	{
		GridBitmap* gridBitmap = &labState->bitmaps[i];
		F32 gridCellSize = (gridSize) / (1 << labState->gridLevel);

		F32 left = gridLeft + gridCellSize * gridBitmap->col;
		F32 right = left + gridCellSize;
		F32 top = gridTop + gridCellSize * gridBitmap->row;
		F32 bottom = top + gridCellSize;

		DrawStretchedBitmap(*canvas, &gridBitmap->bitmap, left, right, top, bottom);
		DrawRectOutline(*canvas, left, right, top, bottom, bitmapBorderColor);
	}

	F32 screenPixelSize = 600.0f;
	F32 halfScreenSize = (screenPixelSize * 0.5f) / camera->unitInPixels;

	F32 centerX = (CameraLeftSide(camera) + CameraRightSide(camera)) * 0.5f;
	F32 centerY = (CameraTopSide(camera) + CameraBottomSide(camera)) * 0.5f;
	F32 left = centerX - halfScreenSize;
	F32 right = centerX +  halfScreenSize;
	F32 top = centerY - halfScreenSize;
	F32 bottom = centerY + halfScreenSize;

	V4 screenBorderColor = MakeColor(1.0f, 0.0f, 0.0f);
	DrawRectOutline(*canvas, left, right, top, bottom, screenBorderColor);

	F32 screenUnitSize = screenPixelSize / camera->unitInPixels;
	F32 bitmapUnitSize = (gridRight - gridLeft) / (1 << labState->gridLevel);

	B32 reloadBitmaps = false;
	if (2.0f * bitmapUnitSize < screenUnitSize)
	{
		if (labState->gridLevel > 0)
		{
			labState->gridLevel--;
			reloadBitmaps = true;
		}
	} 
	else if (bitmapUnitSize > screenUnitSize)
	{
		labState->gridLevel++;
		reloadBitmaps = true;
	}

	if (reloadBitmaps)
	{
		F32 screenLeft   = -screenUnitSize * 0.5f;
		F32 screenRight  = +screenUnitSize * 0.5f;
		F32 screenTop    = -screenUnitSize * 0.5f;
		F32 screenBottom = +screenUnitSize * 0.5f;

		F32 leftRatio   = (screenLeft - gridLeft) / gridSize;
		F32 rightRatio  = (screenRight - gridLeft) / gridSize;
		F32 topRatio    = (screenTop - gridTop) / gridSize;
		F32 bottomRatio = (screenBottom - gridTop) / gridSize;

		I32 gridN = (1 << labState->gridLevel);
		I32 leftCol   = Floor(leftRatio * gridN);
		I32 rightCol  = Floor(rightRatio * gridN);
		I32 topRow    = Floor(topRatio * gridN);
		I32 bottomRow = Floor(bottomRatio * gridN);

		leftCol   = IntMax2(leftCol, 0);
		rightCol  = IntMin2(rightCol, gridN - 1);
		topRow    = IntMax2(topRow, 0);
		bottomRow = IntMin2(bottomRow, gridN - 1);

		I32 bitmapN = (rightCol - leftCol + 1) * (bottomRow - topRow + 1);
		Assert(bitmapN <= 9);

		labState->bitmapN = 0;
		for (I32 row = topRow; row <= bottomRow; ++row)
		{
			for (I32 col = leftCol; col <= rightCol; ++col)
			{
				LoadGridBitmap(labState, row, col);
			}
		}
	}
}

void WorldLab(HINSTANCE instance)
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
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	WorldLabInit(labState, width, height);

	MSG message = {};
	while (labState->running)
	{
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		V2 mouse = GetMousePosition(&labState->camera, window);
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