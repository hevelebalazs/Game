#include "CarLab.h"

#include "../Bitmap.h"
#include "../Debug.h"

bool gRunning;
Bitmap gBitmap;

static void CarLabBlit(Bitmap* bitmap, HDC context, RECT rect)
{
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

static void CarLabResize(int width, int height)
{
	ResizeBitmap(&gBitmap, width, height);
}

static LRESULT CALLBACK CarLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			CarLabResize(width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			CarLabBlit(&gBitmap, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_DESTROY: {
			gRunning = false;
			break;
		}

		case WM_CLOSE: {
			gRunning = false;
			break;
		}

		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

static void CarLabInit(int windowWidth, int windowHeight)
{
	gRunning = true;
	CarLabResize(windowWidth, windowHeight);
}

static void CarLabUpdate(Bitmap* bitmap)
{
	Color backgroundColor = {0.0f, 0.0f, 0.0f};
	unsigned int backgroundColorCode = ColorCode(backgroundColor);

	unsigned int* pixel = (unsigned int*)bitmap->memory;
	for (int row = 0; row < bitmap->height; ++row) {
		for (int col = 0; col < bitmap->width; ++col) {
			*pixel = backgroundColorCode;
			pixel++;
		}
	}
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

	RECT rect = {};
	GetClientRect(window, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	CarLabInit(width, height);
	
	MSG message = {};
	while (gRunning) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		CarLabUpdate(&gBitmap);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CarLabBlit(&gBitmap, context, rect);
		ReleaseDC(window, context);
	}
}