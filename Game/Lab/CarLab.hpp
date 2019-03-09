#pragma once

#include <Windows.h>

#include "Lab.hpp"

#include "../Bitmap.hpp"
#include "../Car.hpp"
#include "../Debug.hpp"
#include "../Geometry.hpp"
#include "../Math.hpp"
#include "../Memory.hpp"
#include "../Type.hpp"

#define CarLabTmpMemArenaSize (1 * MegaByte)

struct CarLabState 
{
	B32 running;
	Bitmap windowBitmap;
	Bitmap carBitmap;
	MemArena tmpArena;

	F32 zoomValue;
	F32 rotationAngle;
	F32 rotationAngleAdd;
};
static CarLabState gCarLabState;

static void func CarLabBlit(CarLabState* carLabState, HDC context, RECT rect)
{
	Bitmap* bitmap = &carLabState->windowBitmap;
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	BITMAPINFO bitmapInfo = GetBitmapInfo(bitmap);
	StretchDIBits(context,
				  0, 0, bitmap->width, bitmap->height,
				  0, 0, width, height,
				  bitmap->memory,
				  &bitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void func CarLabResize(CarLabState* carLabState, I32 width, I32 height)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	ResizeBitmap(windowBitmap, width, height);
}

static LRESULT CALLBACK func CarLabCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	CarLabState* carLabState = &gCarLabState;
	switch(message) 
	{
		case WM_SIZE: 
		{
			RECT clientRect = {};
			GetClientRect(window, &clientRect);
			I32 width  = clientRect.right - clientRect.left;
			I32 height = clientRect.bottom - clientRect.top;
			CarLabResize(carLabState, width, height);
			break;
		}
		case WM_PAINT: 
		{
			PAINTSTRUCT paint = {};
			HDC context = BeginPaint(window, &paint);

			RECT clientRect = {};
			GetClientRect(window, &clientRect);

			CarLabBlit(carLabState, context, clientRect);
	
			EndPaint(window, &paint);
			break;
		}
		case WM_KEYDOWN: 
		{
			WPARAM keyCode = wparam;
			switch(keyCode) 
			{
				case 'A':
				{
					carLabState->rotationAngleAdd = -0.1f;
					break;
				}
				case 'D':
				{
					carLabState->rotationAngleAdd = 0.1f;
					break;
				}
			}
			break;
		}
		case WM_KEYUP: 
		{
			WPARAM keyCode = wparam;
				
			switch(keyCode) 
			{
				case 'G':
				{
					GenerateCarBitmap(&carLabState->carBitmap, &carLabState->tmpArena);
					break;
				}
				case 'W':
				{
					carLabState->zoomValue *= 1.1f;
					break;
				}
				case 'S':
				{
					carLabState->zoomValue *= (1.0f / 1.1f);
					break;
				}
				case 'A':
				{
					carLabState->rotationAngleAdd = 0.0f;
					break;
				}
				case 'D':
				{
					carLabState->rotationAngleAdd = 0.0f;
					break;
				}
			}
			break;
		}
		case WM_DESTROY: 
		{
			carLabState->running = false;
			break;
		}
		case WM_CLOSE: 
		{
			carLabState->running = false;
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

static void func CarLabInit(CarLabState* carLabState, I32 windowWidth, I32 windowHeight)
{
	InitRandom();
	carLabState->running = true;
	CarLabResize(carLabState, windowWidth, windowHeight);

	carLabState->tmpArena = CreateMemArena(CarLabTmpMemArenaSize);

	Bitmap* carBitmap = &carLabState->carBitmap;
	AllocateCarBitmap(carBitmap);
	GenerateCarBitmap(carBitmap, &carLabState->tmpArena);

	carLabState->zoomValue = 1.0f;
}

static void func CarLabUpdate(CarLabState* carLabState)
{
	Bitmap* windowBitmap = &carLabState->windowBitmap;
	Bitmap* carBitmap = &carLabState->carBitmap;
	V4 backgroundColor = MakeColor(0.0f, 0.0f, 0.8f);
	FillBitmapWithColor(windowBitmap, backgroundColor);

	I32 halfWindowWidth  = windowBitmap->width / 2;
	I32 halfWindowHeight = windowBitmap->height / 2;
	carLabState->rotationAngle += carLabState->rotationAngleAdd;
	F32 rotationAngle = carLabState->rotationAngle;
	F32 width = carBitmap->width * carLabState->zoomValue;
	F32 height = carBitmap->height * carLabState->zoomValue;
	CopyScaledRotatedBitmap(carBitmap, windowBitmap, halfWindowHeight, halfWindowWidth, width, height, rotationAngle);
}

static void func CarLab(HINSTANCE instance)
{
	WNDCLASS winClass = {};
	winClass.style = CS_OWNDC;
	winClass.lpfnWndProc = CarLabCallback;
	winClass.hInstance = instance;
	winClass.lpszClassName = "CarLabWindowClass";

	Verify(RegisterClass(&winClass));
	HWND window = CreateWindowEx(
		0,
		winClass.lpszClassName,
		"CarLab",
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

	CarLabState* carLabState = &gCarLabState;

	RECT rect = {};
	GetClientRect(window, &rect);
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;
	CarLabInit(carLabState, width, height);
	
	MSG message = {};
	while(carLabState->running) 
	{
		while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		CarLabUpdate(carLabState);

		RECT rect = {};
		GetClientRect(window, &rect);

		HDC context = GetDC(window);
		CarLabBlit(carLabState, context, rect);
		ReleaseDC(window, context);
	}
}