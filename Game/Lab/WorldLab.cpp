#include "WorldLab.hpp"

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"

#define WorldLabGridN 128

struct WorldLabState {
	B32 running;
	Canvas canvas;
	Camera camera;
	V2 directions[WorldLabGridN + 1][WorldLabGridN + 1];

	Bitmap bitmap;
};
WorldLabState gWorldLabState;

static void WorldLabResize(WorldLabState* labState, I32 width, I32 height)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	ResizeBitmap(bitmap, width, height);
}

static F32 SmoothRatio(F32 x)
{
	return x * x * (3.0f - 2.0f * x);
}

static void WorldLabInit(WorldLabState* labState, I32 width, I32 height)
{
	InitRandom();

	labState->running = true;
	WorldLabResize(labState, width, height);

	Bitmap* bitmap = &labState->bitmap;
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

	I32 gridN = 4;
	F32 valueRatio = 1.0f;
	while (gridN <= WorldLabGridN) {
		for (I32 row = 0; row <= gridN; ++row) {
			for (I32 col = 0; col <= gridN; ++col) {
				F32 angle = RandomBetween(0.0f, 2.0f * PI);
				labState->directions[row][col] = RotationVector(angle);
			
				labState->directions[row][col].x = RandomBetween(-1.0f, 1.0f);
				labState->directions[row][col].y = RandomBetween(-1.0f, 1.0f);
			}
		}

		V4 color1 = MakeColor(0.0f, 0.0f, 0.0f);
		V4 color2 = MakeColor(1.0f, 1.0f, 1.0f);

		F32 ratio = F32(gridN) / F32(bitmap->height);

		I32 valueIndex = 0;
		for (I32 row = 0; row < bitmap->height; ++row) {
			for (I32 col = 0; col < bitmap->width; ++col) {
				F32 x = F32(col) * ratio;
				F32 y = F32(row) * ratio;
	
				I32 left = Floor(x);
				I32 right = left + 1;
				Assert(IsIntBetween(left, 0, gridN));
				Assert(IsIntBetween(right, 0, gridN));

				I32 top = Floor(y);
				I32 bottom = top + 1;
				Assert(IsIntBetween(top, 0, gridN));
				Assert(IsIntBetween(bottom, 0, gridN));

				F32 ratioX = SmoothRatio(x - left);
				F32 ratioY = SmoothRatio(y - top);
				Assert(IsBetween(ratioX, 0.0f, 1.0f));
				Assert(IsBetween(ratioY, 0.0f, 1.0f));

				V2 positionInCell = MakePoint(ratioX, ratioY);

				V2 topLeftPoint = MakePoint(0.0f, 0.0f);
				V2 topRightPoint = MakePoint(1.0f, 0.0f);
				V2 bottomLeftPoint = MakePoint(0.0f, 1.0f);
				V2 bottomRightPoint = MakePoint(1.0f, 1.0f);

				F32 topLeft = DotProduct(labState->directions[top][left], positionInCell - topLeftPoint);
				F32 topRight = DotProduct(labState->directions[top][right], positionInCell - topRightPoint);
				F32 bottomLeft = DotProduct(labState->directions[bottom][left], positionInCell - bottomLeftPoint);
				F32 bottomRight = DotProduct(labState->directions[bottom][right], positionInCell - bottomRightPoint);

				F32 topValue = Lerp(topLeft, ratioX, topRight);
				F32 bottomValue = Lerp(bottomLeft, ratioX, bottomRight);

				F32 value = Lerp(topValue, ratioY, bottomValue);

				values[valueIndex] += value * valueRatio;
				valueIndex++;
			}
		}

		gridN *= 2;
		valueRatio *= 0.67f;
	}

	F32 minValue = values[0];
	F32 maxValue = values[0];
	valueIndex = 0;
	for (I32 row = 0; row < bitmap->height; ++row) {
		for (I32 col = 0; col < bitmap->width; ++col) {
			F32 value = values[valueIndex];
			valueIndex++;

			minValue = Min2(minValue, value);
			maxValue = Max2(maxValue, value);
		}
	}

	Assert(minValue < maxValue);
	valueIndex = 0;

	U32* pixel = bitmap->memory;

	V4 darkBlue = MakeColor(0.0f, 0.0f, 0.3f);
	V4 lightBlue = MakeColor(0.0f, 0.0f, 0.8f);
	V4 darkGreen = MakeColor(0.0f, 0.3f, 0.0f);
	V4 lightGreen = MakeColor(0.0f, 0.8f, 0.0f);
	V4 lightBrown = MakeColor(0.3f, 0.4f, 0.0f);
	V4 darkBrown = MakeColor(0.2f, 0.3f, 0.0f);
	V4 lightWhite = MakeColor(0.8f, 0.8f, 0.8f);
	V4 darkWhite = MakeColor(0.3f, 0.3f, 0.3f);

	for (int row = 0; row < bitmap->height; ++row) {
		for (int col = 0; col < bitmap->width; ++col) {
			F32 oldValue = values[valueIndex];
			valueIndex++;

			Assert(IsBetween(oldValue, minValue, maxValue));
			F32 newValue = (oldValue - minValue) / (maxValue - minValue);
			newValue = SmoothRatio(newValue);
			Assert(IsBetween(newValue, 0.0f, 1.0f));

			V4 color = {};

			F32 seaLevel = 0.5f;
			F32 hillLevel = 0.85f;
			F32 mountainLevel = 0.9f;

			if (IsBetween(newValue, 0.0f, seaLevel))
				color = InterpolateColors(darkBlue, (newValue - 0.0f) / (seaLevel - 0.0f), lightBlue);
			else if (IsBetween(newValue, seaLevel, hillLevel))
				color = InterpolateColors(lightGreen, (newValue - seaLevel) / (hillLevel - seaLevel), darkGreen);
			else if (IsBetween(newValue, hillLevel, mountainLevel))
				color = InterpolateColors(darkBrown, (newValue - hillLevel) / (mountainLevel - hillLevel), lightBrown);
			else if (IsBetween(newValue, mountainLevel, 1.0f))
				color = InterpolateColors(darkWhite, (newValue - mountainLevel) / (1.0f - mountainLevel), lightWhite);
			else
				DebugBreak();

			*pixel = GetColorCode(color);
			pixel++;
		}
	}

	delete[] values;
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

static void WorldLabUpdate(WorldLabState* labState)
{
	Bitmap* bitmap = &labState->canvas.bitmap;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	F32 width = F32(bitmap->width);
	F32 height = F32(bitmap->height);

	F32 gridTotalSize = Min2(width, height) * 0.8f;
	F32 gridSize = gridTotalSize / F32(WorldLabGridN);
	F32 gridLeft = (width - gridTotalSize) * 0.5f;
	F32 gridTop = (height - gridTotalSize) * 0.5f;
	F32 gridRight = gridLeft + gridTotalSize;
	F32 gridBottom = gridTop + gridTotalSize;

	Canvas* canvas = &labState->canvas;
	Camera* camera = &labState->camera;
	camera->center.x = width * 0.5f;
	camera->center.y = height * 0.5f;
	camera->screenPixelSize.x = width;
	camera->screenPixelSize.y = height;
	camera->unitInPixels = 1.0f;
	camera->targetUnitInPixels = 1.0f;
	canvas->camera = camera;

	DrawStretchedBitmap(*canvas, &labState->bitmap, gridLeft, gridRight, gridTop, gridBottom);

	float gridY = gridTop;
	float gridX = gridLeft;

	/*
	V4 gridColor = MakeColor(0.5f, 0.5f, 0.0f);
	for (int row = 0; row <= WorldLabGridN; ++row) {
		V2 point1 = MakePoint(gridLeft, gridY);
		V2 point2 = MakePoint(gridRight, gridY);
		Bresenham(*canvas, point1, point2, gridColor);
		gridY += gridSize;
	}

	for (int col = 0; col <= WorldLabGridN; ++col) {
		V2 point1 = MakePoint(gridX, gridTop);
		V2 point2 = MakePoint(gridX, gridBottom);
		Bresenham(*canvas, point1, point2, gridColor);
		gridX += gridSize;
	}
	*/

	/*
	V4 vectorColor = MakeColor(0.5f, 0.0f, 0.0f);

	gridY = gridTop;
	gridX = gridLeft;
	for (int row = 0; row <= WorldLabGridN; ++row) {
		for (int col = 0; col <= WorldLabGridN; ++col) {
			V2 direction = labState->directions[row][col];
			V2 start = MakePoint(gridX, gridY);
			V2 end = start + (gridSize * direction);
			Bresenham(*canvas, start, end, vectorColor);

			gridX += gridSize;
		}
		gridY += gridSize;
		gridX = gridLeft;
	}
	*/
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

		WorldLabUpdate(labState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		WorldLabBlit(labState, context, rect);
		ReleaseDC(window, context);
	}
}