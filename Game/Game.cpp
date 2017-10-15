#include <Windows.h>
#include "GridMapCreator.h"

static bool running;

Map map;

LRESULT CALLBACK WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) 
{
	LRESULT result = 0;

	switch (message) {
	case WM_PAINT: {
		PAINTSTRUCT paint = {};
		HDC context = BeginPaint(window, &paint);
		map.draw(context);
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

	running = true;
	while (running) {
		BOOL result = GetMessageA(&message, 0, 0, 0);
		if (result > 0) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}
		else {
			break;
		}
	}

	return 0;
}