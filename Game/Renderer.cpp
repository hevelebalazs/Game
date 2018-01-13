#include "Debug.h"
#include "Geometry.h"
#include "Math.h"
#include "Memory.h"
#include "Point.h"
#include "Renderer.h"

static inline unsigned int* GetPixelAddress(Bitmap bitmap, int row, int col) {
	return (unsigned int*)bitmap.memory + row * bitmap.width + col;
}

static inline unsigned int GetPixel(Bitmap bitmap, int row, int col) {
	unsigned int* pixelAddress = GetPixelAddress(bitmap, row, col);

	return *pixelAddress;
}

static inline void SetPixel(Bitmap bitmap, int row, int col, unsigned int colorCode) {
	unsigned int* pixelAddress = GetPixelAddress(bitmap, row, col);

	*pixelAddress = colorCode;
}

static inline void SetPixelCheck(Bitmap bitmap, int row, int col, int colorCode) {
	if ((row >= 0 && row < bitmap.height) && (col >= 0 && col < bitmap.width))
		SetPixel(bitmap, row, col, colorCode);
}

static inline unsigned int ColorCode(Color color) {
	unsigned char red = (unsigned char)(color.red * 255);
	unsigned char green = (unsigned char)(color.green * 255);
	unsigned char blue = (unsigned char)(color.blue * 255);

	unsigned int colorCode = (red << 16) + (green << 8) + (blue);

	return colorCode;
}

static inline unsigned int MixColorCodes(unsigned int colorCode1, unsigned int colorCode2) {
	// TODO: is ANDing a good idea here?
	return (colorCode1 & colorCode2);
}

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask) {
	unsigned int* pixel = (unsigned int*)bitmap.memory;
	unsigned int* maskPixel = (unsigned int*)mask.memory;

	for (int row = 0; row < bitmap.height; ++row) {
		for (int col = 0; col < bitmap.width; ++col) {
			*pixel = MixColorCodes(*pixel, *maskPixel);

			pixel++;
			maskPixel++;
		}
	}
}

struct PixelPosition {
	int row;
	int col;
};

void FloodFill(Renderer renderer, Point start, Color color, MemArena* tmpArena) {
	unsigned int colorCode = ColorCode(color);

	Bitmap bitmap = renderer.bitmap;
	Camera camera = *renderer.camera;

	int positionCount = 0;
	PixelPosition* positions = ArenaPushArray(tmpArena, PixelPosition, 0);
	
	int row = (int)CoordYtoPixel(camera, start.y);
	int col = (int)CoordXtoPixel(camera, start.x);

	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionCount++;

	bool fillHorizontally = true;
	int directionSwitchPosition = 1;

	ArenaPush(tmpArena, int, row);
	ArenaPush(tmpArena, int, col);
	positionCount++;

	SetPixel(bitmap, positions[0].row, positions[0].col, colorCode);

	for (int i = 0; i < positionCount; ++i) {
		if (i == directionSwitchPosition) {
			fillHorizontally = (!fillHorizontally);
			directionSwitchPosition = positionCount + 1;
		}

		int row = positions[i].row;
		int col = positions[i].col;

		unsigned int* pixelStart = GetPixelAddress(bitmap, row, col);
		unsigned int* pixel = 0;

		if (fillHorizontally) {
			pixel = pixelStart;
			for (int left = col - 1; left >= 0; --left) {
				pixel--;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, left);
				positionCount++;
			}

			pixel = pixelStart;
			for (int right = col + 1; right < bitmap.width; ++right) {
				pixel++;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, int, row);
				ArenaPush(tmpArena, int, right);
				positionCount++;
			}
		}
		else {
			pixel = pixelStart;
			for (int top = row - 1; top >= 0; --top) {
				pixel -= bitmap.width;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, int, top);
				ArenaPush(tmpArena, int, col);
				positionCount++;
			}

			pixel = pixelStart;
			for (int bottom = row + 1; bottom < bitmap.height; ++bottom) {
				pixel += bitmap.width;
			
				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, int, bottom);
				ArenaPush(tmpArena, int, col);
				positionCount++;
			}
		}
	}

	ArenaPopTo(tmpArena, positions);
}

void SmoothZoom(Camera* camera, float altitude) {
	camera->targetAltitude = altitude;
}

void UpdateCamera(Camera* camera, float seconds) {
	if (camera->altitude == camera->targetAltitude) {
		return;
	}
	else if (camera->altitude < camera->targetAltitude) {
		camera->altitude += seconds * camera->moveSpeed;
		if (camera->altitude > camera->targetAltitude)
			camera->altitude = camera->targetAltitude;
	}
	else {
		camera->altitude -= seconds * camera->moveSpeed;
		if (camera->altitude < camera->targetAltitude)
			camera->altitude = camera->targetAltitude;
	}
}

float CoordXtoPixel(Camera camera, float coordX) {
	// TODO: add unitInPixels to Camera?
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	return (camera.screenSize.x * 0.5f) + ((coordX - camera.center.x) * (unitInPixels));
}

float CoordYtoPixel(Camera camera, float coordY) {
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	return (camera.screenSize.y * 0.5f) + ((coordY - camera.center.y) * (unitInPixels));
}

Point CoordToPixel(Camera camera, Point coord) {
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	Point result =  Point {
		CoordXtoPixel(camera, coord.x),
		CoordYtoPixel(camera, coord.y)
	};

	return result;
}

float PixelToCoordX(Camera camera, float pixelX) {
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	float result = camera.center.x + ((pixelX - (camera.screenSize.x * 0.5f)) / unitInPixels);
	return result;
}

float PixelToCoordY(Camera camera, float pixelY) {
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	float result = camera.center.y + ((pixelY - (camera.screenSize.y * 0.5f)) / unitInPixels);
	return result;
}

Point PixelToCoord(Camera camera, Point pixel) {
	Assert(camera.altitude != 0.0f);
	float unitInPixels = (camera.screenSize.y / camera.altitude);

	Point result = {};
	result.x = PixelToCoordX(camera, pixel.x);
	result.y = PixelToCoordY(camera, pixel.y);
	return result;
}

void ClearScreen(Renderer renderer, Color color) {
	unsigned int colorCode = ColorCode(color);

	unsigned int *pixel = (unsigned int*)renderer.bitmap.memory;
	for (int row = 0; row < renderer.bitmap.height; ++row) {
		for (int col = 0; col < renderer.bitmap.width; ++col) {
			*pixel = colorCode;
			++pixel;
		}
	}
}

void Bresenham(Renderer renderer, Point point1, Point point2, Color color) {
	Bitmap bitmap = renderer.bitmap;
	unsigned int colorCode = ColorCode(color);

	Camera camera = *renderer.camera;
	Point pixelPoint1 = CoordToPixel(camera, point1);
	Point pixelPoint2 = CoordToPixel(camera, point2);

	int x1 = (int)pixelPoint1.x;
	int y1 = (int)pixelPoint1.y;
	int x2 = (int)pixelPoint2.x;
	int y2 = (int)pixelPoint2.y;

	int absX = IntAbs(x1 - x2);
	int absY = IntAbs(y1 - y2);

	int addX = 1;
	if (x1 > x2)
		addX = -1;

	int addY = 1;
	if (y1 > y2)
		addY = -1;

	int error = 0;
	if (absX > absY)
		error = absX / 2;
	else
		error = -absY / 2;

	int error2 = 0;

	while (1) {
		SetPixelCheck(renderer.bitmap, y1, x1, colorCode);

		if (x1 == x2 && y1 == y2)
			break;

		error2 = error;
		if (error2 > -absX) {
			error -= absY;
			x1 += addX;
		}
		if (error2 < absY) {
			error += absX;
			y1 += addY;
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

// TODO: change the order to left, right, top, bottom
// TODO: make this function take two points instead of four floats?
void DrawRect(Renderer renderer, float top, float left, float bottom, float right, Color color) {
	unsigned int colorCode = ColorCode(color);

	Camera camera = *renderer.camera;
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

	if (topPixel < 0)
		topPixel = 0;
	if (bottomPixel >= bitmap.height)
		bottomPixel = bitmap.height - 1;

	if (leftPixel < 0)
		leftPixel = 0;
	if (rightPixel >= bitmap.width)
		rightPixel = bitmap.width - 1;

	for (int row = topPixel; row < bottomPixel; ++row) {
		for (int col = leftPixel; col < rightPixel; ++col) {
			unsigned int *pixel = (unsigned int *)bitmap.memory + row * bitmap.width + col;
			pixel[0] = colorCode;
		}
	}
}

void DrawQuad(Renderer renderer, Point points[4], Color color) {
	unsigned int colorCode = ColorCode(color);

	for (int i = 0; i < 4; ++i)
		points[i] = CoordToPixel(*renderer.camera, points[i]);

	Bitmap bitmap = renderer.bitmap;
	int minX = bitmap.width;
	int minY = bitmap.height;
	int maxX = 0;
	int maxY = 0;

	for (int i = 0; i < 4; ++i) {
		int pointX = (int)points[i].x;
		int pointY = (int)points[i].y;

		if (pointX < minX)
			minX = pointX;
		if (pointX > maxX)
			maxX = pointX;
		if (pointY < minY)
			minY = pointY;
		if (pointY > maxY)
			maxY = pointY;
	}

	if (minX < 0)
		minX = 0;
	if (maxX >= bitmap.width)
		maxX = bitmap.width - 1;
	if (minY < 0)
		minY = 0;
	if (maxY >= bitmap.height)
		maxY = bitmap.height - 1;

	unsigned int *pixel = 0;
	for (int row = minY; row < maxY; ++row) {
		for (int col = minX; col < maxX; ++col) {
			Point testPoint = { (float)col, (float)row };

			bool drawPoint = true;

			// TODO: is using cross product faster than these calls?
			if (!TurnsRight(points[0], points[1], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[1], points[2], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[2], points[3], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[3], points[0], testPoint))
				drawPoint = false;

			if (drawPoint) {
				pixel = (unsigned int *)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}
}

void DrawQuadPoints(Renderer renderer, Point point1, Point point2, Point point3, Point point4, Color color) {
	Point points[4] = {point1, point2, point3, point4};
	DrawQuad(renderer, points, color);
}