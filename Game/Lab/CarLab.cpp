#include "CarLab.h"

#include "../Bitmap.h"
#include "../Car.h"
#include "../Debug.h"
#include "../Geometry.h"
#include "../Math.h"
#include "../Memory.h"

#define CarLabTmpMemArenaSize (1 * MegaByte)

struct CarLabState {
	bool running;
	Bitmap windowBitmap;
	Bitmap carBitmap;
	MemArena tmpArena;

	float zoomValue;
	float rotationAngle;
	float rotationAngleAdd;
};
CarLabState gCarLabState;

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

		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'A': {
					carLabState->rotationAngleAdd = -0.1f;
					break;
				}
				case 'D': {
					carLabState->rotationAngleAdd = 0.1f;
					break;
				}
			}

			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;
				
			switch (keyCode) {
				case 'G': {
					GenerateCarBitmap(&carLabState->carBitmap, &carLabState->tmpArena);
					break;
				}
				case 'W': {
					carLabState->zoomValue *= 1.1f;
					break;
				}
				case 'S': {
					carLabState->zoomValue *= (1.0f / 1.1f);
					break;
				}
				case 'A': {
					carLabState->rotationAngleAdd = 0.0f;
					break;
				}
				case 'D': {
					carLabState->rotationAngleAdd = 0.0f;
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
	AllocateCarBitmap(carBitmap);
	GenerateCarBitmap(carBitmap, &carLabState->tmpArena);

	carLabState->zoomValue = 1.0f;
}

static void CarLabUpdate(CarLabState* carLabState)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	Bitmap* carBitmap = &carLabState->carBitmap;
	Color backgroundColor = GetColor(0.0f, 0.0f, 0.8f);
	FillBitmapWithColor(windowBitmap, backgroundColor);

	int halfWindowWidth  = windowBitmap->width / 2;
	int halfWindowHeight = windowBitmap->height / 2;
	carLabState->rotationAngle += carLabState->rotationAngleAdd;
	float rotationAngle = carLabState->rotationAngle;
	float width = carBitmap->width * carLabState->zoomValue;
	float height = carBitmap->height * carLabState->zoomValue;
	CopyScaledRotatedBitmap(carBitmap, windowBitmap, halfWindowHeight, halfWindowWidth, width, height, rotationAngle);
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