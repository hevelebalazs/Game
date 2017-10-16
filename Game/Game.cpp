#include <Windows.h>
#include <math.h>
#include "GridMapCreator.h"
#include "Bitmap.h"

static bool running;

Bitmap globalBitmap;
Map map;

void WinResize(Bitmap *bitmap, int width, int height) {
	if (bitmap->memory) delete bitmap->memory;

	bitmap->width = width;
	bitmap->height = height;

	bitmap->info.bmiHeader.biSize = sizeof(bitmap->info.bmiHeader);
	bitmap->info.bmiHeader.biWidth = bitmap->width;
	bitmap->info.bmiHeader.biHeight = -bitmap->height;
	bitmap->info.bmiHeader.biPlanes = 1;
	bitmap->info.bmiHeader.biBitCount = 32;
	bitmap->info.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmap->width * bitmap->height) * bytesPerPixel;

	bitmap->memory = (void *)(new char[bitmapMemorySize]);

	bitmap->clear({0.0f, 0.0f, 0.0f});
}

static Point WinMousePosition(HWND window) {
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	return point;
}

void WinDraw(HWND window, Bitmap bitmap) {
	Point mousePoint = WinMousePosition(window);
	Intersection *highlightIntersection = map.getIntersectionAtPoint(mousePoint, 20.0f);

	map.draw(bitmap);

	if (highlightIntersection) highlightIntersection->highlight(bitmap);
}

void WinUpdate(Bitmap bitmap, HDC context, RECT clientRect) {
	int windowWidth = clientRect.right - clientRect.left;
	int windowHeight = clientRect.bottom - clientRect.top;

	StretchDIBits(context,
		0, 0, bitmap.width, bitmap.height,
		0, 0, windowWidth, windowHeight,
		bitmap.memory,
		&bitmap.info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

LRESULT CALLBACK WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) 
{
	LRESULT result = 0;

	switch (message) {
	case WM_SIZE: {
		RECT clientRect;
		GetClientRect(window, &clientRect);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;
		WinResize(&globalBitmap, width, height);
	}

	case WM_PAINT: {
		PAINTSTRUCT paint;
		HDC context = BeginPaint(window, &paint);
		
		RECT clientRect;
		GetClientRect(window, &clientRect);

		WinUpdate(globalBitmap, context, clientRect);

		EndPaint(window, &paint);
	} break;

	case WM_DESTROY: {
		running = false;
	} break;

	case WM_CLOSE: {
		running = false;
	} break;

	default: {
		result = DefWindowProc(window, message, wparam, lparam);
	} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	WNDCLASS windowClass = {};

	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = WinCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "GameWindowClass";

	if (!RegisterClass(&windowClass)) {
		OutputDebugStringA("Failed to register window class!\n");
		return 1;
	}

	HWND window = CreateWindowEx(
		0,
		windowClass.lpszClassName,
		"Game",
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

	if (!window) {
		OutputDebugStringA("Failed to create window!\n");
		return 1;
	}

	HDC context = GetDC(window);
	MSG message;
	
	RECT rect;
	GetClientRect(window, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	map = createGridMap((float)width, (float)height, 100);

	int xOffset = 0;
	int yOffset = 0;

	running = true;
	while (running) {
		xOffset ++;

		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) running = false;

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		WinDraw(window, globalBitmap);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalBitmap, context, rect);
		
		ReleaseDC(window, context);
	}

	return 0;
}