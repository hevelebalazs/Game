#pragma once
#include <Windows.h>
#include <math.h>
#include "Point.h"

struct Color {
	float red;
	float green;
	float blue;
};

struct Bitmap {
	int width;
	int height;
	void* memory;
	BITMAPINFO info;
};

#define BitmapBytesPerPixel 4

Color RandomColor();