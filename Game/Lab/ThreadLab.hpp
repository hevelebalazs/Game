#pragma once

#include <Windows.h>

#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Type.hpp"
#include "../UserInput.hpp"

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
	RowPaintWorkList workList;
};

static void
func ThreadLabBlit(Canvas* canvas, HDC context, RECT rect)
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

static void
func PaintRow(Bitmap* bitmap, Int32 row, UInt32 colorCode)
{
	Assert(row >= 0 && row < bitmap->height);
	UInt32* pixel = bitmap->memory +(row * bitmap->width);
	for(Int32 col = 0; col < bitmap->width; col++) 
	{
		*pixel = colorCode;
		pixel++;
	}
}

static DWORD WINAPI
func RowPaintWorkProc(LPVOID parameter)
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

static void
func PushRowPaintWork(RowPaintWorkList* workList, RowPaintWork work)
{
	Assert(workList->workN < MaxRowPaintWorkListN);
	workList->works[workList->workN] = work;
	workList->workN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
}

static void
func ThreadLabInit(ThreadLabState* labState, Canvas* canvas)
{
	Camera* camera = canvas->camera;
	camera->unitInPixels = 1.0f;

	labState->workList.semaphore = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	labState->workList.semaphoreDone = CreateSemaphore(0, 0, MaxRowPaintWorkListN, 0);
	for(Int32 i = 0; i < 5; i++)
	{
		CreateThread(0, 0, RowPaintWorkProc, &labState->workList, 0, 0);
	}
}

static void
func ThreadLabUpdate(ThreadLabState* labState, Canvas* canvas)
{
	Vec4 backgroundColor = MakeColor(0.8f, 1.0f, 1.0f);
	Bitmap* bitmap = &canvas->bitmap;

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