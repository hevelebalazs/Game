#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include "GridMapCreator.h"
#include "Bitmap.h"
#include "Renderer.h"
#include "Path.h"
#include "Vehicle.h"
#include "AutoVehicle.h"
#include "PlayerVehicle.h"

static bool running;

Renderer globalRenderer;
Map globalMap;
Intersection* globalSelectedIntersection;
PlayerVehicle globalPlayerVehicle;

Building* globalSelectedBuilding;
Building* globalHighlightedBuilding;
Path globalBuildingPath;

static float globalTargetFPS = 60.0f;
static float globalTargetFrameS = 1.0f / globalTargetFPS;
static float globalTargetFrameMS = globalTargetFrameS * 1000.0f;

void WinResize(Renderer* renderer, int width, int height) {
	Bitmap* bitmap = &renderer->bitmap;
	Camera* camera = &renderer->camera;

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

	camera->pixelCoordRatio = 1.0f;
	camera->screenSize = Point{ (float)width, (float)height };
	camera->center = camera->screenSize * 0.5f;

	renderer->Clear({0.0f, 0.0f, 0.0f});
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

void WinDraw(HWND window, Renderer renderer) {
	Point mousePoint = WinMousePosition(window);
	Intersection *highlightIntersection = globalMap.GetIntersectionAtPoint(mousePoint, 20.0f);

	globalMap.Draw(renderer);
	globalPlayerVehicle.vehicle.Draw(renderer);

	if (globalSelectedBuilding) {
		Color highlightColor = {0.0f, 1.0f, 1.0f};
		globalSelectedBuilding->HighLight(globalRenderer, highlightColor);
	}

	if (globalHighlightedBuilding) {
		Color highlightColor = {0.0f, 1.0f, 1.0f};
		globalHighlightedBuilding->HighLight(globalRenderer, highlightColor);
	}

	if (globalBuildingPath.nodeCount > 0) {
		Color color = {0.0f, 1.0f, 1.0f};
		DrawPath(&globalBuildingPath, globalRenderer, color, 5.0f);
	}
}

void WinUpdate(Renderer renderer, HDC context, RECT clientRect) {
	int windowWidth = clientRect.right - clientRect.left;
	int windowHeight = clientRect.bottom - clientRect.top;

	Bitmap bitmap = renderer.bitmap;

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
		WinResize(&globalRenderer, width, height);
	}

	case WM_PAINT: {
		PAINTSTRUCT paint;
		HDC context = BeginPaint(window, &paint);
		
		RECT clientRect;
		GetClientRect(window, &clientRect);

		WinUpdate(globalRenderer, context, clientRect);

		EndPaint(window, &paint);
	} break;

	case WM_KEYUP: {
		WPARAM keyCode = wparam;

		switch (keyCode) {

		case 'W': {
			globalPlayerVehicle.engineForce = 0.0f;
		} break;
		case 'S': {
			globalPlayerVehicle.engineForce = 0.0f;
		} break;
		case 'A': {
			globalPlayerVehicle.turnAngle = 0.0f;
		} break;

		case 'D': {
			globalPlayerVehicle.turnAngle = 0.0f;
		}

		}
	} break;

	case WM_KEYDOWN: {
		WPARAM keyCode = wparam;

		switch (keyCode) {

		case 'W': {
			globalPlayerVehicle.engineForce = globalPlayerVehicle.maxEngineForce;
		} break;

		case 'S': {
			globalPlayerVehicle.engineForce = -globalPlayerVehicle.breakForce;
		} break;

		case 'A': {
			globalPlayerVehicle.turnAngle = -0.75f;
		} break;

		case 'D': {
			globalPlayerVehicle.turnAngle = 0.75f;
		}	

		}
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
	globalMap = CreateGridMap((float)width, (float)height, 100);

	globalPlayerVehicle.vehicle.position = { (float)width / 2.0f, (float)height / 2.0f };
	globalPlayerVehicle.vehicle.angle = 0.0f;
	globalPlayerVehicle.vehicle.color = { 1.0f, 0.0f, 0.0f };
	globalPlayerVehicle.vehicle.width = 5.0f;
	globalPlayerVehicle.vehicle.length = 8.0f;

	globalPlayerVehicle.mass = 200.0f;
	globalPlayerVehicle.maxEngineForce = 1000.0f;
	globalPlayerVehicle.breakForce = 1000.0f;

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
				globalSelectedBuilding = globalHighlightedBuilding;
			}

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		globalPlayerVehicle.Update(globalTargetFrameS);

		Point mousePosition = WinMousePosition(window);
		globalHighlightedBuilding = globalMap.GetBuildingAtPoint(mousePosition);

		if (globalSelectedBuilding && globalHighlightedBuilding && globalSelectedBuilding != globalHighlightedBuilding) {
			ClearPath(&globalBuildingPath);

			globalBuildingPath = ConnectBuildings(&globalMap, globalSelectedBuilding, globalHighlightedBuilding);
		}

		WinDraw(window, globalRenderer);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalRenderer, context, rect);
		
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