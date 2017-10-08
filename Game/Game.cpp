#include <Windows.h>

static bool running;

LRESULT CALLBACK WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) 
{
	LRESULT result = 0;

	switch (message) {
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

void WinDraw(HWND window, HDC context)
{
	RECT rect;
	GetClientRect(window, &rect);

	HBRUSH brushBlack = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH brushRed = CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH brushWhite = CreateSolidBrush(RGB(255, 255, 255));

	FillRect(context, &rect, brushBlack);

	RECT rect1 = {};

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	rect1.left = rect.left + width / 4;
	rect1.right = rect.right - width / 2;

	rect1.bottom = rect.bottom - height / 4;
	rect1.top = rect.top + height / 4;

	FillRect(context, &rect1, brushRed);

	rect1.left = rect.left + width / 2;
	rect1.right = rect.right - width / 4;

	FillRect(context, &rect1, brushWhite);
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

		WinDraw(window, context);
	}

	return 0;
}