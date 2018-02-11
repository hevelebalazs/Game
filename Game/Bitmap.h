#pragma once

#include <windows.h>
#include "Math.h"
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

inline unsigned int ColorCode(Color color) {
	unsigned char red = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (red << 16) + (green << 8) + (blue);

	return colorCode;
}

inline Color ColorFromCode(unsigned int colorCode) {
	Color color = {};
	color.red   = (float)((colorCode & 0x00ff0000) >> 16) * (1.0f / 255.0f);
	color.green = (float)((colorCode & 0x0000ff00) >>  8) * (1.0f / 255.0f);
	color.blue  = (float)((colorCode & 0x000000ff) >>  0) * (1.0f / 255.0f);
	return color;
}

inline Color ColorAdd(Color color1, Color color2) {
	Color result = {};
	result.red   = (color1.red   + color2.red);
	result.green = (color1.green + color2.green);
	result.blue  = (color1.blue  + color2.blue);
	return result;
}

inline Color ColorSum(Color color1, Color color2) {
	Color result = {};
	result.red   = Min2(color1.red   + color2.red,   1.0f);
	result.green = Min2(color1.green + color2.green, 1.0f);
	result.blue  = Min2(color1.blue  + color2.blue,  1.0f);
	return result;
}

inline Color ColorMax(Color color1, Color color2) {
	Color result = {};
	result.red   = Max2(color1.red,   color2.red);
	result.green = Max2(color1.green, color2.green);
	result.blue  = Max2(color1.blue,  color2.blue);
	return result;
}

inline Color ColorProd(float times, Color color) {
	Color result = {};
	result.red   = (times * color.red);
	result.green = (times * color.green);
	result.blue  = (times * color.blue);
	return result;
}

inline Color ColorLerp(Color color1, float ratio, Color color2) {
	Color result = {};
	result.red   = Lerp(color1.red,   ratio, color2.red);
	result.green = Lerp(color1.green, ratio, color2.green);
	result.blue  = Lerp(color1.blue,  ratio, color2.blue);
	return result;
}