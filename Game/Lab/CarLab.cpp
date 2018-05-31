#include "CarLab.h"

#include "../Bitmap.h"
#include "../Debug.h"
#include "../Math.h"

static int CarBitmapWidth  = 100;
static int CarBitmapHeight = 200;

struct CarLabState {
	bool running;
	Bitmap windowBitmap;
	Bitmap carBitmap;
};
CarLabState gCarLabState;

static unsigned int* GetBitmapPixelAddress(Bitmap* bitmap, int row, int col)
{
	Assert(row >= 0 && row < bitmap->height);
	Assert(col >= 0 && col < bitmap->width);
	unsigned int* pixelAddress = (unsigned int*)bitmap->memory + row * bitmap->width + col;
	return pixelAddress;
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
	carLabState->running = true;
	CarLabResize(carLabState, windowWidth, windowHeight);

	Bitmap* carBitmap = &carLabState->carBitmap;
	ResizeBitmap(carBitmap, CarBitmapWidth, CarBitmapHeight);
	Color carColor = {0.0f, 0.0f, 1.0f};
	FillBitmapWithColor(carBitmap, carColor);
}

static void CarLabUpdate(CarLabState* carLabState)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	Color backgroundColor = {0.0f, 0.0f, 0.0f};
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