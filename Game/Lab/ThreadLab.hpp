#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Type.hpp"

struct RowPaintWork 
{
	Bitmap* bitmap;
	Int32 row;
	UInt32 colorCode;
};

#define MaxRowPaintWorkListN 1024

struct RowPaintWorkList 
{
	RowPaintWork works[MaxRowPaintWorkListN];
	volatile Int32 workN;
	volatile Int32 firstWorkToDo;
	HANDLE semaphore;
	HANDLE semaphoreDone;
};

struct ThreadLabState 
{
	Camera camera;
	Canvas canvas;

	RowPaintWorkList workList;

	Bool32 running;
};
static ThreadLabState gThreadLabState;

static void func ThreadLabResize(ThreadLabState* labState, Int32 width, Int32 height)
{
	Camera* camera = &labState->camera;
	ResizeCamera(camera, width, height);

	Canvas* canvas = &labState->canvas;
	ResizeBitmap(&canvas->bitmap, width, height);
	canvas->camera = camera;
	camera->unitInPixels = 1.0f;
}

static void func ThreadLabBlit(Canvas* canvas, HDC context, RECT rect)
{
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas->bitmap;
	BITMAPINFO bitmapInfo = GetBitmapInfo(&bitmap);
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void func PaintRow(Bitmap* bitmap, Int32 row, UInt32 colorCode)
{
	Assert(row >= 0 && row < bitmap->height);
	UInt32* pixel = bitmap->memory +(row * bitmap->width);
	for(Int32 col = 0; col < bitmap->width; col++) 
	{
		*pixel = colorCode;
		pixel++;
	}
}

static DWORD WINAPI func RowPaintWorkProc(LPVOID parameter)
{
	RowPaintWorkList* workList = (RowPaintWorkList*)parameter;
	while(1) 
	{
		WaitForSingleObjectEx(workList->semaphore, INFINITE, FALSE);
		Int32 workIndex = (Int32)InterlockedIncrement((volatile UInt64*)&workList->firstWorkToDo) - 1;
		RowPaintWork work = workList->works[workIndex];
		PaintRow(work.bitmap, work.row, work.colorCode);
		ReleaseSemaphore(workList->semaphoreDone, 1, 0);
	}
}

static void func PushRowPaintWork(RowPaintWorkList* workList, RowPaintWork work)
{
	Assert(workList->workN < MaxRowPaintWorkListN);
	workList->works[workList->workN] = work;
	workList->workN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
}

static void func ThreadLabInit(ThreadLabState* labState, Int32 windowWidth, Int32 windowHeight)
{
	labState->running = true;
	ThreadLabResize(labState, windowWidth, windowHeight);
	labState->workList.semaphore = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	labState->workList.semaphoreDone = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	for(Int32 i = 0; i < 5; i++)
	{
		CreateThread(0, 0, RowPaintWorkProc, &labState->workList, 0, 0);
	}
}

static void func ThreadLabUpdate(ThreadLabState* labState)
{
	Canvas canvas = labState->canvas;
	Vec4 backgroundColor = MakeColor(0.8f, 1.0f, 1.0f);
	Bitmap* bitmap = &canvas.bitmap;

	UInt32 paintColorCode = GetRandomColorCode();

	RowPaintWorkList* workList = &labState->workList;
	workList->workN = 0;
	workList->firstWorkToDo = 0;
	for(Int32 row = 0; row < bitmap->height; row++) 
	{
		RowPaintWork work = {};
		work.bitmap = bitmap;
		work.row = row;
		work.colorCode = paintColorCode;
		PushRowPaintWork(workList, work);
	}

	for(Int32 row = 0; row < bitmap->height; row++)
	{
		WaitForSingleObjectEx(workList->semaphoreDone, INFINITE, FALSE);
	}
}

static LRESULT CALLBACK func ThreadLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	ThreadLabState* labState = &gThreadLabState;
	LRESULT result = 0;
	
	switch(message) 
	{
		case WM_SIZE: 
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			Int32 width = clientRect.right - clientRect.left;
			Int32 height = clientRect.bottom - clientRect.top;
			ThreadLabResize(labState, width, height);
			break;
		}
		case WM_PAINT: 
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			ThreadLabBlit(&labState->canvas, context, clientRect);

			EndPaint(window, &paint);
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
			labState->running = false;
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

static void func ThreadLab(HINSTANCE instance)
{
	ThreadLabState* labState = &gThreadLabState;

	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = ThreadLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "ThreadLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"ThreadLab",
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

	RECT rect = {};
	GetClientRect(window, &rect);
	Int32 width = rect.right - rect.left;
	Int32 height = rect.bottom - rect.top;
	ThreadLabInit(labState, width, height);

	MSG message = {};
	while(labState->running) 
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		ThreadLabUpdate(labState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		ThreadLabBlit(&labState->canvas, context, rect);
		ReleaseDC(window, context);
	}
}