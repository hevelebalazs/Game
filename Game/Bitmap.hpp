#pragma once

#include <windows.h>

#include "Math.hpp"
#include "Memory.hpp"
#include "Point.hpp"

struct Color {
	float red;
	float green;
	float blue;
	float alpha;
};

struct Bitmap {
	int width;
	int height;
	void* memory;
	// TODO: get rid of this Windows specific thing
	BITMAPINFO info;
};

#define BitmapBytesPerPixel 4

void ResizeBitmap(Bitmap* bitmap, int width, int height);
Color GetRandomColor();

Color GetColor(float red, float green, float blue);
Color GetAlphaColor(float red, float green, float blue, float alpha);

unsigned int GetColorCode(Color color);
Color GetColorFromColorCode(unsigned int colorCode);
Color AddColors(Color color1, Color color2);
Color InterpolateColors(Color color1, float ratio, Color color2);

void FloodfillBitmap(Bitmap* bitmap, int row, int col, Color color, MemArena* tmpArena);
void FillBitmapWithColor(Bitmap* bitmap, Color color);
void CopyScaledRotatedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, int toCenterRow, int toCenterCol, float width, float height, float rotationAngle);
void CopyStretchedBitmap(Bitmap* fromBitmap, Bitmap* toBitmap, int toLeft, int toRight, int toTop, int toBottom);
void DrawBitmapPolyOutline(Bitmap* bitmap, int polyN, int* polyColRow, Color color);
