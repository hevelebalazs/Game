#include "Geometry.h"
#include "Point.h"
#include "Renderer.h"

float Camera::CoordXtoPixel(float coordX) {
	return (0.5f * screenSize.x) + (pixelCoordRatio * (coordX - center.x));
}

float Camera::CoordYtoPixel(float coordY) {
	return (0.5f * screenSize.y) + (pixelCoordRatio * (coordY - center.y));
}

Point Camera::PixelToCoord(Point pixel) {
	Point screenCenter = 0.5f * screenSize;

	return center + ((1.0f / pixelCoordRatio) * (pixel - screenCenter));
}

Point Camera::CoordToPixel(Point coord) {
	Point screenCenter = 0.5f * screenSize;

	return screenCenter + (pixelCoordRatio * (coord - center));
}

static unsigned int getColorCode(Color color) {
	unsigned char red = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (red << 16) + (green << 8) + (blue);

	return colorCode;
}

void Renderer::Clear(Color color) {
	unsigned int colorCode = getColorCode(color);

	unsigned int *pixel = (unsigned int*)bitmap.memory;
	for (int row = 0; row < bitmap.height; ++row) {
		for (int col = 0; col < bitmap.width; ++col) {
			*pixel = colorCode;
			++pixel;
		}
	}
}

void Renderer::DrawLine(Point point1, Point point2, Color color, float lineWidth) {
	Point direction = PointDirection(point2, point1);

	float tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	float halfLineWidth = lineWidth * 0.5f;
	Point drawPoints[4] = {};

	drawPoints[0] = point1 - (halfLineWidth * direction);
	drawPoints[1] = point1 + (halfLineWidth * direction);
	drawPoints[2] = point2 + (halfLineWidth * direction);
	drawPoints[3] = point2 - (halfLineWidth * direction);
	this->DrawQuad(drawPoints, color);
}

// TODO: make this function take two points instead of four floats?
void Renderer::DrawRect(float top, float left, float bottom, float right, Color color) {
	unsigned int colorCode = getColorCode(color);

	int topPixel = (int)camera.CoordYtoPixel(top);
	int leftPixel = (int)camera.CoordXtoPixel(left);
	int bottomPixel = (int)camera.CoordYtoPixel(bottom);
	int rightPixel = (int)camera.CoordXtoPixel(right);

	if (topPixel > bottomPixel) {
		int tmp = topPixel;
		topPixel = bottomPixel;
		bottomPixel = tmp;
	}

	if (leftPixel > rightPixel) {
		int tmp = leftPixel;
		leftPixel = rightPixel;
		rightPixel = tmp;
	}

	if (topPixel < 0) topPixel = 0;
	if (bottomPixel >= bitmap.height) bottomPixel = bitmap.height - 1;

	if (leftPixel < 0) leftPixel = 0;
	if (rightPixel >= bitmap.width) rightPixel = bitmap.width - 1;

	for (int row = topPixel; row < bottomPixel; ++row) {
		for (int col = leftPixel; col < rightPixel; ++col) {
			unsigned int *pixel = (unsigned int *)bitmap.memory + row * bitmap.width + col;
			pixel[0] = colorCode;
		}
	}
}

// TODO: this is defined in Geometry, use that
float turnDirection(Point point1, Point point2, Point point3) {
	float dx1 = point2.x - point1.x;
	float dy1 = point2.y - point1.y;

	float dx2 = point3.x - point2.x;
	float dy2 = point3.y - point2.y;

	return (dx1 * dy2 - dx2 * dy1);
}

void Renderer::DrawQuad(Point points[4], Color color) {
	unsigned int colorCode = getColorCode(color);

	for (int i = 0; i < 4; ++i) {
		points[i] = camera.CoordToPixel(points[i]);
	}

	int minX = bitmap.width;
	int minY = bitmap.height;
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
	if (maxX >= bitmap.width) maxX = bitmap.width - 1;
	if (minY < 0) minY = 0;
	if (maxY >= bitmap.height) maxY = bitmap.height - 1;

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
				pixel = (unsigned int *)bitmap.memory + row * bitmap.width + col;
				pixel[0] = colorCode;
			}
		}
	}
}