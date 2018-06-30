#include "Debug.hpp"
#include "Draw.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

void ResizeCamera(Camera* camera, I32 width, I32 height)
{
	camera->screenPixelSize.x = (F32)width;
	camera->screenPixelSize.y = (F32)height;
}

U32* GetPixelAddress(Bitmap bitmap, I32 row, I32 col)
{
	U32* address = bitmap.memory + row * bitmap.width + col;
	return address;
}

static U32 GetPixel(Bitmap bitmap, I32 row, I32 col)
{
	U32* pixelAddress = GetPixelAddress(bitmap, row, col);
	return *pixelAddress;
}

static void SetPixel(Bitmap bitmap, I32 row, I32 col, U32 colorCode)
{
	U32* pixelAddress = GetPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

static void SetPixelCheck(Bitmap bitmap, I32 row, I32 col, I32 colorCode)
{
	if ((row >= 0 && row < bitmap.height) && (col >= 0 && col < bitmap.width))
		SetPixel(bitmap, row, col, colorCode);
}

static V4 ColorProd(V4 color1, V4 color2)
{
	V4 result = {};
	result.red   = color1.red   * color2.red;
	result.green = color1.green * color2.green;
	result.blue  = color1.blue  * color2.blue;
	return result;
}

void ApplyBitmapMask(Bitmap bitmap, Bitmap mask)
{
	U32* pixel = bitmap.memory;
	U32* maskPixel = mask.memory;

	for (I32 row = 0; row < bitmap.height; ++row) {
		for (I32 col = 0; col < bitmap.width; ++col) {
			V4 color1 = GetColorFromColorCode(*pixel);
			V4 color2 = GetColorFromColorCode(*maskPixel);
			V4 color = ColorProd(color1, color2);
			U32 code = GetColorCode(color);
			*pixel = code;

			pixel++;
			maskPixel++;
		}
	}
}

struct PixelPosition {
	I32 row;
	I32 col;
};

void FloodFill(Canvas canvas, V2 start, V4 color, MemArena* tmpArena) {
	U32 colorCode = GetColorCode(color);

	Bitmap bitmap = canvas.bitmap;
	Camera* camera = canvas.camera;

	I32 positionCount = 0;
	PixelPosition* positions = ArenaPushArray(tmpArena, PixelPosition, 0);
	
	I32 row = UnitYtoPixel(camera, start.y);
	I32 col = UnitXtoPixel(camera, start.x);

	ArenaPush(tmpArena, I32, row);
	ArenaPush(tmpArena, I32, col);
	positionCount++;

	B32 fillHorizontally = true;
	I32 directionSwitchPosition = 1;

	ArenaPush(tmpArena, I32, row);
	ArenaPush(tmpArena, I32, col);
	positionCount++;

	SetPixel(bitmap, positions[0].row, positions[0].col, colorCode);

	for (I32 i = 0; i < positionCount; ++i) {
		if (i == directionSwitchPosition) {
			fillHorizontally = (!fillHorizontally);
			directionSwitchPosition = positionCount + 1;
		}

		I32 row = positions[i].row;
		I32 col = positions[i].col;

		U32* pixelStart = GetPixelAddress(bitmap, row, col);
		U32* pixel = 0;

		if (fillHorizontally) {
			pixel = pixelStart;
			for (I32 left = col - 1; left >= 0; --left) {
				pixel--;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, I32, row);
				ArenaPush(tmpArena, I32, left);
				positionCount++;
			}

			pixel = pixelStart;
			for (I32 right = col + 1; right < bitmap.width; ++right) {
				pixel++;

				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, I32, row);
				ArenaPush(tmpArena, I32, right);
				positionCount++;
			}
		} else {
			pixel = pixelStart;
			for (I32 top = row - 1; top >= 0; --top) {
				pixel -= bitmap.width;

				if (*pixel == colorCode) 
					break;
				*pixel = colorCode;

				ArenaPush(tmpArena, I32, top);
				ArenaPush(tmpArena, I32, col);
				positionCount++;
			}

			pixel = pixelStart;
			for (I32 bottom = row + 1; bottom < bitmap.height; ++bottom) {
				pixel += bitmap.width;
			
				if (*pixel == colorCode) break;
				*pixel = colorCode;

				ArenaPush(tmpArena, I32, bottom);
				ArenaPush(tmpArena, I32, col);
				positionCount++;
			}
		}
	}

	ArenaPopTo(tmpArena, positions);
}

void SmoothZoom(Camera* camera, F32 pixelPerUnit)
{
	camera->targetUnitInPixels = pixelPerUnit;
}

#define PixelPerUnitChangeSpeed 10.0f

void UpdateCamera(Camera* camera, F32 seconds)
{
	if (camera->unitInPixels != camera->targetUnitInPixels) {
		if (camera->unitInPixels < camera->targetUnitInPixels) {
			camera->unitInPixels += seconds * PixelPerUnitChangeSpeed;
			if (camera->unitInPixels > camera->targetUnitInPixels)
				camera->unitInPixels = camera->targetUnitInPixels;
		} else {
			camera->unitInPixels -= seconds * PixelPerUnitChangeSpeed;
			if (camera->unitInPixels < camera->targetUnitInPixels)
				camera->unitInPixels = camera->targetUnitInPixels;
		}
	}
}

F32 GetUnitDistanceInPixel(Camera* camera, F32 unitDistance)
{
	F32 pixelDistance = unitDistance * camera->unitInPixels;
	return pixelDistance;
}

I32 UnitXtoPixel(Camera* camera, F32 unitX)
{
	I32 pixelX = Floor((camera->screenPixelSize.x * 0.5f) + ((unitX - camera->center.x) * camera->unitInPixels));
	return pixelX;
}

I32 UnitYtoPixel(Camera* camera, F32 unitY)
{
	I32 pixelY = Floor((camera->screenPixelSize.y * 0.5f) + ((unitY - camera->center.y) * camera->unitInPixels));
	return pixelY;
}

V2 UnitToPixel(Camera* camera, V2 unit)
{
	F32 x = (F32)UnitXtoPixel(camera, unit.x);
	F32 y = (F32)UnitYtoPixel(camera, unit.y);
	V2 result = MakePoint(x, y);
	return result;
}

F32 PixelToUnitX(Camera* camera, F32 pixelX)
{
	F32 pixelInUnits = Invert(camera->unitInPixels);
	F32 unitX = camera->center.x + (pixelX - camera->screenPixelSize.x * 0.5f) * pixelInUnits;
	return unitX;
}

F32 PixelToUnitY(Camera* camera, F32 pixelY) 
{
	F32 pixelInUnits = Invert(camera->unitInPixels);
	F32 unitY = camera->center.y + (pixelY - camera->screenPixelSize.y * 0.5f) * pixelInUnits;
	return unitY;
}

V2 PixelToUnit(Camera* camera, V2 pixel)
{
	V2 result = {};
	result.x = PixelToUnitX(camera, pixel.x);
	result.y = PixelToUnitY(camera, pixel.y);
	return result;
}

F32 CameraLeftSide(Camera* camera)
{
	F32 leftSide = PixelToUnitX(camera, 0);
	return leftSide;
}

F32 CameraRightSide(Camera* camera)
{
	F32 rightSide = PixelToUnitX(camera, camera->screenPixelSize.x - 1);
	return rightSide;
}

F32 CameraTopSide(Camera* camera)
{
	F32 topSide = PixelToUnitY(camera, 0);
	return topSide;
}

F32 CameraBottomSide(Camera* camera)
{
	F32 bottomSide = PixelToUnitY(camera, camera->screenPixelSize.y - 1);
	return bottomSide;
}

void ClearScreen(Canvas canvas, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Bitmap bitmap = canvas.bitmap;
	U32 *pixel = bitmap.memory;
	for (I32 row = 0; row < bitmap.height; ++row) {
		for (I32 col = 0; col < bitmap.width; ++col) {
			*pixel = colorCode;
			++pixel;
		}
	}
}

struct BresenhamContext {
	I32 x1, y1;
	// TODO: is this needed?
	I32 x2, y2;
	I32 absX, absY;
	I32 addX, addY;
	I32 error, error2;
};

static BresenhamContext BresenhamInitPixel(V2 pixelPoint1, V2 pixelPoint2)
{
	BresenhamContext context = {};

	context.x1 = (I32)pixelPoint1.x;
	context.y1 = (I32)pixelPoint1.y;
	context.x2 = (I32)pixelPoint2.x;
	context.y2 = (I32)pixelPoint2.y;

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

static BresenhamContext BresenhamInitUnit(Canvas canvas, V2 point1, V2 point2)
{
	Camera* camera = canvas.camera;
	V2 pixelPoint1 = UnitToPixel(camera, point1);
	V2 pixelPoint2 = UnitToPixel(camera, point2);
	BresenhamContext context = BresenhamInitPixel(pixelPoint1, pixelPoint2);
	return context;
}

static void BresenhamAdvance(BresenhamContext* context)
{
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

void Bresenham(Canvas canvas, V2 point1, V2 point2, V4 color)
{
	Bitmap bitmap = canvas.bitmap;
	U32 colorCode = GetColorCode(color);

	BresenhamContext context = BresenhamInitUnit(canvas, point1, point2);
	while (1) {
		SetPixelCheck(bitmap, context.y1, context.x1, colorCode);

		if (context.x1 == context.x2 && context.y1 == context.y2)
			break;

		BresenhamAdvance(&context);
	}
}

// TODO: move this to Geometry?
static V2 LineAtX(V2 linePoint1, V2 linePoint2, F32 x)
{
	F32 minX = Min2(linePoint1.x, linePoint2.x);
	F32 maxX = Max2(linePoint1.x, linePoint2.x);
	Assert ((minX <= x) && (x <= maxX));

	if (linePoint1.x == linePoint2.x)
		return linePoint1;

	F32 alpha = (x - linePoint1.x) / (linePoint2.x - linePoint1.x);
	V2 result = {};
	result.x = x;
	result.y = linePoint1.y + alpha * (linePoint2.y - linePoint1.y);
	return result;
}

// TODO: move this to Geometry?
static V2 LineAtY(V2 linePoint1, V2 linePoint2, F32 y)
{
	F32 minY = Min2(linePoint1.y, linePoint2.y);
	F32 maxY = Max2(linePoint1.y, linePoint2.y);
	Assert ((minY <= y) && (y <= maxY));
	
	if (linePoint1.y == linePoint2.y)
		return linePoint1;

	F32 alpha = (y - linePoint1.y) / (linePoint2.y - linePoint1.y);
	V2 result = {};
	result.x = linePoint1.x + alpha * (linePoint2.x - linePoint1.x);
	result.y = y;
	return result;
}

// TODO: check this for performance issues
void DrawHorizontalTrapezoid(Canvas canvas, V2 topLeft, V2 topRight, V2 bottomLeft, V2 bottomRight, V4 color)
{
	Camera* camera = canvas.camera;
	F32 cameraTop     = CameraTopSide(camera);
	F32 cameraBottom  = CameraBottomSide(camera);

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

	Bitmap bitmap = canvas.bitmap;
	U32 colorCode = GetColorCode(color);

	BresenhamContext leftLine = BresenhamInitUnit(canvas, topLeft, bottomLeft);
	BresenhamContext rightLine = BresenhamInitUnit(canvas, topRight, bottomRight);

	I32 top = UnitYtoPixel(camera, topLeft.y);
	I32 bottom = UnitYtoPixel(camera, bottomLeft.y);
	if (bottom < top)
		return;

	for (I32 row = top; row <= bottom; ++row) {
		while (leftLine.y1 < row)
			BresenhamAdvance(&leftLine);
		while (rightLine.y1 < row)
			BresenhamAdvance(&rightLine);

		if ((row < 0) || (row > bitmap.height - 1))
			continue;

		I32 left = leftLine.x1;
		I32 right = rightLine.x1;

		if (left < 0) 
			left = 0;
		if (right > bitmap.width - 1)
			right = bitmap.width - 1;

		U32* pixel = GetPixelAddress(bitmap, row, left);
		for (I32 col = left; col <= right; ++col) {
			*pixel = colorCode;
			pixel++;
		}
	}
}

// TODO: check this for performance issues
void DrawVerticalTrapezoid(Canvas canvas, V2 topLeft, V2 topRight, V2 bottomLeft, V2 bottomRight, V4 color)
{
	Camera* camera = canvas.camera;
	F32 cameraLeft  = CameraLeftSide(camera);
	F32 cameraRight = CameraRightSide(camera);

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

	Bitmap bitmap = canvas.bitmap;
	U32 colorCode = GetColorCode(color);

	BresenhamContext topLine = BresenhamInitUnit(canvas, topLeft, topRight);
	BresenhamContext bottomLine = BresenhamInitUnit(canvas, bottomLeft, bottomRight);

	I32 left = UnitXtoPixel(camera, topLeft.x);
	I32 right = UnitXtoPixel(camera, topRight.x);
	if (right < left)
		return;

	for (I32 col = left; col <= right; ++col) {
		while (topLine.x1 < col)
			BresenhamAdvance(&topLine);
		while (bottomLine.x1 < col)
			BresenhamAdvance(&bottomLine);

		if ((col < 0) || (col > bitmap.width - 1))
			continue;

		I32 top = topLine.y1;
		I32 bottom = bottomLine.y1;

		if (top < 0) 
			top = 0;
		if (bottom > bitmap.height - 1)
			bottom = bitmap.height - 1;

		U32* pixel = GetPixelAddress(bitmap, top, col);
		for (I32 row = top; row <= bottom; ++row) {
			*pixel = colorCode;
			pixel += bitmap.width;
		}
	}
}

void DrawGridLine(Canvas canvas, V2 point1, V2 point2, V4 color, F32 lineWidth)
{
	F32 left   = 0.0f;
	F32 right  = 0.0f;
	F32 top    = 0.0f;
	F32 bottom = 0.0f;

	if (point1.x == point2.x) {
		left  = point1.x - lineWidth * 0.5f;
		right = point1.x + lineWidth * 0.5f;

		top    = Min2(point1.y, point2.y);
		bottom = Max2(point1.y, point2.y);
	} else if (point1.y == point2.y) {
		top    = point1.y - lineWidth * 0.5f;
		bottom = point1.y + lineWidth * 0.5f;

		left  = Min2(point1.x, point2.x);
		right = Max2(point1.x, point2.x);
	}

	DrawRect(canvas, left, right, top, bottom, color);
}

void DrawLine(Canvas canvas, V2 point1, V2 point2, V4 color, F32 lineWidth)
{
	V2 direction = PointDirection(point2, point1);

	F32 tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	F32 halfLineWidth = lineWidth * 0.5f;
	Quad quad = {};
	quad.points[0] = point1 - (halfLineWidth * direction);
	quad.points[1] = point1 + (halfLineWidth * direction);
	quad.points[2] = point2 + (halfLineWidth * direction);
	quad.points[3] = point2 - (halfLineWidth * direction);
	DrawQuad(canvas, quad, color);
}

void FillScreenWithWorldTexture(Canvas canvas, Texture texture)
{
	Bitmap bitmap = canvas.bitmap;
	Camera* camera = canvas.camera;

	F32 scale = 10.0f;
	F32 pixelInUnits = Invert(camera->unitInPixels);

	F32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	F32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= scale;
	startY *= scale;

	F32 addX = pixelInUnits;
	F32 addY = pixelInUnits;
	addX *= scale;
	addY *= scale;
	I32 andVal = (texture.side - 1);

	U32* pixel = bitmap.memory;

	I32 startIntX = ((I32)startX) & andVal;
	I32 startIntY = ((I32)startY) & andVal;

	U32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);

	F32 y = startY;
	I32 prevIntY = ((I32)y) & andVal;

	for (I32 row = 0; row < bitmap.height; ++row) {
		F32 x = startX;
		I32 prevIntX = ((I32)x & andVal);
		U32* texturePixel = textureRow;

		for (I32 col = 0; col < bitmap.width; ++col) {
			*pixel = *texturePixel;
			pixel++;

			x += addX;
			I32 intX = ((I32)x) & andVal;
			texturePixel += (intX - prevIntX);
			prevIntX = intX;
		}

		y += addY;
		I32 intY = ((I32)y) & andVal;
		textureRow += (intY - prevIntY) << texture.logSide;
		prevIntY = intY;
	}
}

void DrawWorldTextureLine(Canvas canvas, V2 point1, V2 point2, F32 lineWidth, Texture texture)
{
	V2 direction = PointDirection(point2, point1);
	V2 turnedDirection = TurnVectorToRight(direction);
	
	F32 halfLineWidth = lineWidth * 0.5f;
	Quad quad = {};
	V2 pointProduct = (halfLineWidth * turnedDirection);
	quad.points[0] = (point1 - pointProduct);
	quad.points[1] = (point1 + pointProduct);
	quad.points[2] = (point2 + pointProduct);
	quad.points[3] = (point2 - pointProduct);
	DrawWorldTextureQuad(canvas, quad, texture);
}

void DrawRect(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera *camera = canvas.camera;
	I32 topPixel =    UnitYtoPixel(camera, top);
	I32 leftPixel =   UnitXtoPixel(camera, left);
	I32 bottomPixel = UnitYtoPixel(camera, bottom);
	I32 rightPixel =  UnitXtoPixel(camera, right);

	if (topPixel > bottomPixel)
		IntSwap(&topPixel, &bottomPixel);

	if (leftPixel > rightPixel)
		IntSwap(&leftPixel, &rightPixel);

	Bitmap bitmap = canvas.bitmap;
	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	for (I32 row = topPixel; row < bottomPixel; ++row) {
		for (I32 col = leftPixel; col < rightPixel; ++col) {
			U32* pixel = bitmap.memory + row * bitmap.width + col;
			*pixel = colorCode;
		}
	}
}

void DrawRectOutline(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, V4 color)
{
	V2 topLeft     = MakePoint(left,  top);
	V2 topRight    = MakePoint(right, top);
	V2 bottomLeft  = MakePoint(left,  bottom);
	V2 bottomRight = MakePoint(right, bottom);

	Bresenham(canvas, topLeft,     topRight,    color);
	Bresenham(canvas, topRight,    bottomRight, color);
	Bresenham(canvas, bottomRight, bottomLeft,  color);
	Bresenham(canvas, bottomLeft,  topLeft,     color);
}

// TODO: change the order of parameters to left, right, top, bottom
void WorldTextureRect(Canvas canvas, F32 left, F32 right, F32 top, F32 bottom, Texture texture)
{
	Camera* camera = canvas.camera;
	I32 topPixel =    UnitYtoPixel(camera, top);
	I32 leftPixel =   UnitXtoPixel(camera, left);
	I32 bottomPixel = UnitYtoPixel(camera, bottom);
	I32 rightPixel =  UnitXtoPixel(camera, right);

	if (topPixel > bottomPixel)
		IntSwap(&topPixel, &bottomPixel);

	if (leftPixel > rightPixel)
		IntSwap(&leftPixel, &rightPixel);

	Bitmap bitmap = canvas.bitmap;

	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	F32 textureZoom = 16.0f;

	V2 topLeftPixel = {(F32)leftPixel, (F32)topPixel};
	V2 topLeftUnit = PixelToUnit(camera, topLeftPixel);
	// NOTE: optimization stuff
	topLeftUnit.x *= textureZoom;
	topLeftUnit.y *= textureZoom;

	F32 unitPerPixel = Invert(camera->unitInPixels);
	// NOTE: optimization stuff
	unitPerPixel *= textureZoom;

	I32 subUnitPerPixel = (I32)(unitPerPixel * 255.0f);

	U32* topLeft = bitmap.memory + topPixel * bitmap.width + leftPixel;

	// TODO: try to optimize this code
	V2 worldUnit = topLeftUnit;
	I32 leftx  = (((I32)(worldUnit.x)) & (texture.side - 1));
	I32 topy   = (((I32)(worldUnit.y)) & (texture.side - 1));

	I32 leftsubx = (I32)((worldUnit.x - Floor(worldUnit.x)) * 255.0f);
	I32 topsuby = (I32)((worldUnit.y - Floor(worldUnit.y)) * 255.0f);

	I32 x = leftx;
	I32 y = topy;

	I32 subx = leftsubx;
	I32 suby = topsuby;

	for (I32 row = topPixel; row < bottomPixel; ++row) {
		x = leftx;
		subx = leftsubx;

		U32* pixel = topLeft;

		for (I32 col = leftPixel; col < rightPixel; ++col) {
			*pixel = TextureColorCodeInt(texture, y, x);
			//*pixel = TextureColorCode(texture, x, subx, y, suby);
			pixel++;

			subx += subUnitPerPixel;
			if (subx > 0xFF) {
				x = ((x + (subx >> 8)) & (texture.side - 1));
				subx = (subx & 0xFF);
			}
		}

		suby += subUnitPerPixel;
		if (suby > 0xFF) {
			y = ((y + (suby >> 8)) & (texture.side - 1));
			suby = (suby & 0xFF);
		}

		topLeft += bitmap.width;
	}
}

void WorldTextureGridLine(Canvas canvas, V2 point1, V2 point2, F32 width, Texture texture)
{
	F32 left   = Min2(point1.x, point2.x);
	F32 right  = Max2(point1.x, point2.x);
	F32 top    = Min2(point1.y, point2.y);
	F32 bottom = Max2(point1.y, point2.y);

	// TODO: this Assert was triggered, check what's happening
	Assert((left == right) || (top == bottom));
	if (left == right) {
		left  -= width * 0.5f;
		right += width * 0.5f;
	} else if (top == bottom) {
		top    -= width * 0.5f;
		bottom += width * 0.5f;
	}

	WorldTextureRect(canvas, left, right, top, bottom, texture);
}

void DrawPolyOutline(Canvas canvas, V2* points, I32 pointN, V4 color)
{
	I32 prev = pointN - 1;
	for (I32 i = 0; i < pointN; ++i) {
		Bresenham(canvas, points[prev], points[i], color);
		prev = i;
	}
}

void DrawPoly(Canvas canvas, V2* points, I32 pointN, V4 color)
{
	U32 colorCode = GetColorCode(color);

	Camera* camera = canvas.camera;
	for (I32 i = 0; i < pointN; ++i)
		points[i] = UnitToPixel(camera, points[i]);

	Bitmap bitmap = canvas.bitmap;
	I32 minX = bitmap.width;
	I32 minY = bitmap.height;
	I32 maxX = 0;
	I32 maxY = 0;

	for (I32 i = 0; i < pointN; ++i) {
		I32 pointX = (I32)points[i].x;
		I32 pointY = (I32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	U32 *pixel = 0;
	for (I32 row = minY; row < maxY; ++row) {
		for (I32 col = minX; col < maxX; ++col) {
			V2 testPoint = { (F32)col, (F32)row };

			B32 drawPoint = true;

			I32 prev = pointN - 1;
			for (I32 i = 0; i < pointN; ++i) {
				// TODO: is using cross product faster than these calls?
				if (!TurnsRight(points[prev], points[i], testPoint)) {
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if (drawPoint) {
				pixel = (U32*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}

	for (I32 i = 0; i < pointN; ++i)
		points[i] = PixelToUnit(camera, points[i]);
}

void DrawWorldTexturePoly(Canvas canvas, V2* points, I32 pointN, Texture texture)
{
	Bitmap bitmap = canvas.bitmap;
	Camera* camera = canvas.camera;

	I32 minX = bitmap.width;
	I32 minY = bitmap.height;
	I32 maxX = 0;
	I32 maxY = 0;

	for (I32 i = 0; i < pointN; ++i) {
		V2 pointInPixels = UnitToPixel(camera, points[i]);
		I32 pointX = (I32)pointInPixels.x;
		I32 pointY = (I32)pointInPixels.y;
		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	F32 scale = 10.0f;
	F32 pixelInUnits = Invert(camera->unitInPixels);
    
    F32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	F32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= scale;
	startY *= scale;

	F32 addX = pixelInUnits;
	F32 addY = pixelInUnits;
	addX *= scale;
	addY *= scale;
    
    startX += (minX * addX);
    startY += (minY * addY);
    
    I32 andVal = (texture.side - 1);

	I32 startIntX = ((I32)startX) & andVal;
	I32 startIntY = ((I32)startY) & andVal;

	U32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);
	U32* pixelRow = bitmap.memory + (bitmap.width * minY) + (minX);

	F32 y = startY;
	I32 prevIntY = ((I32)y) & andVal;

	U32* pixel = 0;
	for (I32 row = minY; row <= maxY; ++row) {
        F32 x = startX;
		I32 prevIntX = ((I32)x & andVal);
		U32* texturePixel = textureRow;
		U32* pixel = pixelRow;
    
		for (I32 col = minX; col <= maxX; ++col) {
            V2 testPoint = {};
            testPoint.x = (F32)col;
            testPoint.y = (F32)row;

			B32 drawPoint = true;

			I32 prev = pointN - 1;
			for (I32 i = 0; i < pointN; ++i) {
				V2 prevPoint = UnitToPixel(camera, points[prev]);
				V2 thisPoint = UnitToPixel(camera, points[i]);
				if (!TurnsRight(prevPoint, thisPoint, testPoint)) {
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if (drawPoint)
                *pixel = *texturePixel;
                
            pixel++;
            x += addX;
            I32 intX = ((I32)x) & andVal;
            texturePixel += (intX - prevIntX);
            prevIntX = intX;
		}
        
        y += addY;
        I32 intY = ((I32)y) & andVal;
        textureRow += (intY - prevIntY) << texture.logSide;
        pixelRow += (bitmap.width);
        prevIntY = intY;
	}

	for (I32 i = 0; i < pointN; ++i)
		points[i] = PixelToUnit(camera, points[i]);
}

void DrawQuad(Canvas canvas, Quad quad, V4 color)
{
	U32 colorCode = GetColorCode(color);

	V2* points = quad.points;
	Camera* camera = canvas.camera;
	for (I32 i = 0; i < 4; ++i)
		points[i] = UnitToPixel(camera, points[i]);

	Bitmap bitmap = canvas.bitmap;
	I32 minX = bitmap.width;
	I32 minY = bitmap.height;
	I32 maxX = 0;
	I32 maxY = 0;

	for (I32 i = 0; i < 4; ++i) {
		I32 pointX = (I32)points[i].x;
		I32 pointY = (I32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	U32* pixel = 0;
	for (I32 row = minY; row < maxY; ++row) {
		for (I32 col = minX; col < maxX; ++col) {
			V2 testPoint = { (F32)col, (F32)row };

			B32 drawPoint = true;

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
				pixel = (U32*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}
}

void DrawWorldTextureQuad(Canvas canvas, Quad quad, Texture texture)
{
    Bitmap bitmap = canvas.bitmap;
	Camera* camera = canvas.camera;
    
    V2* points = quad.points;
    for (I32 i = 0; i < 4; ++i)
        points[i] = UnitToPixel(camera, points[i]);

	I32 minX = bitmap.width;
	I32 minY = bitmap.height;
	I32 maxX = 0;
	I32 maxY = 0;
   
	for (I32 i = 0; i < 4; ++i) {
		I32 pointX = (I32)points[i].x;
        I32 pointY = (I32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);
    
    F32 scale = 10.0f;
	F32 pixelInUnits = Invert(camera->unitInPixels);

	F32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	F32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= scale;
	startY *= scale;

	F32 addX = pixelInUnits;
	F32 addY = pixelInUnits;
	addX *= scale;
	addY *= scale;
    
    startX += (minX * addX);
    startY += (minY * addY);
    
    I32 andVal = (texture.side - 1);

	I32 startIntX = ((I32)startX) & andVal;
	I32 startIntY = ((I32)startY) & andVal;

	U32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);
	U32* pixelRow = bitmap.memory + (bitmap.width * minY) + (minX);

	F32 y = startY;
	I32 prevIntY = ((I32)y) & andVal;

	for (I32 row = minY; row < maxY; ++row) {
        F32 x = startX;
		I32 prevIntX = ((I32)x & andVal);
		U32* texturePixel = textureRow;
		U32* pixel = pixelRow;
    
		for (I32 col = minX; col < maxX; ++col) {
			V2 testPoint = {};
            testPoint.x = (F32)col;
            testPoint.y = (F32)row;

			B32 drawPoint = true;

			if (!TurnsRight(points[0], points[1], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[1], points[2], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[2], points[3], testPoint))
				drawPoint = false;
			else if (!TurnsRight(points[3], points[0], testPoint))
				drawPoint = false;

			if (drawPoint) {
                *pixel = *texturePixel;
			}
            
            pixel++;
            
            x += addX;
			I32 intX = ((I32)x) & andVal;
			texturePixel += (intX - prevIntX);
			prevIntX = intX;
		}
        
        y += addY;
		I32 intY = ((I32)y) & andVal;
		textureRow += (intY - prevIntY) << texture.logSide;
		pixelRow += (bitmap.width);
		prevIntY = intY;
	}
}

void DrawQuadPoints(Canvas canvas, V2 point1, V2 point2, V2 point3, V2 point4, V4 color)
{
	Quad quad = {point1, point2, point3, point4};
	DrawQuad(canvas, quad, color);
}

void DrawScaledRotatedBitmap(Canvas canvas, Bitmap* bitmap, V2 position, F32 width, F32 height, F32 rotationAngle)
{
	Camera* camera = canvas.camera;
	I32 col = UnitXtoPixel(camera, position.x);
	I32 row = UnitYtoPixel(camera, position.y);

	F32 pixelWidth  = GetUnitDistanceInPixel(camera, width);
	F32 pixelHeight = GetUnitDistanceInPixel(camera, height);

	CopyScaledRotatedBitmap(bitmap, &canvas.bitmap, row, col, pixelWidth, pixelHeight, rotationAngle);
}

void DrawStretchedBitmap(Canvas canvas, Bitmap* bitmap, F32 left, F32 right, F32 top, F32 bottom)
{
	Camera* camera = canvas.camera;
	I32 pixelLeft   = UnitXtoPixel(camera, left);
	I32 pixelRight  = UnitXtoPixel(camera, right);
	I32 pixelTop    = UnitYtoPixel(camera, top);
	I32 pixelBottom = UnitYtoPixel(camera, bottom);
	CopyStretchedBitmap(bitmap, &canvas.bitmap, pixelLeft, pixelRight, pixelTop, pixelBottom);
}