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

Camera global_camera;
Canvas global_canvas;
UserInput global_user_input;
Bool32 global_running;

#if RUN_GAME
Game global_game;
#else
WorldLabState global_lab_state;
#endif

static void
func WinInit()
{
	global_canvas.camera = &global_camera;

#if RUN_GAME
	GameInit(&global_game, &global_canvas);
#else
	WorldLabInit(&global_lab_state, &global_canvas);
#endif
}

static void
func WinUpdate(Real32 seconds, UserInput *user_input)
{
#if RUN_GAME
	GameUpdate(&global_game, &global_canvas, seconds, user_input);
#else
	WorldLabUpdate(&global_lab_state, &global_canvas, seconds, user_input);
#endif
}

static LRESULT CALLBACK
func WinCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	UserInput *user_input = &global_user_input;
	switch(message)
	{
		case WM_SIZE:
		{
			RECT client_rect = {};
			GetClientRect(window, &client_rect);
			Int32 width  = client_rect.right - client_rect.left;
			Int32 height = client_rect.bottom - client_rect.top;

			Canvas *canvas = &global_canvas;
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
			global_running = false;
			break;
		}
		case WM_KEYDOWN:
		{
			Assert(wparam >= 0 && wparam < 256);
			UInt8 key_code = (UInt8)wparam;
			user_input->is_key_down[key_code] = true;
			user_input->key_toggle_count[key_code]++;
			break;
		}
		case WM_KEYUP:
		{
			Assert(wparam >= 0 && wparam < 256);
			UInt8 key_code = (UInt8)wparam;
			user_input->is_key_down[key_code] = false;
			user_input->key_toggle_count[key_code]++;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			user_input->is_key_down[VK_LBUTTON] = true;
			user_input->key_toggle_count[VK_LBUTTON]++;
			break;
		}
		case WM_LBUTTONUP:
		{
			user_input->is_key_down[VK_LBUTTON] = false;
			user_input->key_toggle_count[VK_LBUTTON]++;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			user_input->is_key_down[VK_RBUTTON] = true;
			user_input->key_toggle_count[VK_RBUTTON]++;
			break;
		}
		case WM_RBUTTONUP:
		{
			user_input->is_key_down[VK_RBUTTON] = false;
			user_input->key_toggle_count[VK_RBUTTON]++;
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
	POINT cursor_point = {};
	GetCursorPos(&cursor_point);
	ScreenToClient(window, &cursor_point);

	IntVec2 mouse_position = MakeIntPoint(cursor_point.y, cursor_point.x);
	return mouse_position;
}

Int32 CALLBACK
func WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, Int32 cmd_show)
{
	global_running = true;

	WinInit();

	WNDCLASS win_class = {};
	win_class.style = CS_OWNDC;
	win_class.lpfnWndProc = WinCallback;
	win_class.hInstance = instance;
	win_class.lpszClassName = "GameWindowClass";

	Verify(RegisterClass(&win_class));
	HWND window = CreateWindowEx(
		0,
		win_class.lpszClassName,
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

	UserInput *user_input = &global_user_input;

	LARGE_INTEGER counter_frequency;
	QueryPerformanceFrequency(&counter_frequency);

	LARGE_INTEGER last_counter;
	QueryPerformanceCounter(&last_counter);

	MSG message = {};
	while(global_running)
	{
		ResetKeyToggleCounts(user_input);
		user_input->mouse_pixel_position = GetMousePixelPosition(window);

		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		Int64 microseconds = counter.QuadPart - last_counter.QuadPart;
		Real32 milliseconds = (Real32(microseconds) * 1000.0f) / Real32(counter_frequency.QuadPart);
		Real32 seconds = 0.001f * milliseconds;
		last_counter = counter;

		WinUpdate(seconds, user_input);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		Canvas *canvas = &global_canvas;
		Bitmap *bitmap = &canvas->bitmap;
		BITMAPINFO bitmap_info = GetBitmapInfo(bitmap);

		Int32 width  = rect.right - rect.left;
		Int32 height = rect.bottom - rect.top;

		StretchDIBits(context,
					  0, 0, bitmap->width, bitmap->height,
					  0, 0, width, height,
					  bitmap->memory,
					  &bitmap_info,
					  DIB_RGB_COLORS,
					  SRCCOPY
		);
		ReleaseDC(window, context);
	}
	return 0;
}