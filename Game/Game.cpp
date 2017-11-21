// TODO: separate Windows and Game layers

#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "AutoHuman.h"
#include "AutoVehicle.h"
#include "Bitmap.h"
#include "GridMapCreator.h"
#include "Human.h"
#include "Path.h"
#include "PlayerHuman.h"
#include "PlayerVehicle.h"
#include "Renderer.h"
#include "Vehicle.h"

static bool running;

Renderer globalRenderer;
Map globalMap;
Intersection* globalSelectedIntersection;

Building* globalSelectedBuilding;
Building* globalHighlightedBuilding;

PathHelper globalPathHelper;
Path globalBuildingPath;

AutoHuman globalAutoHumans[100];
int globalAutoHumanCount = 100;

PlayerHuman globalPlayerHuman;

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

void GameDraw(Renderer renderer);

void WinDraw(HWND window, Renderer renderer) {
	Point mousePoint = WinMousePosition(window);
	Intersection *highlightIntersection = globalMap.GetIntersectionAtPoint(mousePoint, 20.0f);

	GameDraw(renderer);
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
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC context = BeginPaint(window, &paint);
		
			RECT clientRect;
			GetClientRect(window, &clientRect);

			WinUpdate(globalRenderer, context, clientRect);

			EndPaint(window, &paint);
			break;
		}

		case WM_KEYUP: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalPlayerHuman.moveUp = false;
					break;
				}
				case 'S': {
					globalPlayerHuman.moveDown = false;
					break;
				}
				case 'A': {
					globalPlayerHuman.moveLeft = false;
					break;
				}
				case 'D': {
					globalPlayerHuman.moveRight = false;
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN: {
			WPARAM keyCode = wparam;

			switch (keyCode) {
				case 'W': {
					globalPlayerHuman.moveUp = true;
					break;
				}
				case 'S': {
					globalPlayerHuman.moveDown = true;
					break;
				}
				case 'A': {
					globalPlayerHuman.moveLeft = true;
					break;
				}
				case 'D': {
					globalPlayerHuman.moveRight = true;
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

// TODO: move this to a math file?
static float RandomBetween(float left, float right) {
	return (left)+(right - left) * ((float)rand() / (float)RAND_MAX);
}

static void GameInit(int windowWidth, int windowHeight) {
	globalMap = CreateGridMap((float)windowWidth, (float)windowHeight, 100);
	globalPathHelper = PathHelperForMap(&globalMap);

	for (int i = 0; i < globalAutoHumanCount; ++i) {
		AutoHuman* autoHuman = &globalAutoHumans[i];
		Human* human = &autoHuman->human;

		Building* building = globalMap.GetRandomBuilding();

		human->map = &globalMap;
		autoHuman->inBuilding = building;
		human->position = building->connectPointClose;
		autoHuman->moveHelper = &globalPathHelper;

		human->needRed = RandomBetween(0.0f, 100.0f);
		human->needGreen = RandomBetween(0.0f, 100.0f);
		human->needBlue = RandomBetween(0.0f, 100.0f);

		autoHuman->needRedSpeed = RandomBetween(1.0f, 5.0f);
		autoHuman->needGreenSpeed = RandomBetween(1.0f, 5.0f);
		autoHuman->needBlueSpeed = RandomBetween(1.0f, 5.0f);
	}

	globalPlayerHuman.human.position = Point{(float)windowWidth, (float)windowHeight} * 0.5f;

	globalRenderer.camera.pixelCoordRatio = 10.0f;
}

// TODO: pass seconds to this function
// TODO: get rid of the mousePosition parameter?
static void GameUpdate(Point mousePosition) {
	for (int i = 0; i < globalAutoHumanCount; ++i) {
		AutoHuman* human = &globalAutoHumans[i];

		human->Update(globalTargetFrameS);
	}

	globalPlayerHuman.Update(globalTargetFrameS);

	globalHighlightedBuilding = globalMap.GetBuildingAtPoint(mousePosition);

	if (globalSelectedBuilding && globalHighlightedBuilding && globalSelectedBuilding != globalHighlightedBuilding) {
		ClearPath(&globalBuildingPath);

		// TODO: create a function for these?
		MapElem selectedBuildingElem = {};
		selectedBuildingElem.type = MapElemType::BUILDING;
		selectedBuildingElem.building = globalSelectedBuilding;

		MapElem highlightedBuildingElem = {};
		highlightedBuildingElem.type = MapElemType::BUILDING;
		highlightedBuildingElem.building = globalHighlightedBuilding;

		globalBuildingPath = ConnectElems(&globalMap, selectedBuildingElem, highlightedBuildingElem, &globalPathHelper);
	}

	globalRenderer.camera.center = globalPlayerHuman.human.position;
}

static void GameDraw(Renderer renderer) {
	globalMap.Draw(renderer);

	if (globalSelectedBuilding) {
		Color highlightColor = {0.0f, 1.0f, 1.0f};
		globalSelectedBuilding->HighLight(globalRenderer, highlightColor);
	}

	if (globalHighlightedBuilding) {
		Color highlightColor = {0.0f, 1.0f, 1.0f};
		globalHighlightedBuilding->HighLight(globalRenderer, highlightColor);
	}

	if (globalBuildingPath.nodeCount > 0 && globalSelectedBuilding && globalHighlightedBuilding) {
		Color color = {0.0f, 1.0f, 1.0f};
		DrawPath(&globalBuildingPath, globalRenderer, color, 5.0f);
	}

	for (int i = 0; i < globalAutoHumanCount; ++i) {
		Human* human = &globalAutoHumans[i].human;

		human->Draw(renderer);
	}

	globalPlayerHuman.human.Draw(renderer);
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

	GameInit(width, height);

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

		Point mousePosition = WinMousePosition(window);
		GameUpdate(mousePosition);

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