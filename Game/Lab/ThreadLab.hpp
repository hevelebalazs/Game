#pragma once

#include <Windows.h>

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Type.hpp"
#include "../UserInput.hpp"

struct RowPaintWork 
{
	Bitmap *bitmap;
	I32 row;
	U32 color_code;
};

#define MaxRowPaintWorkListN 1024

struct RowPaintWorkList 
{
	RowPaintWork works[MaxRowPaintWorkListN];
	volatile I32 work_n;
	volatile I32 first_work_to_do;
	HANDLE semaphore;
	HANDLE semaphore_done;
};

struct ThreadLabState 
{
	RowPaintWorkList work_list;
};

static void
func ThreadLabBlit(Canvas *canvas, HDC context, RECT rect)
{
	I32 width = rect.right - rect.left;
	I32 height = rect.bottom - rect.top;

	Bitmap bitmap = canvas->bitmap;
	BITMAPINFO bitmap_info = GetBitmapInfo(&bitmap);
	StretchDIBits(context,
				  0, 0, bitmap.width, bitmap.height,
				  0, 0, width, height,
				  bitmap.memory,
				  &bitmap_info,
				  DIB_RGB_COLORS,
				  SRCCOPY
	);
}

static void
func PaintRow(Bitmap *bitmap, I32 row, U32 color_code)
{
	Assert(row >= 0 && row < bitmap->height);
	U32 *pixel = bitmap->memory +(row * bitmap->width);
	for(I32 col = 0; col < bitmap->width; col++) 
	{
		*pixel = color_code;
		pixel++;
	}
}

static DWORD WINAPI
func RowPaintWorkProc(LPVOID parameter)
{
	RowPaintWorkList *work_list = (RowPaintWorkList *)parameter;
	while(1) 
	{
		WaitForSingleObjectEx(work_list->semaphore, INFINITE, FALSE);
		I32 work_index = (I32)InterlockedIncrement((volatile U64 *)&work_list->first_work_to_do) - 1;
		RowPaintWork work = work_list->works[work_index];
		PaintRow(work.bitmap, work.row, work.color_code);
		ReleaseSemaphore(work_list->semaphore_done, 1, 0);
	}
}

static void
func PushRowPaintWork(RowPaintWorkList *work_list, RowPaintWork work)
{
	Assert(work_list->work_n < MaxRowPaintWorkListN);
	work_list->works[work_list->work_n] = work;
	work_list->work_n++;
	ReleaseSemaphore(work_list->semaphore, 1, 0);
}

static void
func ThreadLabInit(ThreadLabState *lab_state, Canvas *canvas)
{
	Camera *camera = canvas->camera;
	camera->unit_in_pixels = 1.0f;

	lab_state->work_list.semaphore = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	lab_state->work_list.semaphore_done = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	for(I32 i = 0; i < 5; i++)
	{
		CreateThread(0, 0, RowPaintWorkProc, &lab_state->work_list, 0, 0);
	}
}

static void
func ThreadLabUpdate(ThreadLabState *lab_state, Canvas *canvas)
{
	V4 background_color = MakeColor(0.8f, 1.0f, 1.0f);
	Bitmap *bitmap = &canvas->bitmap;

	U32 paint_color_code = GetRandomColorCode();

	RowPaintWorkList *work_list = &lab_state->work_list;
	work_list->work_n = 0;
	work_list->first_work_to_do = 0;
	for(I32 row = 0; row < bitmap->height; row++) 
	{
		RowPaintWork work = {};
		work.bitmap = bitmap;
		work.row = row;
		work.color_code = paint_color_code;
		PushRowPaintWork(work_list, work);
	}

	for(I32 row = 0; row < bitmap->height; row++)
	{
		WaitForSingleObjectEx(work_list->semaphore_done, INFINITE, FALSE);
	}
}