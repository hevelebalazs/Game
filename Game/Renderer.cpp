#include "Geometry.h"
#include "Math.h"
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

FillHelper FillHelperForBitmap(Bitmap bitmap) {
	FillHelper result = {};
	result.count = 0;
	result.rows = new int[bitmap.width * bitmap.height];
	result.cols = new int[bitmap.width * bitmap.height];

	return result;
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

static inline void AddPosition(FillHelper* fillHelper, int row, int col) {
	fillHelper->rows[fillHelper->count] = row;
	fillHelper->cols[fillHelper->count] = col;
	fillHelper->count++;
}

// TODO: use a temporary memory arena so helpers are no longer needed!
void FloodFill(Renderer renderer, Point start, Color color, FillHelper fillHelper) {
	unsigned int colorCode = ColorCode(color);

	Bitmap bitmap = renderer.bitmap;
	Camera camera = *renderer.camera;

	fillHelper.count = 0;
	
	int row = (int)CoordYtoPixel(camera, start.y);
	int col = (int)CoordXtoPixel(camera, start.x);
	AddPosition(&fillHelper, row, col);

	bool fillHorizontally = true;
	int directionSwitchPosition = 1;

	AddPosition(&fillHelper, row, col);

	SetPixel(bitmap, fillHelper.rows[0], fillHelper.cols[0], colorCode);

	for (int i = 0; i < fillHelper.count; ++i) {
		if (i == directionSwitchPosition) {
			fillHorizontally = (!fillHorizontally);
			directionSwitchPosition = fillHelper.count + 1;
		}

		int row = fillHelper.rows[i];
		int col = fillHelper.cols[i];

		unsigned int* pixelStart = GetPixelAddress(bitmap, row, col);
		unsigned int* pixel = 0;

		if (fillHorizontally) {
			pixel = pixelStart;
			for (int left = col - 1; left >= 0; --left) {
				pixel--;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				AddPosition(&fillHelper, row, left);
			}

			pixel = pixelStart;
			for (int right = col + 1; right < bitmap.width; ++right) {
				pixel++;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				AddPosition(&fillHelper, row, right);
			}
		}
		else {
			pixel = pixelStart;
			for (int top = row - 1; top >= 0; --top) {
				pixel -= bitmap.width;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				AddPosition(&fillHelper, top, col);
			}

			pixel = pixelStart;
			for (int bottom = row + 1; bottom < bitmap.height; ++bottom) {
				pixel += bitmap.width;
			
				if (*pixel == colorCode) break;
				*pixel = colorCode;

				AddPosition(&fillHelper, bottom, col);
			}
		}
	}
}

void SmoothZoom(Camera* camera, float pixelCoordRatio) {
	camera->zoomTargetRatio = pixelCoordRatio;
}

void UpdateCamera(Camera* camera, float seconds) {
	if (camera->pixelCoordRatio == camera->zoomTargetRatio) {
		return;
	}
	else if (camera->pixelCoordRatio < camera->zoomTargetRatio) {
		camera->pixelCoordRatio += seconds * camera->zoomSpeed;
		if (camera->pixelCoordRatio > camera->zoomTargetRatio)
			camera->pixelCoordRatio = camera->zoomTargetRatio;
	}
	else {
		camera->pixelCoordRatio -= seconds * camera->zoomSpeed;
		if (camera->pixelCoordRatio < camera->zoomTargetRatio)
			camera->pixelCoordRatio = camera->zoomTargetRatio;
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
	int x1 = (int)CoordXtoPixel(camera, point1.x);
	int y1 = (int)CoordYtoPixel(camera, point1.y);
	int x2 = (int)CoordXtoPixel(camera, point2.x);
	int y2 = (int)CoordYtoPixel(camera, point2.y);

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
				pixel[0] = colorCode;
			}
		}
	}
}