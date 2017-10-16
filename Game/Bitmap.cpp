#include "Bitmap.h"

static unsigned int getColorCode(Color color) {
	unsigned char red = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (red << 16) + (green << 8) + (blue);

	return colorCode;
}

void Bitmap::clear(Color color) {
	unsigned int colorCode = getColorCode(color);

	unsigned int *pixel = (unsigned int*)memory;
	for (int row = 0; row < height; ++row) {
		for (int col = 0; col < width; ++col) {
			*pixel = colorCode;
			++pixel;
		}
	}
}

void Bitmap::drawPixel(int row, int col, Color color) {
	unsigned int colorCode = getColorCode(color);

	if (row < 0 || row >= height) return;
	if (col < 0 || col >= width) return;

	unsigned int *memoryUint = (unsigned int*)memory;

	memoryUint[row * width + col] = colorCode;
}

void Bitmap::drawRect(int top, int left, int bottom, int right, Color color) {
	for (int row = top; row < bottom; ++row) {
		for (int col = left; col < right; ++col) {
			drawPixel(row, col, color);
		}
	}
}