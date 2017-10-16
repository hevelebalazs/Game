#pragma once
#include <Windows.h>

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
	void drawPixel(int row, int col, Color color);
	void drawRect(int top, int left, int bottom, int right, Color color);
};
