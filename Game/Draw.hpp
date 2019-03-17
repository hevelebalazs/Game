#pragma once

#include "Bitmap.hpp"
#include "Debug.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Text.hpp"
#include "Texture.hpp"
#include "Type.hpp"

struct Camera
{
	Vec2 center;
	Vec2 screenPixelSize;

	Real32 unitInPixels;
	Real32 targetUnitInPixels;
};

struct Canvas 
{
	Bitmap bitmap;
	Camera* camera;
	GlyphData* glyphData;
};

static Int32 func UnitXtoPixel(Camera* camera, Real32 unitX)
{
	Int32 pixelX = Floor((camera->screenPixelSize.x * 0.5f) + ((unitX - camera->center.x) * camera->unitInPixels));
	return pixelX;
}

static Int32 func UnitYtoPixel(Camera* camera, Real32 unitY)
{
	Int32 pixelY = Floor((camera->screenPixelSize.y * 0.5f) + ((unitY - camera->center.y) * camera->unitInPixels));
	return pixelY;
}

static Vec2 func UnitToPixel(Camera* camera, Vec2 unit)
{
	Real32 x = (Real32)UnitXtoPixel(camera, unit.x);
	Real32 y = (Real32)UnitYtoPixel(camera, unit.y);
	Vec2 result = MakePoint(x, y);
	return result;
}

static void func ResizeCamera(Camera* camera, Int32 width, Int32 height)
{
	camera->screenPixelSize.x = (Real32)width;
	camera->screenPixelSize.y = (Real32)height;
	camera->center = (0.5f * camera->screenPixelSize);
}

static UInt32* func GetPixelAddress(Bitmap bitmap, Int32 row, Int32 col)
{
	UInt32* address = bitmap.memory + row * bitmap.width + col;
	return address;
}

static UInt32 func GetPixel(Bitmap bitmap, Int32 row, Int32 col)
{
	UInt32* pixelAddress = GetPixelAddress(bitmap, row, col);
	return *pixelAddress;
}

static void func SetPixel(Bitmap bitmap, Int32 row, Int32 col, UInt32 colorCode)
{
	UInt32* pixelAddress = GetPixelAddress(bitmap, row, col);
	*pixelAddress = colorCode;
}

static void func SetPixelCheck(Bitmap bitmap, Int32 row, Int32 col, Int32 colorCode)
{
	if((row >= 0 && row < bitmap.height) && (col >= 0 && col < bitmap.width))
	{
		SetPixel(bitmap, row, col, colorCode);
	}
}

static Vec4 func ColorProd(Vec4 color1, Vec4 color2)
{
	Vec4 result = {};
	result.red   = color1.red   * color2.red;
	result.green = color1.green * color2.green;
	result.blue  = color1.blue  * color2.blue;
	return result;
}

static void func ApplyBitmapMask(Bitmap bitmap, Bitmap mask)
{
	UInt32* pixel = bitmap.memory;
	UInt32* maskPixel = mask.memory;

	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			Vec4 color1 = GetColorFromColorCode(*pixel);
			Vec4 color2 = GetColorFromColorCode(*maskPixel);
			Vec4 color = ColorProd(color1, color2);
			UInt32 code = GetColorCode(color);
			*pixel = code;

			pixel++;
			maskPixel++;
		}
	}
}

struct PixelPosition
{
	Int32 row;
	Int32 col;
};

static void func FloodFill(Canvas* canvas, Vec2 start, Vec4 color, MemArena* tmpArena) 
{
	UInt32 colorCode = GetColorCode(color);

	Bitmap bitmap = canvas->bitmap;
	Camera* camera = canvas->camera;

	Int32 positionCount = 0;
	PixelPosition* positions = ArenaPushArray(tmpArena, PixelPosition, 0);
	
	Int32 row = UnitYtoPixel(camera, start.y);
	Int32 col = UnitXtoPixel(camera, start.x);

	ArenaPush(tmpArena, Int32, row);
	ArenaPush(tmpArena, Int32, col);
	positionCount++;

	Bool32 fillHorizontally = true;
	Int32 directionSwitchPosition = 1;

	ArenaPush(tmpArena, Int32, row);
	ArenaPush(tmpArena, Int32, col);
	positionCount++;

	SetPixel(bitmap, positions[0].row, positions[0].col, colorCode);

	for(Int32 i = 0; i < positionCount; i++) 
	{
		if(i == directionSwitchPosition) 
		{
			fillHorizontally = (!fillHorizontally);
			directionSwitchPosition = positionCount + 1;
		}

		Int32 row = positions[i].row;
		Int32 col = positions[i].col;

		UInt32* pixelStart = GetPixelAddress(bitmap, row, col);
		UInt32* pixel = 0;

		if(fillHorizontally) 
		{
			pixel = pixelStart;
			for(Int32 left = col - 1; left >= 0; left--) 
			{
				pixel--;

				if(*pixel == colorCode)
				{
					break;
				}
				*pixel = colorCode;

				ArenaPush(tmpArena, Int32, row);
				ArenaPush(tmpArena, Int32, left);
				positionCount++;
			}

			pixel = pixelStart;
			for(Int32 right = col + 1; right < bitmap.width; right++) 
			{
				pixel++;

				if(*pixel == colorCode) 
				{
					break;
				}
				*pixel = colorCode;

				ArenaPush(tmpArena, Int32, row);
				ArenaPush(tmpArena, Int32, right);
				positionCount++;
			}
		}
		else 
		{
			pixel = pixelStart;
			for(Int32 top = row - 1; top >= 0; top--) 
			{
				pixel -= bitmap.width;

				if(*pixel == colorCode) 
				{
					break;
				}
				*pixel = colorCode;

				ArenaPush(tmpArena, Int32, top);
				ArenaPush(tmpArena, Int32, col);
				positionCount++;
			}

			pixel = pixelStart;
			for(Int32 bottom = row + 1; bottom < bitmap.height; bottom++) 
			{
				pixel += bitmap.width;
			
				if(*pixel == colorCode)
				{
					break;
				}
				*pixel = colorCode;

				ArenaPush(tmpArena, Int32, bottom);
				ArenaPush(tmpArena, Int32, col);
				positionCount++;
			}
		}
	}

	ArenaPopTo(tmpArena, positions);
}

static void func SmoothZoom(Camera* camera, Real32 pixelPerUnit)
{
	camera->targetUnitInPixels = pixelPerUnit;
}

#define PixelPerUnitChangeSpeed 10.0f

static void func UpdateCamera(Camera* camera, Real32 seconds)
{
	if(camera->unitInPixels != camera->targetUnitInPixels) 
	{
		if(camera->unitInPixels < camera->targetUnitInPixels)
		{
			camera->unitInPixels += seconds * PixelPerUnitChangeSpeed;
			if(camera->unitInPixels > camera->targetUnitInPixels)
			{
				camera->unitInPixels = camera->targetUnitInPixels;
			}
		} 
		else 
		{
			camera->unitInPixels -= seconds * PixelPerUnitChangeSpeed;
			if(camera->unitInPixels < camera->targetUnitInPixels)
			{
				camera->unitInPixels = camera->targetUnitInPixels;
			}
		}
	}
}

static Real32 func GetUnitDistanceInPixel(Camera* camera, Real32 unitDistance)
{
	Real32 pixelDistance = unitDistance * camera->unitInPixels;
	return pixelDistance;
}

static Real32 func GetPixelDistanceInUnit(Camera* camera, Real32 pixelDistance)
{
	Assert(camera->unitInPixels > 0.0f);
	Real32 unitDistance = pixelDistance / camera->unitInPixels;
	return unitDistance;
}

static Real32 func PixelToUnitX(Camera* camera, Real32 pixelX)
{
	Real32 pixelInUnits = Invert(camera->unitInPixels);
	Real32 unitX = camera->center.x + (pixelX - camera->screenPixelSize.x * 0.5f) * pixelInUnits;
	return unitX;
}

static Real32 func PixelToUnitY(Camera* camera, Real32 pixelY) 
{
	Real32 pixelInUnits = Invert(camera->unitInPixels);
	Real32 unitY = camera->center.y + (pixelY - camera->screenPixelSize.y * 0.5f) * pixelInUnits;
	return unitY;
}

static Vec2 func PixelToUnit(Camera* camera, IntVec2 pixel)
{
	Vec2 result = {};
	result.x = PixelToUnitX(camera, (Real32)pixel.x);
	result.y = PixelToUnitY(camera, (Real32)pixel.y);
	return result;
}

static Vec2 func PixelToUnit(Camera* camera, Vec2 pixel)
{
	Vec2 result = {};
	result.x = PixelToUnitX(camera, pixel.x);
	result.y = PixelToUnitY(camera, pixel.y);
	return result;
}

static Real32 func CameraLeftSide(Camera* camera)
{
	Real32 leftSide = PixelToUnitX(camera, 0);
	return leftSide;
}

static Real32 func CameraRightSide(Camera* camera)
{
	Real32 rightSide = PixelToUnitX(camera, camera->screenPixelSize.x - 1);
	return rightSide;
}

static Real32 func CameraTopSide(Camera* camera)
{
	Real32 topSide = PixelToUnitY(camera, 0);
	return topSide;
}

static Real32 func CameraBottomSide(Camera* camera)
{
	Real32 bottomSide = PixelToUnitY(camera, camera->screenPixelSize.y - 1);
	return bottomSide;
}

static void func ClearScreen(Canvas* canvas, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);

	Bitmap bitmap = canvas->bitmap;
	UInt32 *pixel = bitmap.memory;
	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = colorCode;
			pixel++;
		}
	}
}

struct BresenhamContext 
{
	Int32 x1, y1;
	Int32 x2, y2;
	Int32 absX, absY;
	Int32 addX, addY;
	Int32 error, error2;
};

static BresenhamContext func BresenhamInitPixel(Vec2 pixelPoint1, Vec2 pixelPoint2)
{
	BresenhamContext context = {};

	context.x1 = (Int32)pixelPoint1.x;
	context.y1 = (Int32)pixelPoint1.y;
	context.x2 = (Int32)pixelPoint2.x;
	context.y2 = (Int32)pixelPoint2.y;

	context.absX = IntAbs(context.x1 - context.x2);
	context.absY = IntAbs(context.y1 - context.y2);

	context.addX = 1;
	if(context.x1 > context.x2)
	{
		context.addX = -1;
	}

	context.addY = 1;
	if(context.y1 > context.y2)
	{
		context.addY = -1;
	}

	context.error = 0;
	if(context.absX > context.absY)
	{
		context.error = context.absX / 2;
	}
	else
	{
		context.error = -context.absY / 2;
	}

	context.error2 = 0;

	return context;
}

static BresenhamContext func BresenhamInitUnit(Canvas* canvas, Vec2 point1, Vec2 point2)
{
	Camera* camera = canvas->camera;
	Vec2 pixelPoint1 = UnitToPixel(camera, point1);
	Vec2 pixelPoint2 = UnitToPixel(camera, point2);
	BresenhamContext context = BresenhamInitPixel(pixelPoint1, pixelPoint2);
	return context;
}

static void func BresenhamAdvance(BresenhamContext* context)
{
	context->error2 = context->error;
	if(context->error2 > -context->absX) 
	{
		context->error -= context->absY;
		context->x1 += context->addX;
	}
	if(context->error2 < context->absY) 
	{
		context->error += context->absX;
		context->y1 += context->addY;
	}
}

static void func Bresenham(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color)
{
	Bitmap bitmap = canvas->bitmap;
	UInt32 colorCode = GetColorCode(color);

	BresenhamContext context = BresenhamInitUnit(canvas, point1, point2);
	while(1) 
	{
		SetPixelCheck(bitmap, context.y1, context.x1, colorCode);

		if(context.x1 == context.x2 && context.y1 == context.y2)
		{
			break;
		}

		BresenhamAdvance(&context);
	}
}

static Vec2 func LineAtX(Vec2 linePoint1, Vec2 linePoint2, Real32 x)
{
	Real32 minX = Min2(linePoint1.x, linePoint2.x);
	Real32 maxX = Max2(linePoint1.x, linePoint2.x);
	Assert((minX <= x) && (x <= maxX));

	if(linePoint1.x == linePoint2.x)
	{
		return linePoint1;
	}

	Real32 alpha = (x - linePoint1.x) / (linePoint2.x - linePoint1.x);
	Vec2 result = {};
	result.x = x;
	result.y = linePoint1.y + alpha * (linePoint2.y - linePoint1.y);
	return result;
}

static Vec2 func LineAtY(Vec2 linePoint1, Vec2 linePoint2, Real32 y)
{
	Real32 minY = Min2(linePoint1.y, linePoint2.y);
	Real32 maxY = Max2(linePoint1.y, linePoint2.y);
	Assert((minY <= y) && (y <= maxY));
	
	if(linePoint1.y == linePoint2.y)
	{
		return linePoint1;
	}

	Real32 alpha = (y - linePoint1.y) / (linePoint2.y - linePoint1.y);
	Vec2 result = {};
	result.x = linePoint1.x + alpha * (linePoint2.x - linePoint1.x);
	result.y = y;
	return result;
}

static void func DrawHorizontalTrapezoid(Canvas* canvas, Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight, Vec4 color)
{
	Camera* camera = canvas->camera;
	Real32 cameraTop    = CameraTopSide(camera);
	Real32 cameraBottom = CameraBottomSide(camera);

	if(topLeft.y < cameraTop) 
	{
		if(bottomLeft.y < cameraTop)
		{
			return;
		}

		topLeft = LineAtY(topLeft, bottomLeft, cameraTop);
	}
	if(topRight.y < cameraTop) 
	{
		if(bottomRight.y < cameraTop)
		{
			return;
		}

		topRight = LineAtY(topRight, bottomRight, cameraTop);
	}
	if(bottomLeft.y > cameraBottom) 
	{
		if(topLeft.y > cameraBottom)
		{
			return;
		}

		bottomLeft = LineAtY(topLeft, bottomLeft, cameraBottom);
	}
	if(bottomRight.y > cameraBottom) 
	{
		if(topRight.y > cameraBottom)
		{
			return;
		}

		bottomRight = LineAtY(topRight, bottomRight, cameraBottom);
	}

	Bitmap bitmap = canvas->bitmap;
	UInt32 colorCode = GetColorCode(color);

	BresenhamContext leftLine = BresenhamInitUnit(canvas, topLeft, bottomLeft);
	BresenhamContext rightLine = BresenhamInitUnit(canvas, topRight, bottomRight);

	Int32 top = UnitYtoPixel(camera, topLeft.y);
	Int32 bottom = UnitYtoPixel(camera, bottomLeft.y);
	if(bottom < top)
	{
		return;
	}

	for(Int32 row = top; row <= bottom; row++) 
	{
		while(leftLine.y1 < row)
		{
			BresenhamAdvance(&leftLine);
		}

		while(rightLine.y1 < row)
		{
			BresenhamAdvance(&rightLine);
		}

		if((row < 0) || (row > bitmap.height - 1))
		{
			continue;
		}

		Int32 left = leftLine.x1;
		Int32 right = rightLine.x1;

		if(left < 0) 
		{
			left = 0;
		}
		if(right > bitmap.width - 1)
		{
			right = bitmap.width - 1;
		}

		UInt32* pixel = GetPixelAddress(bitmap, row, left);
		for(Int32 col = left; col <= right; col++) 
		{
			*pixel = colorCode;
			pixel++;
		}
	}
}

static void func DrawVerticalTrapezoid(Canvas* canvas, Vec2 topLeft, Vec2 topRight, Vec2 bottomLeft, Vec2 bottomRight, Vec4 color)
{
	Camera* camera = canvas->camera;
	Real32 cameraLeft  = CameraLeftSide(camera);
	Real32 cameraRight = CameraRightSide(camera);

	if(topLeft.x < cameraLeft) 
	{
		if(topRight.x < cameraLeft)
		{
			return;
		}

		topLeft = LineAtX(topLeft, topRight, cameraLeft);
	}
	if(topRight.x > cameraRight) 
	{
		if(topLeft.x > cameraRight)
		{
			return;
		}

		topRight = LineAtX(topLeft, topRight, cameraRight);
	}
	if(bottomLeft.x < cameraLeft) 
	{
		if(bottomRight.x < cameraLeft)
		{
			return;
		}

		bottomLeft = LineAtX(bottomLeft, bottomRight, cameraLeft);
	}
	if(bottomRight.x > cameraRight) 
	{
		if(bottomLeft.x > cameraRight)
		{
			return;
		}

		bottomRight = LineAtX(bottomLeft, bottomRight, cameraRight);
	}

	Bitmap bitmap = canvas->bitmap;
	UInt32 colorCode = GetColorCode(color);

	BresenhamContext topLine = BresenhamInitUnit(canvas, topLeft, topRight);
	BresenhamContext bottomLine = BresenhamInitUnit(canvas, bottomLeft, bottomRight);

	Int32 left = UnitXtoPixel(camera, topLeft.x);
	Int32 right = UnitXtoPixel(camera, topRight.x);
	if(right < left)
	{
		return;
	}

	for(Int32 col = left; col <= right; col++) 
	{
		while(topLine.x1 < col)
		{
			BresenhamAdvance(&topLine);
		}

		while(bottomLine.x1 < col)
		{
			BresenhamAdvance(&bottomLine);
		}

		if((col < 0) || (col > bitmap.width - 1))
		{
			continue;
		}

		Int32 top = topLine.y1;
		Int32 bottom = bottomLine.y1;

		if(top < 0) 
		{
			top = 0;
		}

		if(bottom > bitmap.height - 1)
		{
			bottom = bitmap.height - 1;
		}

		UInt32* pixel = GetPixelAddress(bitmap, top, col);
		for(Int32 row = top; row <= bottom; row++) 
		{
			*pixel = colorCode;
			pixel += bitmap.width;
		}
	}
}

static void func DrawRectLRTB(Canvas* canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);

	Camera *camera = canvas->camera;
	Int32 topPixel    = UnitYtoPixel(camera, top);
	Int32 leftPixel   = UnitXtoPixel(camera, left);
	Int32 bottomPixel = UnitYtoPixel(camera, bottom);
	Int32 rightPixel  = UnitXtoPixel(camera, right);

	if(topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if(leftPixel > rightPixel)
	{
		IntSwap(&leftPixel, &rightPixel);
	}

	Bitmap bitmap = canvas->bitmap;
	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	for(Int32 row = topPixel; row < bottomPixel; row++) 
	{
		for(Int32 col = leftPixel; col < rightPixel; col++) 
		{
			UInt32* pixel = bitmap.memory + row * bitmap.width + col;
			*pixel = colorCode;
		}
	}
}

static void func DrawRect(Canvas* canvas, Rect rect, Vec4 color)
{
	DrawRectLRTB(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void func DrawGridLine(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color, Real32 lineWidth)
{
	Real32 left   = 0.0f;
	Real32 right  = 0.0f;
	Real32 top    = 0.0f;
	Real32 bottom = 0.0f;

	if(point1.x == point2.x) 
	{
		left  = point1.x - lineWidth * 0.5f;
		right = point1.x + lineWidth * 0.5f;

		top    = Min2(point1.y, point2.y);
		bottom = Max2(point1.y, point2.y);
	} 
	else if(point1.y == point2.y)
	{
		top    = point1.y - lineWidth * 0.5f;
		bottom = point1.y + lineWidth * 0.5f;

		left  = Min2(point1.x, point2.x);
		right = Max2(point1.x, point2.x);
	}

	DrawRectLRTB(canvas, left, right, top, bottom, color);
}

static void func DrawQuad(Canvas* canvas, Quad quad, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);

	Vec2* points = quad.points;
	Camera* camera = canvas->camera;
	for(Int32 i = 0; i < 4; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	Int32 minX = bitmap.width;
	Int32 minY = bitmap.height;
	Int32 maxX = 0;
	Int32 maxY = 0;

	for(Int32 i = 0; i < 4; i++) 
	{
		Int32 pointX = (Int32)points[i].x;
		Int32 pointY = (Int32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	UInt32* pixel = 0;
	for(Int32 row = minY; row < maxY; ++row) 
	{
		for(Int32 col = minX; col < maxX; ++col) 
		{
			Vec2 testPoint = MakePoint(Real32(col), Real32(row));

			Bool32 drawPoint = true;

			if(!TurnsRight(points[0], points[1], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[1], points[2], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[2], points[3], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[3], points[0], testPoint))
			{
				drawPoint = false;
			}

			if(drawPoint) 
			{
				pixel = (UInt32*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}
}

static void func DrawLine(Canvas* canvas, Vec2 point1, Vec2 point2, Vec4 color, Real32 lineWidth)
{
	Vec2 direction = PointDirection(point2, point1);

	Real32 tmp = direction.x;
	direction.x = -direction.y;
	direction.y = tmp;

	Real32 halfLineWidth = lineWidth * 0.5f;
	Quad quad = {};
	quad.points[0] = point1 - (halfLineWidth * direction);
	quad.points[1] = point1 + (halfLineWidth * direction);
	quad.points[2] = point2 + (halfLineWidth * direction);
	quad.points[3] = point2 - (halfLineWidth * direction);
	DrawQuad(canvas, quad, color);
}

#define WorldTextureScale 20.0f

static void func FillScreenWithWorldTexture(Canvas* canvas, Texture texture)
{
	Bitmap bitmap = canvas->bitmap;
	Camera* camera = canvas->camera;

	Real32 pixelInUnits = Invert(camera->unitInPixels);

	Real32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	Real32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= WorldTextureScale;
	startY *= WorldTextureScale;

	Real32 addX = pixelInUnits;
	Real32 addY = pixelInUnits;
	addX *= WorldTextureScale;
	addY *= WorldTextureScale;
	Int32 andVal = (texture.side - 1);

	UInt32* pixel = bitmap.memory;

	Int32 startIntX = (Int32(startX)) & andVal;
	Int32 startIntY = (Int32(startY)) & andVal;

	UInt32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);

	Real32 y = startY;
	Int32 prevIntY = (Int32(y)) & andVal;

	for(Int32 row = 0; row < bitmap.height; row++) 
	{
		Real32 x = startX;
		Int32 prevIntX = (((Int32)x) & andVal);
		UInt32* texturePixel = textureRow;

		for(Int32 col = 0; col < bitmap.width; col++) 
		{
			*pixel = *texturePixel;
			pixel++;

			x += addX;
			Int32 intX = ((Int32)x) & andVal;
			texturePixel += (intX - prevIntX);
			prevIntX = intX;
		}

		y += addY;
		Int32 intY = ((Int32)y) & andVal;
		textureRow += (intY - prevIntY) << texture.logSide;
		prevIntY = intY;
	}
}

static void func DrawWorldTextureQuad(Canvas* canvas, Quad quad, Texture texture)
{
    Bitmap bitmap = canvas->bitmap;
	Camera* camera = canvas->camera;
    
    Vec2* points = quad.points;
    for(Int32 i = 0; i < 4; i++)
	{
        points[i] = UnitToPixel(camera, points[i]);
	}

	Int32 minX = bitmap.width;
	Int32 minY = bitmap.height;
	Int32 maxX = 0;
	Int32 maxY = 0;
   
	for(Int32 i = 0; i < 4; i++) 
	{
		Int32 pointX = (Int32)points[i].x;
        Int32 pointY = (Int32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);
    
	Real32 pixelInUnits = Invert(camera->unitInPixels);

	Real32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	Real32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= WorldTextureScale;
	startY *= WorldTextureScale;

	Real32 addX = pixelInUnits;
	Real32 addY = pixelInUnits;
	addX *= WorldTextureScale;
	addY *= WorldTextureScale;
    
    startX += (minX * addX);
    startY += (minY * addY);
    
    Int32 andVal = (texture.side - 1);

	Int32 startIntX = ((Int32)startX) & andVal;
	Int32 startIntY = ((Int32)startY) & andVal;

	UInt32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);
	UInt32* pixelRow = bitmap.memory + (bitmap.width * minY) + (minX);

	Real32 y = startY;
	Int32 prevIntY = (Int32(y)) & andVal;

	for(Int32 row = minY; row < maxY; row++) 
	{
        Real32 x = startX;
		Int32 prevIntX = ((Int32)x & andVal);
		UInt32* texturePixel = textureRow;
		UInt32* pixel = pixelRow;
    
		for(Int32 col = minX; col < maxX; col++) 
		{
			Vec2 testPoint = {};
            testPoint.x = (Real32)col;
            testPoint.y = (Real32)row;

			Bool32 drawPoint = true;

			if(!TurnsRight(points[0], points[1], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[1], points[2], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[2], points[3], testPoint))
			{
				drawPoint = false;
			}
			else if(!TurnsRight(points[3], points[0], testPoint))
			{
				drawPoint = false;
			}

			if(drawPoint)
			{
                *pixel = *texturePixel;
			}
            
            pixel++;
            
            x += addX;
			Int32 intX = ((Int32)x) & andVal;
			texturePixel += (intX - prevIntX);
			prevIntX = intX;
		}
        
        y += addY;
		Int32 intY = ((Int32)y) & andVal;
		textureRow += (intY - prevIntY) << texture.logSide;
		pixelRow += (bitmap.width);
		prevIntY = intY;
	}
}

static void func DrawWorldTextureLine(Canvas* canvas, Vec2 point1, Vec2 point2, Real32 lineWidth, Texture texture)
{
	Vec2 direction = PointDirection(point2, point1);
	Vec2 turnedDirection = TurnVectorToRight(direction);
	
	Real32 halfLineWidth = lineWidth * 0.5f;
	Quad quad = {};
	Vec2 pointProduct = (halfLineWidth * turnedDirection);
	quad.points[0] = (point1 - pointProduct);
	quad.points[1] = (point1 + pointProduct);
	quad.points[2] = (point2 + pointProduct);
	quad.points[3] = (point2 - pointProduct);
	DrawWorldTextureQuad(canvas, quad, texture);
}

static void func DrawRectLRTBOutline(Canvas* canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Vec4 color)
{
	Vec2 topLeft     = MakePoint(left,  top);
	Vec2 topRight    = MakePoint(right, top);
	Vec2 bottomLeft  = MakePoint(left,  bottom);
	Vec2 bottomRight = MakePoint(right, bottom);

	Bresenham(canvas, topLeft,     topRight,    color);
	Bresenham(canvas, topRight,    bottomRight, color);
	Bresenham(canvas, bottomRight, bottomLeft,  color);
	Bresenham(canvas, bottomLeft,  topLeft,     color);
}

static void func DrawRectOutline(Canvas* canvas, Rect rect, Vec4 color)
{
	DrawRectLRTBOutline(canvas, rect.left, rect.right, rect.top, rect.bottom, color);
}

static void func WorldTextureRect(Canvas* canvas, Real32 left, Real32 right, Real32 top, Real32 bottom, Texture texture)
{
	Camera* camera = canvas->camera;
	Int32 topPixel =    UnitYtoPixel(camera, top);
	Int32 leftPixel =   UnitXtoPixel(camera, left);
	Int32 bottomPixel = UnitYtoPixel(camera, bottom);
	Int32 rightPixel =  UnitXtoPixel(camera, right);

	if(topPixel > bottomPixel)
	{
		IntSwap(&topPixel, &bottomPixel);
	}

	if(leftPixel > rightPixel)
	{
		IntSwap(&leftPixel, &rightPixel);
	}

	Bitmap bitmap = canvas->bitmap;

	topPixel    = IntMax2(topPixel, 0);
	bottomPixel = IntMin2(bottomPixel, bitmap.height - 1);
	leftPixel   = IntMax2(leftPixel, 0);
	rightPixel  = IntMin2(rightPixel, bitmap.width - 1);

	Real32 textureZoom = 16.0f;

	Vec2 topLeftPixel = MakePoint((Real32)leftPixel, (Real32)topPixel);
	Vec2 topLeftUnit = PixelToUnit(camera, topLeftPixel);
	// NOTE: optimization stuff
	topLeftUnit.x *= textureZoom;
	topLeftUnit.y *= textureZoom;

	Real32 unitPerPixel = Invert(camera->unitInPixels);
	// NOTE: optimization stuff
	unitPerPixel *= textureZoom;

	Int32 subUnitPerPixel = (Int32)(unitPerPixel * 255.0f);

	UInt32* topLeft = bitmap.memory + topPixel * bitmap.width + leftPixel;

	// TODO: try to optimize this code
	Vec2 worldUnit = topLeftUnit;
	Int32 leftx  = ((Int32)worldUnit.x) & (texture.side - 1);
	Int32 topy   = ((Int32)worldUnit.y) & (texture.side - 1);

	Int32 leftsubx = (Int32)((worldUnit.x - Floor(worldUnit.x)) * 255.0f);
	Int32 topsuby = (Int32)((worldUnit.y - Floor(worldUnit.y)) * 255.0f);

	Int32 x = leftx;
	Int32 y = topy;

	Int32 subx = leftsubx;
	Int32 suby = topsuby;

	for(Int32 row = topPixel; row < bottomPixel; row++) 
	{
		x = leftx;
		subx = leftsubx;

		UInt32* pixel = topLeft;

		for(Int32 col = leftPixel; col < rightPixel; col++) 
		{
			*pixel = TextureColorCodeInt(texture, y, x);
			//*pixel = TextureColorCode(texture, x, subx, y, suby);
			pixel++;

			subx += subUnitPerPixel;
			if(subx > 0xFF) 
			{
				x = ((x + (subx >> 8)) & (texture.side - 1));
				subx = (subx & 0xFF);
			}
		}

		suby += subUnitPerPixel;
		if(suby > 0xFF) 
		{
			y = ((y + (suby >> 8)) & (texture.side - 1));
			suby = (suby & 0xFF);
		}

		topLeft += bitmap.width;
	}
}

static void func WorldTextureGridLine(Canvas* canvas, Vec2 point1, Vec2 point2, Real32 width, Texture texture)
{
	Real32 left   = Min2(point1.x, point2.x);
	Real32 right  = Max2(point1.x, point2.x);
	Real32 top    = Min2(point1.y, point2.y);
	Real32 bottom = Max2(point1.y, point2.y);

	// TODO: this Assert was triggered, check what's happening
	Assert((left == right) || (top == bottom));
	if(left == right) 
	{
		left  -= width * 0.5f;
		right += width * 0.5f;
	} 
	else if(top == bottom) 
	{
		top    -= width * 0.5f;
		bottom += width * 0.5f;
	}

	WorldTextureRect(canvas, left, right, top, bottom, texture);
}

static void func DrawPolyOutline(Canvas* canvas, Vec2* points, Int32 pointN, Vec4 color)
{
	Int32 prev = pointN - 1;
	for(Int32 i = 0; i < pointN; i++) 
	{
		Bresenham(canvas, points[prev], points[i], color);
		prev = i;
	}
}

static void func DrawPoly(Canvas* canvas, Vec2* points, Int32 pointN, Vec4 color)
{
	UInt32 colorCode = GetColorCode(color);

	Camera* camera = canvas->camera;
	for(Int32 i = 0; i < pointN; i++)
	{
		points[i] = UnitToPixel(camera, points[i]);
	}

	Bitmap bitmap = canvas->bitmap;
	Int32 minX = bitmap.width;
	Int32 minY = bitmap.height;
	Int32 maxX = 0;
	Int32 maxY = 0;

	for(Int32 i = 0; i < pointN; i++) 
	{
		Int32 pointX = (Int32)points[i].x;
		Int32 pointY = (Int32)points[i].y;

		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	UInt32 *pixel = 0;
	for(Int32 row = minY; row < maxY; row++) 
	{
		for(Int32 col = minX; col < maxX; col++) 
		{
			Vec2 testPoint = MakePoint(Real32(col), Real32(row));

			Bool32 drawPoint = true;

			Int32 prev = pointN - 1;
			for(Int32 i = 0; i < pointN; i++) 
			{
				// TODO: is using cross product faster than these calls?
				if(!TurnsRight(points[prev], points[i], testPoint)) 
				{
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if(drawPoint) 
			{
				pixel = (UInt32*)bitmap.memory + row * bitmap.width + col;
				*pixel = colorCode;
			}
		}
	}

	for(Int32 i = 0; i < pointN; ++i)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void func DrawWorldTexturePoly(Canvas* canvas, Vec2* points, Int32 pointN, Texture texture)
{
	Bitmap bitmap = canvas->bitmap;
	Camera* camera = canvas->camera;

	Int32 minX = bitmap.width;
	Int32 minY = bitmap.height;
	Int32 maxX = 0;
	Int32 maxY = 0;

	for(Int32 i = 0; i < pointN; i++) 
	{
		Vec2 pointInPixels = UnitToPixel(camera, points[i]);
		Int32 pointX = Int32(pointInPixels.x);
		Int32 pointY = Int32(pointInPixels.y);
		minX = IntMin2(minX, pointX);
		maxX = IntMax2(maxX, pointX);
		minY = IntMin2(minY, pointY);
		maxY = IntMax2(maxY, pointY);
	}

	minX = IntMax2(minX, 0);
	maxX = IntMin2(maxX, bitmap.width - 1);
	minY = IntMax2(minY, 0);
	maxY = IntMin2(maxY, bitmap.height - 1);

	Real32 pixelInUnits = Invert(camera->unitInPixels);
    
    Real32 startX = camera->center.x - (camera->screenPixelSize.x * 0.5f * pixelInUnits);
	Real32 startY = camera->center.y - (camera->screenPixelSize.y * 0.5f * pixelInUnits);
	startX *= WorldTextureScale;
	startY *= WorldTextureScale;

	Real32 addX = pixelInUnits;
	Real32 addY = pixelInUnits;
	addX *= WorldTextureScale;
	addY *= WorldTextureScale;
    
    startX += (minX * addX);
    startY += (minY * addY);
    
    Int32 andVal = (texture.side - 1);

	Int32 startIntX = ((Int32)startX) & andVal;
	Int32 startIntY = ((Int32)startY) & andVal;

	UInt32* textureRow = texture.memory + (startIntY << texture.logSide) + (startIntX);
	UInt32* pixelRow = bitmap.memory + (bitmap.width * minY) + (minX);

	Real32 y = startY;
	Int32 prevIntY = (Int32(y)) & andVal;

	UInt32* pixel = 0;
	for(Int32 row = minY; row <= maxY; row++) 
	{
        Real32 x = startX;
		Int32 prevIntX = (Int32(x) & andVal);
		UInt32* texturePixel = textureRow;
		UInt32* pixel = pixelRow;
    
		for(Int32 col = minX; col <= maxX; col++) 
		{
            Vec2 testPoint = {};
            testPoint.x = Real32(col);
            testPoint.y = Real32(row);

			Bool32 drawPoint = true;

			Int32 prev = pointN - 1;
			for(Int32 i = 0; i < pointN; i++) 
			{
				Vec2 prevPoint = UnitToPixel(camera, points[prev]);
				Vec2 thisPoint = UnitToPixel(camera, points[i]);
				if(!TurnsRight(prevPoint, thisPoint, testPoint)) 
				{
					drawPoint = false;
					break;
				}
				prev = i;
			}

			if(drawPoint)
			{
                *pixel = *texturePixel;
			}
                
            pixel++;
            x += addX;
            Int32 intX = ((Int32)x) & andVal;
            texturePixel += (intX - prevIntX);
            prevIntX = intX;
		}
        
        y += addY;
        Int32 intY = ((Int32)y) & andVal;
        textureRow += (intY - prevIntY) << texture.logSide;
        pixelRow += (bitmap.width);
        prevIntY = intY;
	}

	for(Int32 i = 0; i < pointN; i++)
	{
		points[i] = PixelToUnit(camera, points[i]);
	}
}

static void func DrawQuadPoints(Canvas* canvas, Vec2 point1, Vec2 point2, Vec2 point3, Vec2 point4, Vec4 color)
{
	Quad quad = MakeQuad(point1, point2, point3, point4);
	DrawQuad(canvas, quad, color);
}

static void func DrawBitmap(Canvas* canvas, Bitmap* bitmap, Real32 left, Real32 top)
{
	Camera* camera = canvas->camera;
	Int32 pixelLeft = UnitXtoPixel(camera, left);
	Int32 pixelTop  = UnitYtoPixel(camera, top);
	CopyBitmap(bitmap, &canvas->bitmap, pixelLeft, pixelTop);
}

static void func DrawScaledRotatedBitmap(Canvas* canvas, Bitmap* bitmap, Vec2 position, Real32 width, Real32 height, Real32 rotationAngle)
{
	Camera* camera = canvas->camera;
	Int32 col = UnitXtoPixel(camera, position.x);
	Int32 row = UnitYtoPixel(camera, position.y);

	Real32 pixelWidth  = GetUnitDistanceInPixel(camera, width);
	Real32 pixelHeight = GetUnitDistanceInPixel(camera, height);

	CopyScaledRotatedBitmap(bitmap, &canvas->bitmap, row, col, pixelWidth, pixelHeight, rotationAngle);
}

static void func DrawStretchedBitmap(Canvas* canvas, Bitmap* bitmap, Real32 left, Real32 right, Real32 top, Real32 bottom)
{
	Camera* camera = canvas->camera;
	Int32 pixelLeft   = UnitXtoPixel(camera, left);
	Int32 pixelRight  = UnitXtoPixel(camera, right);
	Int32 pixelTop    = UnitYtoPixel(camera, top);
	Int32 pixelBottom = UnitYtoPixel(camera, bottom);
	CopyStretchedBitmap(bitmap, &canvas->bitmap, pixelLeft, pixelRight, pixelTop, pixelBottom);
}

static Real32 func GetTextHeight(Canvas* canvas, Int8* text)
{
	Camera* camera = canvas->camera;
	Assert(camera->unitInPixels > 0.0f);
	Real32 height = (Real32)TextHeightInPixels / camera->unitInPixels;
	return height;
}

static Real32 func GetTextWidth(Canvas* canvas, Int8* text)
{
	Assert(canvas->glyphData != 0);
	Real32 pixelWidth = GetTextPixelWidth(text, canvas->glyphData);

	Camera* camera = canvas->camera;
	Assert(camera->unitInPixels > 0.0f);
	Real32 width = pixelWidth / camera->unitInPixels;
	return width;
}

static void func DrawTextLine(Canvas* canvas, Int8* text, Real32 baseLineY, Real32 left, Vec4 textColor)
{
	Assert(canvas->glyphData != 0);
	Int32 leftPixel = UnitXtoPixel(canvas->camera, left);
	Int32 baseLineYPixel = UnitYtoPixel(canvas->camera, baseLineY);
	DrawBitmapTextLine(&canvas->bitmap, text, canvas->glyphData, leftPixel, baseLineYPixel, textColor);
}

static void func DrawTextLineXCentered(Canvas* canvas, Int8* text, Real32 baseLineY, Real32 centerX, Vec4 textColor)
{
	Assert(canvas->glyphData != 0);
	Real32 left = centerX - GetTextWidth(canvas, text) * 0.5f;
	DrawTextLine(canvas, text, baseLineY, left, textColor);
}

static void func DrawTextLineXYCentered(Canvas* canvas, Int8* text, Real32 centerY, Real32 centerX, Vec4 textColor)
{
	Assert(canvas->glyphData != 0);
	Camera* camera = canvas->camera;

	Real32 left = centerX - GetTextWidth(canvas, text) * 0.5f;
	Real32 textHeight = GetPixelDistanceInUnit(camera, TextHeightInPixels);
	Real32 textTop = centerY - textHeight * 0.5f;
	Real32 baseLineY = textTop + GetPixelDistanceInUnit(camera, TextPixelsAboveBaseLine);
	DrawTextLine(canvas, text, baseLineY, left, textColor);
}