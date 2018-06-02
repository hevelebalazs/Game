#include "CarLab.h"

#include "../Bitmap.h"
#include "../Debug.h"
#include "../Math.h"
#include "../Memory.h"

#define CarLabTmpMemArenaSize (1 * MegaByte)
#define CarBitmapWidth 100
#define CarBitmapHeight 300

struct CarLabState {
	bool running;
	Bitmap windowBitmap;
	Bitmap carBitmap;
	MemArena tmpArena;
};
CarLabState gCarLabState;

static unsigned int* GetBitmapPixelAddress(Bitmap* bitmap, int row, int col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	unsigned int* pixelAddress = (unsigned int*)bitmap->memory + row * bitmap->width + col;
	return pixelAddress;
}

static void PaintBitmapPixel(Bitmap* bitmap, int row, int col, unsigned int colorCode)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);

	unsigned int* pixelAddress = GetBitmapPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

struct BresenhamData {
	int row1, col1;
	int row2, col2;
	int rowAbs, colAbs;
	int rowAdd, colAdd;
	int error1, error2;
};

static BresenhamData InitBresenham(int row1, int col1, int row2, int col2)
{
	BresenhamData data = {};
	data.col1 = col1;
	data.row1 = row1;
	data.col2 = col2;
	data.row2 = row2;

	data.rowAbs = IntAbs(data.row1 - data.row2);
	data.colAbs = IntAbs(data.col1 - data.col2);

	data.colAdd = 1;
	if (data.col1 > data.col2)
		data.colAdd = -1;

	data.rowAdd = 1;
	if (data.row1 > data.row2)
		data.rowAdd = -1;

	data.error1 = 0;
	if (data.colAbs > data.rowAbs)
		data.error1 = data.colAbs / 2;
	else
		data.error1 = -data.rowAbs / 2;

	data.error2 = 0;
	return data;
}

static void AdvanceBresenham(BresenhamData* data)
{
	data->error2 = data->error1;
	if (data->error2 > -data->colAbs) {
		data->error1 -= data->rowAbs;
		data->col1 += data->colAdd;
	}
	if (data->error2 < data->rowAbs) {
		data->error1 += data->colAbs;
		data->row1 += data->rowAdd;
	}
}

static void DrawBitmapBresenhamLine(Bitmap* bitmap, int row1, int col1, int row2, int col2, Color color)
{
	unsigned int colorCode = ColorCode(color);
	BresenhamData data = InitBresenham(row1, col1, row2, col2);
	while (1) {
		PaintBitmapPixel(bitmap, data.row1, data.col1, colorCode);

		if (data.row1 == data.row2 && data.col1 == data.col2)
			break;

		AdvanceBresenham(&data);
	}
}

static void FloodfillBitmap(Bitmap* bitmap, int row, int col, Color color, MemArena* tmpArena)
{
	unsigned int paintColorCode = ColorCode(color);
	unsigned int baseColorCode  = *GetBitmapPixelAddress(bitmap, row, col);
	Assert(paintColorCode != baseColorCode);
	int positionN = 0;
	int* positions = ArenaPushArray(tmpArena, int, 0);
	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionN++;

	// NOTE: Initial position has to be inserted twice, one for horizontal, one for vertical fill.
	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionN++;

	bool fillHorizontally = true;
	int directionSwitchPosition = 1;

	PaintBitmapPixel(bitmap, row, col, paintColorCode);

	for (int i = 0; i < positionN; ++i) {
		if (i == directionSwitchPosition) {
			fillHorizontally = !fillHorizontally;
			directionSwitchPosition = positionN;
		}
		int row = positions[2 * i];
		int col = positions[2 * i + 1];

		unsigned int* pixelStart = GetBitmapPixelAddress(bitmap, row, col);
		unsigned int* pixel = 0;

		if (fillHorizontally) {
			pixel = pixelStart;
			for (int left = col - 1; left >= 0; --left) {
				pixel--;
				if (*pixel != baseColorCode) 
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, left);
				positionN++;
			}

			pixel = pixelStart;
			for (int right = col + 1; right < bitmap->width; ++right) {
				pixel++;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, right);
				positionN++;
			}
		} else {
			pixel = pixelStart;
			for (int top = row - 1; top >= 0; --top) {
				pixel -= bitmap->width;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, top);
				ArenaPush(tmpArena, int, col);
				positionN++;
			}

			pixel = pixelStart;
			for (int bottom = row + 1; bottom < bitmap->height; ++bottom) {
				pixel += bitmap->width;
				if (*pixel != baseColorCode)
					break;
				*pixel = paintColorCode;
				ArenaPush(tmpArena, int, bottom);
				ArenaPush(tmpArena, int, col);
				positionN++;
			}
		}
	}
	ArenaPopTo(tmpArena, positions);
}

static void FillBitmapWithColor(Bitmap* bitmap, Color color)
{
	unsigned int colorCode = ColorCode(color);
	unsigned int* pixel = (unsigned int*)bitmap->memory;
	for (int row = 0; row < bitmap->height; ++row) {
		for (int col = 0; col < bitmap->width; ++col) {
			*pixel = colorCode;
			pixel++;
		}
	}
}

static void CopyBitmapToPosition(Bitmap* fromBitmap, Bitmap* toBitmap, int toLeft, int toTop)
{
	int rowsToCopy = IntMin2(fromBitmap->height, toBitmap->height - toTop);
	int colsToCopy = IntMin2(fromBitmap->width,  toBitmap->width  - toLeft);
	for (int fromRow = 0; fromRow < rowsToCopy; ++fromRow) {
		int toRow = toTop + fromRow;
		unsigned int* fromPixel = GetBitmapPixelAddress(fromBitmap, fromRow, 0);
		unsigned int* toPixel = GetBitmapPixelAddress(toBitmap, toRow, toLeft);
		for (int fromCol = 0; fromCol < colsToCopy; ++fromCol) {
			*toPixel = *fromPixel;
			fromPixel++;
			toPixel++;
		}
	}
}

static void CopyBitmapToCenter(Bitmap* fromBitmap, Bitmap* toBitmap)
{
	int toLeft = (toBitmap->width  / 2) - (fromBitmap->width  / 2);
	int toTop  = (toBitmap->height / 2) - (fromBitmap->height / 2);
	CopyBitmapToPosition(fromBitmap, toBitmap, toLeft, toTop);
}

static void CarLabBlit(CarLabState* carLabState, HDC context, RECT rect)
{
	Bitmap* bitmap = &carLabState->windowBitmap;
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	StretchDIBits(context,
				  0, 0, bitmap->width, bitmap->height,
				  0, 0, width, height,
				  bitmap->memory,
				  &bitmap->info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void CarLabResize(CarLabState* carLabState, int width, int height)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	ResizeBitmap(windowBitmap, width, height);
}

static void DrawBitmapPolyOutline(Bitmap* bitmap, int polyN, int* polyColRow, Color color)
{
	for (int i = 0; i < polyN; ++i) {
		int next = 0;
		if (i < polyN - 1)
			next = i + 1;
		else
			next = 0;
		int row1 = polyColRow[2 * i];
		int col1 = polyColRow[2 * i + 1];
		int row2 = polyColRow[2 * next];
		int col2 = polyColRow[2 * next + 1];
		DrawBitmapBresenhamLine(bitmap, row1, col1, row2, col2, color);
	}
}

static Color GetRandomCarColor()
{
	Color color = {};
	float colorSum = RandomBetween(1.0f, 1.5f);
	color.red = RandomBetween(0.0f, colorSum);
	color.red = Clip(color.red, 0.0f, 1.0f);
	color.green = RandomBetween(0.0f, colorSum - color.red);
	color.green = Clip(color.green, 0.0f, 1.0f);
	color.blue = RandomBetween(0.0f, colorSum - color.red - color.green);
	color.blue = Clip(color.blue, 0.0f, 1.0f);
	return color;
}

static Color GetRandomWindowColor()
{
	Color color = {};
	color.red   = RandomBetween(0.0f, 0.05f);
	color.green = RandomBetween(0.0f, 0.05f);
	color.blue  = RandomBetween(0.0f, 0.25f);
	return color;
}

static Color GetShadowColor(Color color)
{
	Color shadowColor = {};
	// float brightnessDifference = 0.25f;
	float shadowRatio = 0.5f;
	shadowColor.red   = shadowRatio * color.red;
	shadowColor.green = shadowRatio * color.green;
	shadowColor.blue  = shadowRatio * color.blue;
	/*
	shadowColor.red   = Max2(0.0f, color.red   - brightnessDifference);
	shadowColor.green = Max2(0.0f, color.green - brightnessDifference);
	shadowColor.blue  = Max2(0.0f, color.blue  - brightnessDifference);
	*/
	return shadowColor;
}

static void GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena)
{
	Color backgroundColor = {0.2f, 0.2f, 0.2f};
	FillBitmapWithColor(carBitmap, backgroundColor);

	Color carColor = GetRandomCarColor();
	Color shadowColor = GetShadowColor(carColor);

	Color borderColor = Color{0.2f, 0.2f, 0.2f};

	int bitmapLeft   = 0;
	int bitmapRight  = carBitmap->width - 1;
	int bitmapTop    = 0;
	int bitmapBottom = carBitmap->height - 1;

	int frontHoodLength    = IntRandom(65, 70);
	int frontHoodWidthDiff = IntRandom(3, 7);

	int frontHoodTopLeft     = bitmapLeft + frontHoodWidthDiff;
	int frontHoodBottomLeft  = bitmapLeft;
	int frontHoodTopRight    = bitmapRight - frontHoodWidthDiff;
	int frontHoodBottomRight = bitmapRight;
	int frontHoodTop         = bitmapTop + 20;
	int frontHoodBottom      = frontHoodTop + frontHoodLength;

	int middlePartLength = IntRandom(135, 150);
	
	int backHoodLength    = IntRandom(20, 30);
	int backHoodWidthDiff = IntRandom(0, 5);

	int backHoodTopLeft     = bitmapLeft;
	int backHoodBottomLeft  = bitmapLeft + backHoodWidthDiff;
	int backHoodTopRight    = bitmapRight;
	int backHoodBottomRight = bitmapRight - backHoodWidthDiff;
	int backHoodTop         = frontHoodBottom + middlePartLength;
	int backHoodBottom      = backHoodTop + backHoodLength;

	int carPoly[] = {
		frontHoodTop, frontHoodTopLeft,
		frontHoodTop, frontHoodTopRight,
		frontHoodBottom, frontHoodBottomRight,
		backHoodTop, backHoodTopRight,
		backHoodBottom, backHoodBottomRight,
		backHoodBottom, backHoodBottomLeft,
		backHoodTop, backHoodTopLeft,
		frontHoodBottom, frontHoodBottomLeft
	};

	DrawBitmapPolyOutline(carBitmap, 8, carPoly, carColor);
	int carCenterRow = (frontHoodTop + backHoodBottom) / 2;
	int carCenterCol = (frontHoodBottomLeft + frontHoodBottomRight) / 2;
	FloodfillBitmap(carBitmap, carCenterRow, carCenterCol, carColor, tmpArena);

	int frontWindowLength    = IntRandom(40, 60);
	int windowSeparatorWidth = IntRandom(3, 5);
	int frontWindowWidthDiff = IntRandom(5, 15);

	Color windowColor = GetRandomWindowColor();
	int frontWindowTop         = frontHoodBottom;
	int frontWindowBottom      = frontWindowTop + frontWindowLength;
	int frontWindowTopLeft     = frontHoodBottomLeft + windowSeparatorWidth;
	int frontWindowTopRight    = frontHoodBottomRight - windowSeparatorWidth;
	int frontWindowBottomLeft  = frontWindowTopLeft + frontWindowWidthDiff;
	int frontWindowBottomRight = frontWindowTopRight - frontWindowWidthDiff;
	int frontWindowPoly[] = {
		frontWindowTop,    frontWindowTopLeft,
		frontWindowTop,    frontWindowTopRight,
		frontWindowBottom, frontWindowBottomRight,
		frontWindowBottom, frontWindowBottomLeft
	};

	DrawBitmapPolyOutline(carBitmap, 4, frontWindowPoly, borderColor);
	int frontWindowCenterRow = (frontWindowTop + frontWindowBottom) / 2;
	int frontWindowCenterCol = (frontWindowTopLeft + frontWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontWindowCenterRow, frontWindowCenterCol, windowColor, tmpArena);

	int backWindowLength = IntRandom(15, 25);
	int backWindowWidthDiff = IntRandom(5, 15);

	int backWindowBottom      = backHoodTop;
	int backWindowTop         = backWindowBottom - backWindowLength;
	int backWindowBottomLeft  = backHoodTopLeft + windowSeparatorWidth;
	int backWindowBottomRight = backHoodTopRight - windowSeparatorWidth;
	int backWindowTopLeft     = frontWindowBottomLeft;
	int backWindowTopRight    = frontWindowBottomRight;
	int backWindowPoly[] = {
		backWindowTop,    backWindowTopLeft,
		backWindowTop,    backWindowTopRight,
		backWindowBottom, backWindowBottomRight,
		backWindowBottom, backWindowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backWindowPoly, borderColor);
	int backWindowCenterRow = (backWindowTop + backWindowBottom) / 2;
	int backWindowCenterCol = (backWindowTopLeft + backWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, backWindowCenterRow, backWindowCenterCol, windowColor, tmpArena);

	int topLeftWindowLeft     = frontWindowTopLeft - windowSeparatorWidth;
	int topLeftWindowRight    = frontWindowBottomLeft - windowSeparatorWidth;
	int topLeftWindowLeftTop  = frontWindowTop + windowSeparatorWidth;
	int topLeftWindowRightTop = frontWindowBottom + windowSeparatorWidth;
	int topLeftWindowBottom   = topLeftWindowRightTop + 25;
	int topLeftWindowPoly[] = {
		topLeftWindowLeftTop,  topLeftWindowLeft,
		topLeftWindowRightTop, topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topLeftWindowPoly, borderColor);
	int topLeftWindowCenterRow = (topLeftWindowLeftTop + topLeftWindowBottom) / 2;
	int topLeftWindowCenterCol = (topLeftWindowLeft + topLeftWindowRight) / 2;
	FloodfillBitmap(carBitmap, topLeftWindowCenterRow, topLeftWindowCenterCol, windowColor, tmpArena);

	int bottomLeftWindowLeft        = topLeftWindowLeft;
	int bottomLeftWindowRight       = topLeftWindowRight;
	int bottomLeftWindowTop         = topLeftWindowBottom + windowSeparatorWidth;
	int bottomLeftWindowLeftBottom  = backWindowBottom - windowSeparatorWidth;
	int bottomLeftWindowRightBottom = backWindowTop - windowSeparatorWidth;
	int bottomLeftWindowPoly[] = {
		bottomLeftWindowTop,         bottomLeftWindowLeft,
		bottomLeftWindowTop,         bottomLeftWindowRight,
		bottomLeftWindowRightBottom, bottomLeftWindowRight,
		bottomLeftWindowLeftBottom,  bottomLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomLeftWindowPoly, borderColor);
	int bottomLeftWindowCenterRow = (bottomLeftWindowTop + bottomLeftWindowLeftBottom) / 2;
	int bottomLeftWindowCenterCol = (bottomLeftWindowLeft + bottomLeftWindowRight) / 2;
	FloodfillBitmap(carBitmap, bottomLeftWindowCenterRow, bottomLeftWindowCenterCol, windowColor, tmpArena);

	int topRightWindowLeft = frontWindowBottomRight + windowSeparatorWidth;
	int topRightWindowRight = frontWindowTopRight + windowSeparatorWidth;
	int topRightWindowLeftTop = frontWindowBottom + windowSeparatorWidth;
	int topRightWindowRightTop = frontWindowTop + windowSeparatorWidth;
	int topRightWindowBottom = topRightWindowLeftTop + 25;
	int topRightWindowPoly[] = {
		topRightWindowLeftTop,  topRightWindowLeft,
		topRightWindowRightTop, topRightWindowRight,
		topRightWindowBottom,   topRightWindowRight,
		topRightWindowBottom,   topRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topRightWindowPoly, borderColor);
	int topRightWindowCenterRow = (topRightWindowRightTop + topRightWindowBottom) / 2;
	int topRightWindowCenterCol = (topRightWindowLeft + topRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, topRightWindowCenterRow, topRightWindowCenterCol, windowColor, tmpArena);

	int bottomRightWindowLeft        = topRightWindowLeft;
	int bottomRightWindowRight       = topRightWindowRight;
	int bottomRightWindowTop         = topRightWindowBottom + windowSeparatorWidth;
	int bottomRightWindowLeftBottom  = backWindowTop - windowSeparatorWidth;
	int bottomRightWindowRightBottom = backWindowBottom - windowSeparatorWidth;
	int bottomRightWindowPoly[] = {
		bottomRightWindowTop,         bottomRightWindowLeft,
		bottomRightWindowTop,         bottomRightWindowRight,
		bottomRightWindowRightBottom, bottomRightWindowRight,
		bottomRightWindowLeftBottom,  bottomRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomRightWindowPoly, borderColor);
	int bottomRightWindowCenterRow = (bottomRightWindowTop + bottomRightWindowRightBottom) / 2;
	int bottomRightWindowCenterCol = (bottomRightWindowLeft + bottomRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, bottomRightWindowCenterRow, bottomRightWindowCenterCol, windowColor, tmpArena);

	int leftShadowPoly[] = {
		frontHoodTop,      frontHoodTopLeft,
		frontHoodTop,      frontHoodTopLeft + 5,
		frontWindowTop,    frontWindowTopLeft,
		frontWindowBottom, frontWindowBottomLeft,
		backWindowTop,     backWindowTopLeft,
		backWindowBottom,  backWindowBottomLeft,
		backHoodBottom,    backHoodBottomLeft + 5,
		backHoodBottom,    backHoodBottomLeft,
		backHoodTop,       backHoodTopLeft,
		frontHoodBottom,   frontHoodBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 10, leftShadowPoly, shadowColor);
	FloodfillBitmap(carBitmap, frontHoodTop + 1, frontHoodTopLeft + 1, shadowColor, tmpArena);

	int rightShadowPoly[] = {
		frontHoodTop, frontHoodTopRight - 5,
		frontHoodTop, frontHoodTopRight,
		frontHoodBottom, frontHoodBottomRight,
		backHoodTop, backHoodTopRight,
		backHoodBottom, backHoodBottomRight,
		backHoodBottom, backHoodBottomRight - 5,
		backWindowBottom, backWindowBottomRight,
		backWindowTop, backWindowTopRight,
		frontWindowBottom, frontWindowBottomRight,
		frontWindowTop, frontWindowTopRight
	};
	DrawBitmapPolyOutline(carBitmap, 10, rightShadowPoly, shadowColor);
	FloodfillBitmap(carBitmap, frontHoodTop + 1, frontHoodTopRight - 1, shadowColor, tmpArena);

	int frontShadowTopLeft     = frontHoodTopLeft + 10;
	int frontShadowBottomLeft  = frontHoodTopLeft;
	int frontShadowTopRight    = frontHoodTopRight - 10;
	int frontShadowBottomRight = frontHoodTopRight;
	int frontShadowTop         = frontHoodTop - 3;
	int frontShadowBottom      = frontHoodTop;
	int frontShadowPoly[] = {
		frontShadowTop,    frontShadowTopLeft,
		frontShadowTop,    frontShadowTopRight,
		frontShadowBottom, frontShadowBottomRight,
		frontShadowBottom, frontShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontShadowPoly, shadowColor);
	int frontShadowCenterRow = (frontShadowTop + frontShadowBottom) / 2;
	int frontShadowCenterCol = (frontShadowTopLeft + frontShadowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontShadowCenterRow, frontShadowCenterCol, shadowColor, tmpArena);

	int backShadowTopLeft     = backHoodBottomLeft;
	int backShadowBottomLeft  = backHoodBottomLeft + 10;
	int backShadowTopRight    = backHoodBottomRight;
	int backShadowBottomRight = backHoodBottomRight - 10;
	int backShadowTop         = backHoodBottom;
	int backShadowBottom      = backHoodBottom + 3;
	int backShadowPoly[] = {
		backShadowTop,    backShadowTopLeft,
		backShadowTop,    backShadowTopRight,
		backShadowBottom, backShadowBottomRight,
		backShadowBottom, backShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backShadowPoly, shadowColor);
	int backShadowCenterRow = (backShadowTop + backShadowBottom) / 2;
	int backShadowCenterCol = (backShadowTopLeft + backShadowTopRight) / 2;
	FloodfillBitmap(carBitmap, backShadowCenterRow, backShadowCenterCol, shadowColor, tmpArena);
}

static LRESULT CALLBACK CarLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	CarLabState* carLabState = &gCarLabState;
	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			CarLabResize(carLabState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			CarLabBlit(carLabState, context, clientRect);
	
			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;
				
			switch (keyCode) {
				case 'G': {
					GenerateCarBitmap(&carLabState->carBitmap, &carLabState->tmpArena);
					break;
				}
			}
			break;
		}

		case WM_DESTROY: {
			carLabState->running = false;
			break;
		}

		case WM_CLOSE: {
			carLabState->running = false;
			break;
		}
	
		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}
	return result;
}

static void CarLabInit(CarLabState* carLabState, int windowWidth, int windowHeight)
{
	InitRandom();
	carLabState->running = true;
	CarLabResize(carLabState, windowWidth, windowHeight);

	carLabState->tmpArena = CreateMemArena(CarLabTmpMemArenaSize);

	Bitmap* carBitmap = &carLabState->carBitmap;
	ResizeBitmap(carBitmap, CarBitmapWidth, CarBitmapHeight);
	GenerateCarBitmap(carBitmap, &carLabState->tmpArena);
}

static void CarLabUpdate(CarLabState* carLabState)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	Color backgroundColor = {0.2f, 0.2f, 0.2f};
	FillBitmapWithColor(windowBitmap, backgroundColor);

	Bitmap* carBitmap = &carLabState->carBitmap;
	CopyBitmapToCenter(carBitmap, windowBitmap);
}

void CarLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = CarLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "CarLabWindowClass";

	Assert(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"CarLab",
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

	CarLabState* carLabState = &gCarLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	CarLabInit(carLabState, width, height);
	
	MSG message = {};
	while (carLabState->running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		CarLabUpdate(carLabState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CarLabBlit(carLabState, context, rect);
		ReleaseDC(window, context);
	}
}

// TODO: Add mirrors!
// TODO: Add lamps!
// TODO: Alpha channel!
// TODO: Draw scaled bitmap!
// TODO: Draw rotated bitmap!
// TODO: Make sure car always fits in the bitmap!