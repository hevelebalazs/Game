#include "WorldLab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"

#define WorldLabRandomTableN 16384
#define WorldLabLevelSwitchRatio 0.67f
#define WorldLabMaxValue (1.0f / (1.0f - WorldLabLevelSwitchRatio))

enum {
	GridTopLeft		= 1,
	GridTopRight	= 2,
	GridBottomLeft  = 3,
	GridBottomRight = 4
};

struct WorldLabState {
	B32 running;
	Canvas canvas;
	Camera camera;
	
	F32 randomTable[WorldLabRandomTableN];

	Bitmap bitmap;

	I32 mouseIndex;

	I32 gridLevel;
	I32 gridRow;
	I32 gridCol;
};
WorldLabState gWorldLabState;

static void InitRandomTable(WorldLabState* labState)
{
	for (I32 i = 0; i < WorldLabRandomTableN; ++i)
		labState->randomTable[i] = RandomBetween(-1.0f, 0.0f);
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
		firstIndexOnLevel += IntSquare((1 << i) + 1);

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
	for (I32 row = 0; row < bitmap->height; ++row) {
		for (I32 col = 0; col < bitmap->width; ++col) {
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
	I32 lastLevel = gridLevel + 8;
	for (I32 level = 3; level <= lastLevel; ++level) {
		V4 color1 = MakeColor(0.0f, 0.0f, 0.0f);
		V4 color2 = MakeColor(1.0f, 1.0f, 1.0f);

		I32 valueIndex = 0;
		for (I32 row = 0; row < bitmap->height; ++row) {
			for (I32 col = 0; col < bitmap->width; ++col) {
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
	for (I32 row = 0; row < bitmap->height; ++row) {
		for (I32 col = 0; col < bitmap->width; ++col) {
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
	for (int row = 0; row < bitmap->height; ++row) {
		for (int col = 0; col < bitmap->width; ++col) {
			F32 value = values[valueIndex];
			valueIndex++;
			Assert(IsBetween(value, 0.0f, 1.0f));

			V4 color = {};

			F32 seaLevel = 0.18f;
			F32 hillLevel = 0.35f;
			F32 mountainLevel = 0.4f;
			F32 topLevel = 0.5f;

			if (IsBetween(value, 0.0f, seaLevel))
				color = InterpolateColors(darkBlue, (value - 0.0f) / (seaLevel - 0.0f), lightBlue);
			else if (IsBetween(value, seaLevel, hillLevel))
				color = InterpolateColors(lightGreen, (value - seaLevel) / (hillLevel - seaLevel), darkGreen);
			else if (IsBetween(value, hillLevel, mountainLevel))
				color = InterpolateColors(darkBrown, (value - hillLevel) / (mountainLevel - hillLevel), lightBrown);
			else if (IsBetween(value, mountainLevel, topLevel))
				color = InterpolateColors(darkWhite, (value - mountainLevel) / (topLevel - mountainLevel), lightWhite);
			else
				DebugBreak();

			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	delete[] values;
}

static void WorldLabInit(WorldLabState* labState, I32 width, I32 height)
{
	InitRandom();

	labState->running = true;
	WorldLabResize(labState, width, height);

	InitRandomTable(labState);
	labState->gridLevel = 0;
	labState->gridRow = 0;
	labState->gridCol = 0;
	GenerateGridBitmap(labState, labState->gridLevel, labState->gridRow, labState->gridCol, &labState->bitmap);

	labState->camera.unitInPixels = 1.0f;
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

static V2 GetMousePosition(Camera* camera, HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	V2 point = {};
	point.x = (F32)cursorPoint.x;
	point.y = (F32)cursorPoint.y;

	point = PixelToUnit(camera, point);

	return point;
}

static LRESULT CALLBACK WorldLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	WorldLabState* labState = &gWorldLabState;
	switch (message) {
		case WM_SIZE: {
			RECT rect = {};
			GetClientRect(window, &rect);
			I32 width = rect.right - rect.left;
			I32 height = rect.bottom - rect.top;
			WorldLabResize(labState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT rect = {};
			GetClientRect(window, &rect);

			WorldLabBlit(labState, context, rect);

			EndPaint(window, &paint);
			break;
		}

		case WM_RBUTTONDOWN: {
			if (labState->gridLevel >= 1) {
				labState->gridLevel--;
				labState->gridRow = labState->gridRow / 2;
				labState->gridCol = labState->gridCol / 2;
				GenerateGridBitmap(labState, labState->gridLevel, labState->gridRow, labState->gridCol, &labState->bitmap);
			}

			break;
		}

		case WM_LBUTTONDOWN: {
			if (labState->gridLevel < 7) {
				B32 regenerateBitmap = true;
				switch (labState->mouseIndex) {
					case GridTopLeft: {
						labState->gridLevel++;
						labState->gridRow = 2 * labState->gridRow;
						labState->gridCol = 2 * labState->gridCol;
						break;
					}
					case GridTopRight: {
						labState->gridLevel++;
						labState->gridRow = 2 * labState->gridRow;
						labState->gridCol = 2 * labState->gridCol + 1;
						break;
					}
					case GridBottomLeft: {
						labState->gridLevel++;
						labState->gridRow = 2 * labState->gridRow + 1;
						labState->gridCol = 2 * labState->gridCol;
						break;
					}
					case GridBottomRight: {
						labState->gridLevel++;
						labState->gridRow = 2 * labState->gridRow + 1;
						labState->gridCol = 2 * labState->gridCol + 1;
						break;
					}
					default: {
						regenerateBitmap = false;
						break;
					}
				}

				if (regenerateBitmap)
					GenerateGridBitmap(labState, labState->gridLevel, labState->gridRow, labState->gridCol, &labState->bitmap);
			}

			break;
		}

		case WM_DESTROY: 
		case WM_CLOSE: {
			labState->running = false;
			break;
		}

		default: {
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

	F32 width = F32(bitmap->width);
	F32 height = F32(bitmap->height);

	F32 gridTotalSize = Min2(width, height) * 0.8f;
	F32 gridLeft = (width - gridTotalSize) * 0.5f;
	F32 gridTop = (height - gridTotalSize) * 0.5f;
	F32 gridRight = gridLeft + gridTotalSize;
	F32 gridBottom = gridTop + gridTotalSize;

	F32 gridCenterX = (gridLeft + gridRight) * 0.5f;
	F32 gridCenterY = (gridTop + gridBottom) * 0.5f;

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;
	camera->center.x = width * 0.5f;
	camera->center.y = height * 0.5f;
	camera->screenPixelSize.x = width;
	camera->screenPixelSize.y = height;
	camera->targetUnitInPixels = 1.0f;
	canvas->camera = camera;

	DrawStretchedBitmap(*canvas, &labState->bitmap, gridLeft, gridRight, gridTop, gridBottom);

	labState->mouseIndex = 0;

	B32 mouseLeft   = IsBetween(mouse.x, gridLeft, gridCenterX);
	B32 mouseRight  = IsBetween(mouse.x, gridCenterX, gridRight);
	B32 mouseTop    = IsBetween(mouse.y, gridTop, gridCenterY);
	B32 mouseBottom = IsBetween(mouse.y, gridCenterY, gridBottom);

	V4 mouseGridColor = MakeColor(1.0f, 1.0f, 0.0f);
	if (mouseLeft && mouseTop) {
		DrawRectOutline(*canvas, gridLeft, gridCenterX, gridTop, gridCenterY, mouseGridColor);
		labState->mouseIndex = GridTopLeft;
	} else if (mouseRight && mouseTop) {
		DrawRectOutline(*canvas, gridCenterX, gridRight, gridTop, gridCenterY, mouseGridColor);
		labState->mouseIndex = GridTopRight;
	} else if (mouseLeft && mouseBottom) {
		DrawRectOutline(*canvas, gridLeft, gridCenterX, gridCenterY, gridBottom, mouseGridColor);
		labState->mouseIndex = GridBottomLeft;
	} else if (mouseRight && mouseBottom) {
		DrawRectOutline(*canvas, gridCenterX, gridRight, gridCenterY, gridBottom, mouseGridColor);
		labState->mouseIndex = GridBottomRight;
	} else {
		labState->mouseIndex = 0;
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
	while (labState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
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
// TODO: Make water-ground edges look better!
// TODO: Remove inline functions from project!
// TODO: Pass canvas by address in project!