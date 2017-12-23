#pragma once

#include <windows.h>
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
	// TODO: get rid of this Windows specific thing
	BITMAPINFO info;
};

#define BitmapBytesPerPixel 4

Color RandomColor();