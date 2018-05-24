#include "Debug.h"
#include "Geometry.h"
#include "Math.h"
#include "Memory.h"
#include "Point.h"
#include "Renderer.h"

void ResizeCamera(Camera* camera, int width, int height)
{
	camera->screenSize = Point{(float)width, (float)height};
	camera->center = PointProd(0.5f, camera->screenSize);
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

static inline Color ColorProd(Color color1, Color color2) {
	Color result = {};
	result.red   = color1.red   * color2.red;
	result.green = color1.green * color2.green;
	result.blue  = color1.blue  * color2.blue;
	return result;
}

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask) {
	unsigned int* pixel = (unsigned int*)bitmap.memory;
	unsigned int* maskPixel = (unsigned int*)mask.memory;

	for (int row = 0; row < bitmap.height; ++row) {
		for (int col = 0; col < bitmap.width; ++col) {
			Color color1 = ColorFromCode(*pixel);
			Color color2 = ColorFromCode(*maskPixel);
			Color color = ColorProd(color1, color2);
			unsigned int code = ColorCode(color);
			*pixel = code;

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

struct BresenhamContext {
	int x1, y1;
	// TODO: is this needed?
	int x2, y2;
	int absX, absY;
	int addX, addY;
	int error, error2;
};

inline BresenhamContext BresenhamInitPixel(Point pixelPoint1, Point pixelPoint2) {
	BresenhamContext context = {};

	context.x1 = (int)pixelPoint1.x;
	context.y1 = (int)pixelPoint1.y;
	context.x2 = (int)pixelPoint2.x;
	context.y2 = (int)pixelPoint2.y;

	context.absX = IntAbs(context.x1 - context.x2);
	context.absY = IntAbs(context.y1 - context.y2);

	context.addX = 1;
	if (context.x1 > context.x2)
		context.addX = -1;

	context.addY = 1;
	if (context.y1 > context.y2)
		context.addY = -1;

	context.error = 0;
	if (context.absX > context.absY)
		context.error = context.absX / 2;
	else
		context.error = -context.absY / 2;

	context.error2 = 0;

	return context;
}

inline BresenhamContext BresenhamInitCoord(Renderer renderer, Point point1, Point point2) {
	Camera camera = *renderer.camera;
	Point pixelPoint1 = CoordToPixel(camera, point1);
	Point pixelPoint2 = CoordToPixel(camera, point2);
	BresenhamContext context = BresenhamInitPixel(pixelPoint1, pixelPoint2);
	return context;
}

inline void BresenhamAdvance(BresenhamContext* context) {
	context->error2 = context->error;
	if (context->error2 > -context->absX) {
		context->error -= context->absY;
		context->x1 += context->addX;
	}
	if (context->error2 < context->absY) {
		context->error += context->absX;
		context->y1 += context->addY;
	}
}

void Bresenham(Renderer renderer, Point point1, Point point2, Color color) {
	Bitmap bitmap = renderer.bitmap;
	unsigned int colorCode = ColorCode(color);

	BresenhamContext context = BresenhamInitCoord(renderer, point1, point2);
	while (1) {
		SetPixelCheck(bitmap, context.y1, context.x1, colorCode);

		if (context.x1 == context.x2 && context.y1 == context.y2)
			break;

		BresenhamAdvance(&context);
	}
}

// TODO: move this to Geometry?
inline Point LineAtX(Point linePoint1, Point linePoint2, float x) {
	float minX = Min2(linePoint1.x, linePoint2.x);
	float maxX = Max2(linePoint1.x, linePoint2.x);
	Assert ((minX <= x) && (x <= maxX));

	if (linePoint1.x == linePoint2.x)
		return linePoint1;

	float alpha = (x - linePoint1.x) / (linePoint2.x - linePoint1.x);
	Point result = {};
	result.x = x;
	result.y = linePoint1.y + alpha * (linePoint2.y - linePoint1.y);
	return result;
}

// TODO: move this to Geometry?
inline Point LineAtY(Point linePoint1, Point linePoint2, float y) {
	float minY = Min2(linePoint1.y, linePoint2.y);
	float maxY = Max2(linePoint1.y, linePoint2.y);
	Assert ((minY <= y) && (y <= maxY));
	
	if (linePoint1.y == linePoint2.y)
		return linePoint1;

	float alpha = (y - linePoint1.y) / (linePoint2.y - linePoint1.y);
	Point result = {};
	result.x = linePoint1.x + alpha * (linePoint2.x - linePoint1.x);
	result.y = y;
	return result;
}

// TODO: check this for performance issues
void DrawHorizontalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color) {
	Camera* camera = renderer.camera;
	float cameraTop     = CameraTopCoord(renderer.camera);
	float cameraBottom  = CameraBottomCoord(renderer.camera);

	if (topLeft.y < cameraTop) {
		if (bottomLeft.y < cameraTop)
			return;

		topLeft = LineAtY(topLeft, bottomLeft, cameraTop);
	}
	if (topRight.y < cameraTop) {
		if (bottomRight.y < cameraTop)
			return;

		topRight = LineAtY(topRight, bottomRight, cameraTop);
	}
	if (bottomLeft.y > cameraBottom) {
		if (topLeft.y > cameraBottom)
			return;

		bottomLeft = LineAtY(topLeft, bottomLeft, cameraBottom);
	}
	if (bottomRight.y > cameraBottom) {
		if (topRight.y > cameraBottom)
			return;

		bottomRight = LineAtY(topRight, bottomRight, cameraBottom);
	}

	Bitmap bitmap = renderer.bitmap;
	unsigned int colorCode = ColorCode(color);

	BresenhamContext leftLine = BresenhamInitCoord(renderer, topLeft, bottomLeft);
	BresenhamContext rightLine = BresenhamInitCoord(renderer, topRight, bottomRight);

	int top = (int)CoordYtoPixel(*renderer.camera, topLeft.y);
	int bottom = (int)CoordYtoPixel(*renderer.camera, bottomLeft.y);
	if (bottom < top)
		return;

	for (int row = top; row <= bottom; ++row) {
		while (leftLine.y1 < row)
			BresenhamAdvance(&leftLine);
		while (rightLine.y1 < row)
			BresenhamAdvance(&rightLine);

		if ((row < 0) || (row > bitmap.height - 1))
			continue;

		int left = leftLine.x1;
		int right = rightLine.x1;

		if (left < 0) 
			left = 0;
		if (right > bitmap.width - 1)
			right = bitmap.width - 1;

		unsigned int* pixel = GetPixelAddress(bitmap, row, left);
		for (int col = left; col <= right; ++col) {
			*pixel = colorCode;
			pixel++;
		}
	}
}

// TODO: check this for performance issues
void DrawVerticalTrapezoid(Renderer renderer, Point topLeft, Point topRight, Point bottomLeft, Point bottomRight, Color color) {
	Camera* camera = renderer.camera;
	float cameraLeft  = CameraLeftCoord(renderer.camera);
	float cameraRight = CameraRightCoord(renderer.camera);

	if (topLeft.x < cameraLeft) {
		if (topRight.x < cameraLeft)
			return;

		topLeft = LineAtX(topLeft, topRight, cameraLeft);
	}
	if (topRight.x > cameraRight) {
		if (topLeft.x > cameraRight)
			return;

		topRight = LineAtX(topLeft, topRight, cameraRight);
	}
	if (bottomLeft.x < cameraLeft) {
		if (bottomRight.x < cameraLeft)
			return;

		bottomLeft = LineAtX(bottomLeft, bottomRight, cameraLeft);
	}
	if (bottomRight.x > cameraRight) {
		if (bottomLeft.x > cameraRight)
			return;

		bottomRight = LineAtX(bottomLeft, bottomRight, cameraRight);
	}

	Bitmap bitmap = renderer.bitmap;
	unsigned int colorCode = ColorCode(color);

	BresenhamContext topLine = BresenhamInitCoord(renderer, topLeft, topRight);
	BresenhamContext bottomLine = BresenhamInitCoord(renderer, bottomLeft, bottomRight);

	int left = (int)CoordXtoPixel(*renderer.camera, topLeft.x);
	int right = (int)CoordXtoPixel(*renderer.camera, topRight.x);
	if (right < left)
		return;

	for (int col = left; col <= right; ++col) {
		while (topLine.x1 < col)
			BresenhamAdvance(&topLine);
		while (bottomLine.x1 < col)
			BresenhamAdvance(&bottomLine);

		if ((col < 0) || (col > bitmap.width - 1))
			continue;

		int top = topLine.y1;
		int bottom = bottomLine.y1;

		if (top < 0) 
			top = 0;
		if (bottom > bitmap.height - 1)
			bottom = bitmap.height - 1;

		unsigned int* pixel = GetPixelAddress(bitmap, top, col);
		for (int row = top; row <= bottom; ++row) {
			*pixel = colorCode;
			pixel += bitmap.width;
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
	Quad quad = {};

	quad.points[0] = PointDiff(point1, PointProd(halfLineWidth, direction));
	quad.points[1] = PointSum (point1, PointProd(halfLineWidth, direction));
	quad.points[2] = PointSum (point2, PointProd(halfLineWidth, direction));
	quad.points[3] = PointDiff(point2, PointProd(halfLineWidth, direction));
	DrawQuad(renderer, quad, color);
}

void DrawWorldTextureLine(Renderer renderer, Point point1, Point point2, float lineWidth, Texture texture)
{
	Point direction = PointDirection(point2, point1);
	Point turnedDirection = TurnVectorToRight(direction);
	
	float halfLineWidth = lineWidth * 0.5f;
	Quad quad = {};
	Point pointProduct = PointProd(halfLineWidth, turnedDirection);
	quad.points[0] = PointDiff(point1, pointProduct);
	quad.points[1] = PointSum (point1, pointProduct);
	quad.points[2] = PointSum (point2, pointProduct);
	quad.points[3] = PointDiff(point2, pointProduct);
	DrawWorldTextureQuad(renderer, quad, texture);
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
			unsigned int* pixel = (unsigned int*)bitmap.memory + row * bitmap.width + col;
			*pixel = colorCode;
		}
	}
}

void DrawPolyOutline(Renderer renderer, Point* points, int pointN, Color color) {
	int prev = pointN - 1;
	for (int i = 0; i < pointN; ++i) {
		Bresenham(renderer, points[prev], points[i], color);
		prev = i;
	}
}

void DrawPoly(Renderer renderer, Point* points, int pointN, Color color) {
	unsigned int colorCode = ColorCode(color);

	for (int i = 0; i < pointN; ++i)
		points[i] = CoordToPixel(*renderer.camera, points[i]);

	Bitmap bitmap = renderer.bitmap;
	int minX = bitmap.width;
	int minY = bitmap.height;
	int maxX = 0;
	int maxY = 0;

	for (int i = 0; i < pointN; ++i) {
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

			int prev = pointN - 1;
			for (int i = 0; i < pointN; ++i) {
				// TODO: is using cross product faster than these calls?
				if (!TurnsRight(points[prev], points[i], testPoint)) {
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if (drawPoint) {
				pixel = (unsigned int*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}

	for (int i = 0; i < pointN; ++i)
		points[i] = PixelToCoord(*renderer.camera, points[i]);
}

void DrawWorldTexturePoly(Renderer renderer, Point* points, int pointN, Texture texture)
{
	Bitmap bitmap = renderer.bitmap;
	Camera* camera = renderer.camera;
	int minX = bitmap.width;
	int minY = bitmap.height;
	int maxX = 0;
	int maxY = 0;

	for (int i = 0; i < pointN; ++i) {
		Point pointInPixels = CoordToPixel(*camera, points[i]);
		int pointX = (int)pointInPixels.x;
		int pointY = (int)pointInPixels.y;
		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	unsigned int *pixel = 0;
	for (int row = minY; row <= maxY; ++row) {
		for (int col = minX; col <= maxX; ++col) {
			Point testPoint = { (float)col, (float)row };

			bool drawPoint = true;

			int prev = pointN - 1;
			for (int i = 0; i < pointN; ++i) {
				Point prevPoint = CoordToPixel(*camera, points[prev]);
				Point thisPoint = CoordToPixel(*camera, points[i]);
				if (!TurnsRight(prevPoint, thisPoint, testPoint)) {
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if (drawPoint) {
				pixel = (unsigned int*)bitmap.memory + row * bitmap.width + col;
				Point testCoordPoint = PixelToCoord(*camera, testPoint);
				*pixel = TextureColorCode(texture, testCoordPoint.x * 10.0f, testCoordPoint.y * 10.0f);
			}
		}
	}

	for (int i = 0; i < pointN; ++i)
		points[i] = PixelToCoord(*renderer.camera, points[i]);
}

void DrawQuad(Renderer renderer, Quad quad, Color color) {
	unsigned int colorCode = ColorCode(color);

	for (int i = 0; i < 4; ++i)
		quad.points[i] = CoordToPixel(*renderer.camera, quad.points[i]);

	Bitmap bitmap = renderer.bitmap;
	int minX = bitmap.width;
	int minY = bitmap.height;
	int maxX = 0;
	int maxY = 0;

	for (int i = 0; i < 4; ++i) {
		int pointX = (int)quad.points[i].x;
		int pointY = (int)quad.points[i].y;

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
			if (!TurnsRight(quad.points[0], quad.points[1], testPoint))
				drawPoint = false;
			else if (!TurnsRight(quad.points[1], quad.points[2], testPoint))
				drawPoint = false;
			else if (!TurnsRight(quad.points[2], quad.points[3], testPoint))
				drawPoint = false;
			else if (!TurnsRight(quad.points[3], quad.points[0], testPoint))
				drawPoint = false;

			if (drawPoint) {
				pixel = (unsigned int*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}
}

void DrawWorldTextureQuad(Renderer renderer, Quad quad, Texture texture)
{
	Bitmap bitmap = renderer.bitmap;
	Camera camera = *renderer.camera;
	int minX = bitmap.width;
	int minY = bitmap.height;
	int maxX = 0;
	int maxY = 0;
	for (int i = 0; i < 4; ++i) {
		Point pixelPoint = CoordToPixel(camera, quad.points[i]);
		int pointX = (int)pixelPoint.x;
		int pointY = (int)pixelPoint.y;
		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}
	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	unsigned int *pixel = 0;
	for (int row = minY; row < maxY; ++row) {
		for (int col = minX; col < maxX; ++col) {
			Point testPoint = { (float)col, (float)row };

			bool drawPoint = true;

			Point pixelPoints[4] = {
				CoordToPixel(camera, quad.points[0]),
				CoordToPixel(camera, quad.points[1]),
				CoordToPixel(camera, quad.points[2]),
				CoordToPixel(camera, quad.points[3])
			};

			if (!TurnsRight(pixelPoints[0], pixelPoints[1], testPoint))
				drawPoint = false;
			else if (!TurnsRight(pixelPoints[1], pixelPoints[2], testPoint))
				drawPoint = false;
			else if (!TurnsRight(pixelPoints[2], pixelPoints[3], testPoint))
				drawPoint = false;
			else if (!TurnsRight(pixelPoints[3], pixelPoints[0], testPoint))
				drawPoint = false;

			if (drawPoint) {
				pixel = (unsigned int*)bitmap.memory + row * bitmap.width + col;
				Point testCoordPoint = PixelToCoord(camera, testPoint);
				*pixel = TextureColorCode(texture, testCoordPoint.x * 10.0f, testCoordPoint.y * 10.0f);
			}
		}
	}
}

void DrawQuadPoints(Renderer renderer, Point point1, Point point2, Point point3, Point point4, Color color) {
	Quad quad = {point1, point2, point3, point4};
	DrawQuad(renderer, quad, color);
}