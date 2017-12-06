#include "Geometry.h"
#include "Point.h"
#include "Renderer.h"

void SmoothZoom(Camera* camera, float pixelCoordRatio) {
	camera->zoomTargetRatio = pixelCoordRatio;
}

void UpdateCamera(Camera* camera, float seconds) {
	if (camera->pixelCoordRatio == camera->zoomTargetRatio) {
		return;
	}
	else if (camera->pixelCoordRatio < camera->zoomTargetRatio) {
		camera->pixelCoordRatio += seconds * camera->zoomSpeed;
		if (camera->pixelCoordRatio > camera->zoomTargetRatio) {
			camera->pixelCoordRatio = camera->zoomTargetRatio;
		}
	}
	else {
		camera->pixelCoordRatio -= seconds * camera->zoomSpeed;
		if (camera->pixelCoordRatio < camera->zoomTargetRatio) {
			camera->pixelCoordRatio = camera->zoomTargetRatio;
		}
	}
}

float CoordXtoPixel(Camera camera, float coordX) {
	return (0.5f * camera.screenSize.x) + (camera.pixelCoordRatio * (coordX - camera.center.x));
}

float CoordYtoPixel(Camera camera, float coordY) {
	return (0.5f * camera.screenSize.y) + (camera.pixelCoordRatio * (coordY - camera.center.y));
}

Point PixelToCoord(Camera camera, Point pixel) {
	Point screenCenter = PointProd(0.5f, camera.screenSize);

	return PointSum(camera.center, PointProd(1.0f / camera.pixelCoordRatio, PointDiff(pixel, screenCenter)));
}

Point CoordToPixel(Camera camera, Point coord) {
	Point screenCenter = PointProd(0.5f, camera.screenSize);

	return PointSum(screenCenter, PointProd(camera.pixelCoordRatio, PointDiff(coord, camera.center)));
}

static unsigned int getColorCode(Color color) {
	unsigned char red = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (red << 16) + (green << 8) + (blue);

	return colorCode;
}

void ClearScreen(Renderer renderer, Color color) {
	unsigned int colorCode = getColorCode(color);

	unsigned int *pixel = (unsigned int*)renderer.bitmap.memory;
	for (int row = 0; row < renderer.bitmap.height; ++row) {
		for (int col = 0; col < renderer.bitmap.width; ++col) {
			*pixel = colorCode;
			++pixel;
		}
	}
}

void DrawGridLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth) {
	float left = 0.0f;
	float right = 0.0f;
	float top = 0.0f;
	float bottom = 0.0f;

	if (point1.x == point2.x) {
		left = point1.x - lineWidth * 0.5f;
		right = point1.x + lineWidth * 0.5f;

		if (point1.y < point2.y) {
			top = point1.y;
			bottom = point2.y;
		}
		else {
			top = point2.y;
			bottom = point1.y;
		}
	}
	else if (point1.y == point2.y) {
		top = point1.y - lineWidth * 0.5f;
		bottom = point1.y + lineWidth * 0.5f;

		if (point1.x < point2.x) {
			left = point1.x;
			right = point2.x;
		}
		else {
			left = point2.x;
			right = point1.x;
		}
	}

	DrawRect(renderer, top, left, bottom, right, color);
}

void DrawLine(Renderer renderer, Point point1, Point point2, Color color, float lineWidth) {
	Point direction = PointDirection(point2, point1);

	float tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	float halfLineWidth = lineWidth * 0.5f;
	Point drawPoints[4] = {};

	drawPoints[0] = PointDiff(point1, PointProd(halfLineWidth, direction));
	drawPoints[1] = PointSum (point1, PointProd(halfLineWidth, direction));
	drawPoints[2] = PointSum (point2, PointProd(halfLineWidth, direction));
	drawPoints[3] = PointDiff(point2, PointProd(halfLineWidth, direction));
	DrawQuad(renderer, drawPoints, color);
}

// TODO: make this function take two points instead of four floats?
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color) {
	unsigned int colorCode = getColorCode(color);

	Camera camera = renderer.camera;
	int topPixel =    (int)CoordYtoPixel(camera, top);
	int leftPixel =   (int)CoordXtoPixel(camera, left);
	int bottomPixel = (int)CoordYtoPixel(camera, bottom);
	int rightPixel =  (int)CoordXtoPixel(camera, right);

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

	Bitmap bitmap = renderer.bitmap;

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

void DrawQuad(Renderer renderer, Point points[4], Color color) {
	unsigned int colorCode = getColorCode(color);

	for (int i = 0; i < 4; ++i) {
		points[i] = CoordToPixel(renderer.camera, points[i]);
	}

	Bitmap bitmap = renderer.bitmap;
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