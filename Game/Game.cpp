#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include "GridMapCreator.h"
#include "Bitmap.h"
#include "Path.h"
#include "Vehicle.h"

static bool running;

Bitmap globalBitmap;
Map globalMap;
Intersection *globalSelectedIntersection;
IntersectionPathHelper globalPathHelper;

static float globalTargetFPS = 60.0f;
static float globalTargetFrameS = 1.0f / globalTargetFPS;
static float globalTargetFrameMS = globalTargetFrameS * 1000.0f;

Vehicle globalVehicle;

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
	Intersection *highlightIntersection = globalMap.getIntersectionAtPoint(mousePoint, 20.0f);

	globalMap.draw(bitmap);

	if (highlightIntersection) {
		Color color = { 1.0f, 0.5f, 0.0f };
		highlightIntersection->highlight(bitmap, color);
	}

	if (globalSelectedIntersection) {
		Color color = { 1.0f, 0.0f, 0.0f };
		globalSelectedIntersection->highlight(bitmap, color);
	}

	globalVehicle.draw(bitmap);
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
	globalMap = createGridMap((float)width, (float)height, 150);

	globalPathHelper.indexes = new int[globalMap.intersectionCount];
	globalPathHelper.isHelper = new int[globalMap.intersectionCount];
	globalPathHelper.source = new int[globalMap.intersectionCount];

	globalVehicle.angle = 0.0f;
	globalVehicle.width = 15.0f;
	globalVehicle.length = 25.0f;
	globalVehicle.color = { 0.0f, 0.0f, 1.0f };
	globalVehicle.maxSpeed = 100.0f;

	globalVehicle.map = &globalMap;
	globalVehicle.pathHelper = &globalPathHelper;

	for (int i = 0; i < globalMap.intersectionCount; ++i) {
		Intersection *intersection = &globalMap.intersections[i];

		if (intersection->leftRoad || intersection->rightRoad ||
				intersection->topRoad || intersection->bottomRoad) {
			globalVehicle.position = intersection->coordinate;
			globalVehicle.onIntersection = intersection;
			globalVehicle.targetIntersection = intersection;
			break;
		}
	}

	timeBeginPeriod(1);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	running = true;
	while (running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) running = false;
			else if (message.message == WM_LBUTTONDOWN) {
				Point mousePoint = WinMousePosition(window);

				globalSelectedIntersection = globalMap.getIntersectionAtPoint(mousePoint, 20.0f);
				globalVehicle.targetIntersection = globalSelectedIntersection;
			}

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		globalVehicle.update(globalTargetFrameS);
		WinDraw(window, globalBitmap);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalBitmap, context, rect);
		
		ReleaseDC(window, context);

		LARGE_INTEGER nowCounter;
		QueryPerformanceCounter(&nowCounter);

		long long elapsedUS = nowCounter.QuadPart - lastCounter.QuadPart;
		float elapsedMS = ((float)elapsedUS * 1000.0f) / (float)counterFrequency.QuadPart;

		if (elapsedMS < globalTargetFrameMS) {
			DWORD sleepMS = (DWORD)(globalTargetFrameMS - elapsedMS);
			Sleep(sleepMS);
		}

		QueryPerformanceCounter(&lastCounter);
	}

	return 0;
}