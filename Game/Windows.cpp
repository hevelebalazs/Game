// TODO: separate Windows and Game layers

#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "AutoHuman.h"
#include "AutoVehicle.h"
#include "Bitmap.h"
#include "Game.h"
#include "GridMapCreator.h"
#include "Human.h"
#include "Path.h"
#include "PlayerHuman.h"
#include "PlayerVehicle.h"
#include "Renderer.h"
#include "Vehicle.h"

static bool running;

GameState globalGameState;

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
	camera->screenSize = Point{(float)width, (float)height};
	camera->center = PointProd(0.5f, camera->screenSize);

	Color background = Color{0.0f, 0.0f, 0.0f};

	ClearScreen(globalGameState.renderer, background);
}

// TODO: move this through camera
static Point WinMousePosition(HWND window) {
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	point = PixelToCoord(globalGameState.renderer.camera, point);

	return point;
}

void WinDraw(HWND window) {
	Point mousePoint = WinMousePosition(window);

	GameDraw(&globalGameState);
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

//TODO: fetch all the user input and pass it to GameUpdate?
LRESULT CALLBACK WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	switch (message) {
		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
			WinResize(&globalGameState.renderer, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			WinUpdate(globalGameState.renderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalGameState.playerHuman.moveUp = false;
					globalGameState.playerVehicle.engineForce = 0.0f;
					break;
				}
				case 'S': {
					globalGameState.playerHuman.moveDown = false;
					globalGameState.playerVehicle.engineForce = 0.0f;
					break;
				}
				case 'A': {
					globalGameState.playerHuman.moveLeft = false;
					globalGameState.playerVehicle.turnAngle = 0.0f;
					break;
				}
				case 'D': {
					globalGameState.playerHuman.moveRight = false;
					globalGameState.playerVehicle.turnAngle = 0.0f;
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalGameState.playerHuman.moveUp = true;
					globalGameState.playerVehicle.engineForce = globalGameState.playerVehicle.maxEngineForce;
					break;
				}
				case 'S': {
					globalGameState.playerHuman.moveDown = true;
					globalGameState.playerVehicle.engineForce = -globalGameState.playerVehicle.breakForce;
					break;
				}
				case 'A': {
					globalGameState.playerHuman.moveLeft = true;
					globalGameState.playerVehicle.turnAngle = -0.75f;
					break;
				}
				case 'D': {
					globalGameState.playerHuman.moveRight = true;
					globalGameState.playerVehicle.turnAngle = 0.75f;
					break;
				}
				case 'F': {
					TogglePlayerVehicle(&globalGameState);
					break;
				}
			}

			break;
		}

		case WM_DESTROY: {
			running = false;
			break;
		}

		case WM_CLOSE: {
			running = false;
			break;
		}

		default: {
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}

	return result;
}

// TODO: create a GameState struct for the globals?
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

	GameInit(&globalGameState, width, height);

	timeBeginPeriod(1);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	float elapsedS = globalTargetFrameS;

	running = true;
	while (running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) running = false;

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		Point mousePosition = WinMousePosition(window);
		GameUpdate(&globalGameState, elapsedS, mousePosition);

		WinDraw(window);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalGameState.renderer, context, rect);

		ReleaseDC(window, context);

		LARGE_INTEGER nowCounter;
		QueryPerformanceCounter(&nowCounter);

		long long elapsedUS = nowCounter.QuadPart - lastCounter.QuadPart;
		float elapsedMS = ((float)elapsedUS * 1000.0f) / (float)counterFrequency.QuadPart;

		if (elapsedMS < globalTargetFrameMS) {
			DWORD sleepMS = (DWORD)(globalTargetFrameMS - elapsedMS);
			Sleep(sleepMS);
			elapsedS = globalTargetFrameS;
		}
		else {
			elapsedS = elapsedMS * 0.001f;
		}

		QueryPerformanceCounter(&lastCounter);
	}

	return 0;
}