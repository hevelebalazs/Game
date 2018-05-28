#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "AutoHuman.h"
#include "AutoVehicle.h"
#include "Bitmap.h"
#include "Game.h"
#include "GridMap.h"
#include "Human.h"
#include "Path.h"
#include "PlayerHuman.h"
#include "PlayerVehicle.h"
#include "Renderer.h"
#include "Lab/CarLab.h"
#include "Lab/RoadLab.h"
#include "Vehicle.h"

static bool running;

GameState* globalGameState;
GameStorage globalGameStorage;

static float globalTargetFPS = 60.0f;
static float globalTargetFrameS = 1.0f / globalTargetFPS;
static float globalTargetFrameMS = globalTargetFrameS * 1000.0f;

static Point WinMousePosition(HWND window) {
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	Point point = {};
	point.x = (float)cursorPoint.x;
	point.y = (float)cursorPoint.y;

	point = PixelToCoord(*globalGameState->renderer.camera, point);

	return point;
}

void WinDraw(HWND window) {
	Point mousePoint = WinMousePosition(window);

	GameDraw(&globalGameStorage);
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
			WinResize(globalGameState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			WinUpdate(globalGameState->renderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalGameState->playerHuman.moveUp = false;
					globalGameState->playerVehicle.engineForce = 0.0f;
					break;
				}
				case 'S': {
					globalGameState->playerHuman.moveDown = false;
					globalGameState->playerVehicle.engineForce = 0.0f;
					break;
				}
				case 'A': {
					globalGameState->playerHuman.moveLeft = false;
					globalGameState->playerVehicle.turnDirection = 0.0f;
					break;
				}
				case 'D': {
					globalGameState->playerHuman.moveRight = false;
					globalGameState->playerVehicle.turnDirection = 0.0f;
					break;
				}
				case VK_TAB: {
					globalGameState->showFullMap = false;
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalGameState->playerHuman.moveUp = true;
					globalGameState->playerVehicle.engineForce = globalGameState->playerVehicle.maxEngineForce;
					break;
				}
				case 'S': {
					globalGameState->playerHuman.moveDown = true;
					globalGameState->playerVehicle.engineForce = -globalGameState->playerVehicle.breakForce;
					break;
				}
				case 'A': {
					globalGameState->playerHuman.moveLeft = true;
					globalGameState->playerVehicle.turnDirection = -1.0f;
					break;
				}
				case 'D': {
					globalGameState->playerHuman.moveRight = true;
					globalGameState->playerVehicle.turnDirection = 1.0f;
					break;
				}
				case 'F': {
					TogglePlayerVehicle(globalGameState);
					break;
				}
				case VK_TAB: {
					globalGameState->showFullMap = true;
					break;
				}
			}

			break;
		}

		case WM_LBUTTONDOWN: {
			if (!globalGameState->isPlayerVehicle) {
				Point mousePosition = WinMousePosition(window);
				ShootBullet(&globalGameState->playerHuman, mousePosition);
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

void RunGame(HINSTANCE instance)
{
	WNDCLASS windowClass = {};

	windowClass.style = CS_OWNDC;
	windowClass.lpfnWndProc = WinCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "GameWindowClass";

	if (!RegisterClass(&windowClass)) {
		OutputDebugStringA("Failed to register window class!\n");
		return;
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
		return;
	}

	HDC context = GetDC(window);
	MSG message;

	RECT rect;
	GetClientRect(window, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	GameInit(&globalGameStorage, width, height);
	globalGameState = globalGameStorage.gameState;

	WinResize(globalGameState, width, height);

	timeBeginPeriod(1);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	float elapsedS = globalTargetFrameS;

	running = true;
	while (running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT)
				running = false;

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		Point mousePosition = WinMousePosition(window);
		GameUpdate(&globalGameStorage, elapsedS, mousePosition);

		WinDraw(window);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalGameState->renderer, context, rect);

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
}


int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	CarLab(instance);
	// RunGame(instance);
	return 0;
}