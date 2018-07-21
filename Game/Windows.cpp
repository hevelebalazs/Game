#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "AutoHuman.hpp"
#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Game.hpp"
#include "GridMap.hpp"
#include "Human.hpp"
#include "Path.hpp"
#include "PlayerHuman.hpp"
#include "Type.hpp"

#include "Lab/CarLab.hpp"
#include "Lab/MapLab.hpp"
#include "Lab/RoadLab.hpp"
#include "Lab/ThreadLab.hpp"

static B32 running;

GameState* globalGameState;
GameStorage globalGameStorage;

static F32 globalTargetFPS = 60.0f;
static F32 globalTargetFrameS = 1.0f / globalTargetFPS;
static F32 globalTargetFrameMS = globalTargetFrameS * 1000.0f;

static V2 WinMousePosition(HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	V2 point = {};
	point.x = (F32)cursorPoint.x;
	point.y = (F32)cursorPoint.y;

	point = PixelToUnit(globalGameState->canvas.camera, point);

	return point;
}

void WinDraw(HWND window)
{
	V2 mousePoint = WinMousePosition(window);

	GameDraw(&globalGameStorage);
}

void WinUpdate(Canvas canvas, HDC context, RECT clientRect)
{
	I32 windowWidth = clientRect.right - clientRect.left;
	I32 windowHeight = clientRect.bottom - clientRect.top;

	Bitmap bitmap = canvas.bitmap;

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
			I32 width = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			WinResize(globalGameState, width, height);
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);

			RECT clientRect;
			GetClientRect(window, &clientRect);

			WinUpdate(globalGameState->canvas, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalGameState->playerHuman.moveUp = false;
					globalGameState->playerCar.engineForce = 0.0f;
					break;
				}
				case 'S': {
					globalGameState->playerHuman.moveDown = false;
					globalGameState->playerCar.engineForce = 0.0f;
					break;
				}
				case 'A': {
					globalGameState->playerHuman.moveLeft = false;
					globalGameState->playerCar.turnDirection = 0.0f;
					break;
				}
				case 'D': {
					globalGameState->playerHuman.moveRight = false;
					globalGameState->playerCar.turnDirection = 0.0f;
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
					globalGameState->playerCar.engineForce = MaxCarEngineForce;
					break;
				}
				case 'S': {
					globalGameState->playerHuman.moveDown = true;
					globalGameState->playerCar.engineForce = -MaxCarBrakeForce;
					break;
				}
				case 'A': {
					globalGameState->playerHuman.moveLeft = true;
					globalGameState->playerCar.turnDirection = -1.0f;
					break;
				}
				case 'D': {
					globalGameState->playerHuman.moveRight = true;
					globalGameState->playerCar.turnDirection = 1.0f;
					break;
				}
				case 'F': {
					TogglePlayerCar(globalGameState);
					break;
				}
			}

			break;
		}

		case WM_LBUTTONDOWN: {
			if (!globalGameState->isPlayerCar) {
				V2 mousePosition = WinMousePosition(window);
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

	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	GameInit(&globalGameStorage, width, height);
	globalGameState = globalGameStorage.gameState;

	WinResize(globalGameState, width, height);

	timeBeginPeriod(1);

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	F32 elapsedS = globalTargetFrameS;

	running = true;
	while (running) {
		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT)
				running = false;

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		V2 mousePosition = WinMousePosition(window);
		GameUpdate(&globalGameStorage, elapsedS, mousePosition);

		WinDraw(window);

		RECT rect;
		GetClientRect(window, &rect);

		HDC context = GetDC(window);

		WinUpdate(globalGameState->canvas, context, rect);

		ReleaseDC(window, context);

		LARGE_INTEGER nowCounter;
		QueryPerformanceCounter(&nowCounter);

		I64 elapsedUS = nowCounter.QuadPart - lastCounter.QuadPart;
		F32 elapsedMS = ((F32)elapsedUS * 1000.0f) / (F32)counterFrequency.QuadPart;

		if (elapsedMS < globalTargetFrameMS) {
			DWORD sleepMS = (DWORD)(globalTargetFrameMS - elapsedMS);
			Sleep(sleepMS);
			elapsedS = globalTargetFrameS;
		} else {
			elapsedS = elapsedMS * 0.001f;
		}

		QueryPerformanceCounter(&lastCounter);
	}
}


I32 CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, I32 cmdShow)
{
	RunGame(instance);
	return 0;
}