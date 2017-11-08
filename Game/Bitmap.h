#pragma once
#include <Windows.h>
#include <math.h>
#include "Point.h"

struct Color {
	float red;
	float green;
	float blue;

	static Color Random();
};

struct Bitmap {
	const static int bytesPerPixel = 4;

	int width;
	int height;
	void* memory;
	BITMAPINFO info;
};
