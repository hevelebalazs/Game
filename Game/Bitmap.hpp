#pragma once

#include <windows.h>

#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

struct Bitmap {
	I32 width;
	I32 height;
	U32* memory;
	// TODO: get rid of this Windows specific thing
	BITMAPINFO info;
};

#define BitmapBytesPerPixel 4

void ResizeBitmap(Bitmap* bitmap, I32 width, I32 height);
V4 GetRandomColor();
U32 GetRandomColorCode();

V4 GetColor(F32 red, F32 green, F32 blue);
V4 GetAlphaColor(F32 red, F32 green, F32 blue, F32 alpha);

U32 GetColorCode(V4 color);
V4 GetColorFromColorCode(U32 colorCode);
V4 AddColors(V4 color1, V4 color2);
V4 InterpolateColors(V4 color1, F32 ratio, V4 color2);

void FloodfillBitmap(Bitmap* bitmap, I32 row, I32 col, V4 color, MemArena* tmpArena);
void FillBitmapWithColor(Bitmap* bitmap, V4 color);
void CopyScaledRotatedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, I32 toCenterRow, I32 toCenterCol, F32 width, F32 height, F32 rotationAngle);
void CopyStretchedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, I32 toLeft, I32 toRight, I32 toTop, I32 toBottom);
void DrawBitmapPolyOutline(Bitmap* bitmap, I32 polyN, I32* polyColRow, V4 color);
