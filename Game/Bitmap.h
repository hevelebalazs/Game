#pragma once
#include <Windows.h>
#include "Point.h"

struct Color {
	float red;
	float green;
	float blue;
};

struct Bitmap {
	const static int bytesPerPixel = 4;

	int width;
	int height;
	void *memory;
	BITMAPINFO info;

	void clear(Color color);
	void drawRect(float top, float left, float bottom, float right, Color color);
	void drawQuad(Point points[4], Color color);
};
