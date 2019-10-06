#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Game.hpp"
#include "Type.hpp"
#include "UserInput.hpp"

// #include "Lab/CombatLab.hpp"
#include "Lab/TextLab.hpp"
#include "Lab/ThreadLab.hpp"
#include "Lab/WorldLab.hpp"

#define RUN_GAME 1

Camera gCamera;
Canvas gCanvas;
UserInput gUserInput;
Bool32 gRunning;

#if RUN_GAME
Game gGame;
#else
WorldLabState gLabState;
#endif

static void
func WinInit()
{
	gCanvas.camera = &gCamera;

#if RUN_GAME
	GameInit(&gGame, &gCanvas);
#else
	WorldLabInit(&gLabState, &gCanvas);
#endif
}

static void
func WinUpdate(Real32 seconds, UserInput *userInput)
{
#if RUN_GAME
	GameUpdate(&gGame, &gCanvas, seconds, userInput);
#else
	WorldLabUpdate(&gLabState, &gCanvas, seconds, userInput);
#endif
}

static LRESULT CALLBACK
func WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	UserInput *userInput = &gUserInput;
	switch(message)
	{
		case WM_SIZE:
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			Int32 width  = clientRect.right - clientRect.left;
			Int32 height = clientRect.bottom - clientRect.top;

			Canvas *canvas = &gCanvas;
			Assert(canvas != 0);

			Camera *camera = canvas->camera;
			Assert(camera != 0);

			ResizeCamera(camera, width, height);
			ResizeBitmap(&canvas->bitmap, width, height);

			break;
		}
		case WM_SETCURSOR:
		{
			HCURSOR cursor = LoadCursor(0, IDC_ARROW);
			SetCursor(cursor);
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
		{
			gRunning = false;
			break;
		}
		case WM_KEYDOWN:
		{
			Assert(wparam >= 0 && wparam < 256);
			UInt8 keyCode = (UInt8)wparam;
			userInput->isKeyDown[keyCode] = true;
			userInput->keyToggleCount[keyCode]++;
			break;
		}
		case WM_KEYUP:
		{
			Assert(wparam >= 0 && wparam < 256);
			UInt8 keyCode = (UInt8)wparam;
			userInput->isKeyDown[keyCode] = false;
			userInput->keyToggleCount[keyCode]++;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			userInput->isKeyDown[VK_LBUTTON] = true;
			userInput->keyToggleCount[VK_LBUTTON]++;
			break;
		}
		case WM_LBUTTONUP:
		{
			userInput->isKeyDown[VK_LBUTTON] = false;
			userInput->keyToggleCount[VK_LBUTTON]++;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			userInput->isKeyDown[VK_RBUTTON] = true;
			userInput->keyToggleCount[VK_RBUTTON]++;
			break;
		}
		case WM_RBUTTONUP:
		{
			userInput->isKeyDown[VK_RBUTTON] = false;
			userInput->keyToggleCount[VK_RBUTTON]++;
			break;
		}
		default:
		{
			result = DefWindowProc(window, message, wparam, lparam);
			break;
		}
	}
	return result;
}

static IntVec2
func GetMousePixelPosition(HWND window)
{
	POINT cursorPoint = {};
	GetCursorPos(&cursorPoint);
	ScreenToClient(window, &cursorPoint);

	IntVec2 mousePosition = MakeIntPoint(cursorPoint.y, cursorPoint.x);
	return mousePosition;
}

Int32 CALLBACK
func WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, Int32 cmdShow)
{
	gRunning = true;

	WinInit();

	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = WinCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "GameWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
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
	Assert(window != 0);

	UserInput *userInput = &gUserInput;

	LARGE_INTEGER counterFrequency;
	QueryPerformanceFrequency(&counterFrequency);

	LARGE_INTEGER lastCounter;
	QueryPerformanceCounter(&lastCounter);

	MSG message = {};
	while(gRunning)
	{
		ResetKeyToggleCounts(userInput);
		userInput->mousePixelPosition = GetMousePixelPosition(window);

		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		Int64 microSeconds = counter.QuadPart - lastCounter.QuadPart;
		Real32 milliSeconds = (Real32(microSeconds) * 1000.0f) / Real32(counterFrequency.QuadPart);
		Real32 seconds = 0.001f * milliSeconds;
		lastCounter = counter;

		WinUpdate(seconds, userInput);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		Canvas *canvas = &gCanvas;
		Bitmap *bitmap = &canvas->bitmap;
		BITMAPINFO bitmapInfo = GetBitmapInfo(bitmap);

		Int32 width  = rect.right - rect.left;
		Int32 height = rect.bottom - rect.top;

		StretchDIBits(context,
					  0, 0, bitmap->width, bitmap->height,
					  0, 0, width, height,
					  bitmap->memory,
					  &bitmapInfo,
					  DIB_RGB_COLORS,
					  SRCCOPY
		);
		ReleaseDC(window, context);
	}
	return 0;
}