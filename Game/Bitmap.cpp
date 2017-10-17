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
	unsigned int colorCode = getColorCode(color);

	if (top > bottom) {
		int tmp = top;
		top = bottom;
		bottom = tmp;
	}

	if (left > right) {
		int tmp = left;
		left = right;
		right = tmp;
	}

	if (top < 0) top = 0;
	if (bottom >= height) bottom = height - 1;
	
	if (left < 0) left = 0;
	if (right >= width) right - width - 1;

	for (int row = top; row < bottom; ++row) {
		for (int col = left; col < right; ++col) {
			unsigned int *pixel = (unsigned int *)memory + row * width + col;
			pixel[0] = colorCode;
		}
	}
}

float turnDirection(Point point1, Point point2, Point point3) {
	float dx1 = point2.x - point1.x;
	float dy1 = point2.y - point1.y;

	float dx2 = point3.x - point2.x;
	float dy2 = point3.y - point2.y;

	return (dx1 * dy2 - dx2 * dy1);
}

void Bitmap::drawQuad(Point points[4], Color color) {
	unsigned int colorCode = getColorCode(color);

	int minX = width;
	int minY = height;
	int maxX = 0;
	int maxY = 0;

	for (int i = 0; i < 4; ++i) {
		int pointX = (int)points[i].x;
		int pointY = (int)points[i].y;

		if (pointX < minX) minX = pointX;
		if (pointX > maxX) maxX = pointX;
		if (pointY < minY) minY = pointY;
		if (pointY > maxY) maxY = pointY;
	}

	if (minX < 0) minX = 0;
	if (maxX >= width) maxX = width - 1;
	if (minY < 0) minY = 0;
	if (maxY >= height) maxY = height - 1;

	unsigned int *pixel = 0;
	for (int row = minY; row < maxY; ++row) {
		for (int col = minX; col < maxX; ++col) {
			Point testPoint = { (float)col, (float)row };

			bool drawPoint = true;

			if (turnDirection(points[0], points[1], testPoint) < 0.0f) drawPoint = false;
			else if (turnDirection(points[1], points[2], testPoint) < 0.0f) drawPoint = false;
			else if (turnDirection(points[2], points[3], testPoint) < 0.0f) drawPoint = false;
			else if (turnDirection(points[3], points[0], testPoint) < 0.0f) drawPoint = false;

			if (drawPoint) {
				pixel = (unsigned int *)memory + row * width + col;
				pixel[0] = colorCode;
			}
		}
	}
}