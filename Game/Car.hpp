#pragma once

#include "Bezier.hpp"
#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Map.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Type.hpp"

#define MinCarSpeed	10.0f
#define MaxCarSpeed	16.0f
#define MaxPlayerCarSpeed 25.0f
#define MaxCarEngineForce 1000.0f
#define MaxCarBrakeForce 1000.0f

#define CarCornerMass 250000
#define CarMass (4 * CarCornerMass)

struct Car 
{
	V2 position;
	F32 angle;
	F32 length;
	F32 width;

	F32 moveSpeed;
	F32 maxSpeed;

	Map* map;
	Bitmap* bitmap;
};

struct AutoCar 
{
	Car car;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	V4 moveStartPoint;
	V4 moveEndPoint;
	Bezier4 moveBezier4;
	F32 bezierRatio;

	F32 acceleration;
};

struct PlayerCar 
{
	Car car;

	V2 velocity;
	V2 acceleration;

	F32 angularVelocity;
	F32 angularAcceleration;
	F32 inertia;

	F32 engineForce;
	F32 breakForce;

	F32 frontWheelAngle;
	F32 frontWheelAngleTarget;
};

struct CollisionInfo 
{
	I32 count;
	V2 point;
	V2 normalVector;
};

#define CarBitmapWidth 140
#define CarBitmapHeight 300

static Quad func GetCarCorners(Car* car)
{
	Quad result = {};

	V2 addWidth = ((car->width * 0.5f) * RotationVector(car->angle + PI * 0.5f));

	V2 side1 = (car->position + addWidth);
	V2 side2 = (car->position - addWidth);
	
	V2 addLength = ((car->length * 0.5f) * RotationVector(car->angle));
	
	result.points[0] = (side1 + addLength);
	result.points[1] = (side1 - addLength);
	result.points[2] = (side2 - addLength);
	result.points[3] = (side2 + addLength);
	return result;
}

static Quad func GetCarStopArea(Car* car)
{
	V2 toFrontUnitVector = RotationVector(car->angle);
	V2 toRightUnitVector = RotationVector(car->angle + PI * 0.5f);

	V2 addWidth = (car->width * 0.5f) * toRightUnitVector;

	V2 side1 = (car->position + addWidth);
	V2 side2 = (car->position - addWidth);

	Assert(car->maxSpeed > 0.0f);
	F32 minStopDistance = car->length * 0.5f;
	F32 maxStopDistance = minStopDistance + 5.0f;

	F32 stopDistance = Lerp(minStopDistance, car->moveSpeed / car->maxSpeed, maxStopDistance);

	V2 addClose = (car->length * 0.5f) * toFrontUnitVector;
	V2 addFar = (car->length * 0.5f + stopDistance) * toFrontUnitVector;

	Quad result = {};
	result.points[0] = (side1 + addClose);
	result.points[1] = (side2 + addClose);
	result.points[2] = (side2 + addFar);
	result.points[3] = (side1 + addFar);

	return result;
}

static B32 func IsCarOnPoint(Car* car, V2 point)
{
	Quad carCorners = GetCarCorners(car);
	B32 result = IsPointInQuad(carCorners, point);
	return result;
}

static void func MoveCar(Car* car, V4 point)
{
	car->position = point.position;
	car->angle = VectorAngle(point.direction);
}

static void func DrawCar(Canvas* canvas, Car car)
{
	Assert(car.bitmap != 0);
	DrawScaledRotatedBitmap(canvas, car.bitmap, car.position, car.width, car.length, car.angle);
}

static V4 func GetRandomCarColor()
{
	F32 colorSum = RandomBetween(1.0f, 1.5f);
	F32 red = RandomBetween(0.0f, colorSum);
	red = Clip(red, 0.0f, 1.0f);
	F32 green = RandomBetween(0.0f, colorSum - red);
	green = Clip(green, 0.0f, 1.0f);
	F32 blue = RandomBetween(0.0f, colorSum - red - green);
	blue = Clip(blue, 0.0f, 1.0f);
	V4 color = MakeColor(red, green, blue);
	return color;
}

static V4 func GetRandomWindowColor()
{
	F32 red   = RandomBetween(0.0f, 0.05f);
	F32 green = RandomBetween(0.0f, 0.05f);
	F32 blue  = RandomBetween(0.0f, 0.25f);
	V4 color = MakeColor(red, green, blue);
	return color;
}

static V4 func GetShadowColor(V4 color)
{
	F32 shadowRatio = 0.5f;
	F32 red   = shadowRatio * color.red;
	F32 green = shadowRatio * color.green;
	F32 blue  = shadowRatio * color.blue;
	V4 shadowColor = MakeColor(red, green, blue);
	return shadowColor;
}

static V4 func GetRandomFrontLampColor()
{
	F32 redGreenValue = RandomBetween(0.0f, 1.0f);
	F32 red   = redGreenValue;
	F32 green = redGreenValue;
	F32 blue  = RandomBetween(0.8f, 1.0f);
	V4 color = MakeColor(red, green, blue);
	return color;
}

static void func AllocateCarBitmap(Bitmap* carBitmap)
{
	ResizeBitmap(carBitmap, CarBitmapWidth, CarBitmapHeight);
}

static void func GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena)
{
	Assert(carBitmap != 0);

	V4 backgroundColor = MakeAlphaColor(0.2f, 0.2f, 0.2f, 0.0f);
	FillBitmapWithColor(carBitmap, backgroundColor);

	V4 carColor = GetRandomCarColor();
	V4 shadowColor = GetShadowColor(carColor);

	V4 borderColor = MakeColor(0.2f, 0.2f, 0.2f);

	I32 bitmapLeft   = 0;
	I32 bitmapRight  = carBitmap->width - 1;
	I32 bitmapTop    = 0;
	I32 bitmapBottom = carBitmap->height - 1;

	I32 frontHoodLength    = IntRandom(65, 70);
	I32 frontHoodWidthDiff = IntRandom(0, 5);

	I32 frontHoodTopLeft     = bitmapLeft + 20 + frontHoodWidthDiff;
	I32 frontHoodBottomLeft  = bitmapLeft + 20;
	I32 frontHoodTopRight    = bitmapRight - 20 - frontHoodWidthDiff;
	I32 frontHoodBottomRight = bitmapRight - 20;
	I32 frontHoodTop         = bitmapTop + 20;
	I32 frontHoodBottom      = frontHoodTop + frontHoodLength;

	I32 middlePartLength = IntRandom(135, 150);
	
	I32 backHoodLength    = IntRandom(20, 30);
	I32 backHoodWidthDiff = IntRandom(0, 5);

	I32 backHoodTopLeft     = bitmapLeft + 20;
	I32 backHoodBottomLeft  = bitmapLeft + 20 + backHoodWidthDiff;
	I32 backHoodTopRight    = bitmapRight - 20;
	I32 backHoodBottomRight = bitmapRight - 20 - backHoodWidthDiff;
	I32 backHoodTop         = frontHoodBottom + middlePartLength;
	I32 backHoodBottom      = backHoodTop + backHoodLength;

	I32 carPoly[] = 
	{
		frontHoodTop, frontHoodTopLeft,
		frontHoodTop, frontHoodTopRight,
		frontHoodBottom, frontHoodBottomRight,
		backHoodTop, backHoodTopRight,
		backHoodBottom, backHoodBottomRight,
		backHoodBottom, backHoodBottomLeft,
		backHoodTop, backHoodTopLeft,
		frontHoodBottom, frontHoodBottomLeft
	};

	DrawBitmapPolyOutline(carBitmap, 8, carPoly, carColor);
	I32 carCenterRow = (frontHoodTop + backHoodBottom) / 2;
	I32 carCenterCol = (frontHoodBottomLeft + frontHoodBottomRight) / 2;
	FloodfillBitmap(carBitmap, carCenterRow, carCenterCol, carColor, tmpArena);

	I32 frontWindowLength    = IntRandom(40, 60);
	I32 windowSeparatorWidth = IntRandom(3, 5);
	I32 frontWindowWidthDiff = IntRandom(5, 15);

	V4 windowColor = GetRandomWindowColor();
	I32 frontWindowTop         = frontHoodBottom;
	I32 frontWindowBottom      = frontWindowTop + frontWindowLength;
	I32 frontWindowTopLeft     = frontHoodBottomLeft + windowSeparatorWidth;
	I32 frontWindowTopRight    = frontHoodBottomRight - windowSeparatorWidth;
	I32 frontWindowBottomLeft  = frontWindowTopLeft + frontWindowWidthDiff;
	I32 frontWindowBottomRight = frontWindowTopRight - frontWindowWidthDiff;
	I32 frontWindowPoly[] = 
	{
		frontWindowTop,    frontWindowTopLeft,
		frontWindowTop,    frontWindowTopRight,
		frontWindowBottom, frontWindowBottomRight,
		frontWindowBottom, frontWindowBottomLeft
	};

	DrawBitmapPolyOutline(carBitmap, 4, frontWindowPoly, borderColor);
	I32 frontWindowCenterRow = (frontWindowTop + frontWindowBottom) / 2;
	I32 frontWindowCenterCol = (frontWindowTopLeft + frontWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontWindowCenterRow, frontWindowCenterCol, windowColor, tmpArena);

	I32 backWindowLength = IntRandom(15, 25);
	I32 backWindowWidthDiff = IntRandom(5, 15);

	I32 backWindowBottom      = backHoodTop;
	I32 backWindowTop         = backWindowBottom - backWindowLength;
	I32 backWindowBottomLeft  = backHoodTopLeft + windowSeparatorWidth;
	I32 backWindowBottomRight = backHoodTopRight - windowSeparatorWidth;
	I32 backWindowTopLeft     = frontWindowBottomLeft;
	I32 backWindowTopRight    = frontWindowBottomRight;
	I32 backWindowPoly[] = 
	{
		backWindowTop,    backWindowTopLeft,
		backWindowTop,    backWindowTopRight,
		backWindowBottom, backWindowBottomRight,
		backWindowBottom, backWindowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backWindowPoly, borderColor);
	I32 backWindowCenterRow = (backWindowTop + backWindowBottom) / 2;
	I32 backWindowCenterCol = (backWindowTopLeft + backWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, backWindowCenterRow, backWindowCenterCol, windowColor, tmpArena);

	I32 topLeftWindowLeft     = frontWindowTopLeft - windowSeparatorWidth;
	I32 topLeftWindowRight    = frontWindowBottomLeft - windowSeparatorWidth;
	I32 topLeftWindowLeftTop  = frontWindowTop + windowSeparatorWidth;
	I32 topLeftWindowRightTop = frontWindowBottom + windowSeparatorWidth;
	I32 topLeftWindowBottom   = topLeftWindowRightTop + 25;
	I32 topLeftWindowPoly[] = 
	{
		topLeftWindowLeftTop,  topLeftWindowLeft,
		topLeftWindowRightTop, topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topLeftWindowPoly, borderColor);
	I32 topLeftWindowCenterRow = (topLeftWindowLeftTop + topLeftWindowBottom) / 2;
	I32 topLeftWindowCenterCol = (topLeftWindowLeft + topLeftWindowRight) / 2;
	FloodfillBitmap(carBitmap, topLeftWindowCenterRow, topLeftWindowCenterCol, windowColor, tmpArena);

	I32 bottomLeftWindowLeft        = topLeftWindowLeft;
	I32 bottomLeftWindowRight       = topLeftWindowRight;
	I32 bottomLeftWindowTop         = topLeftWindowBottom + windowSeparatorWidth;
	I32 bottomLeftWindowLeftBottom  = backWindowBottom - windowSeparatorWidth;
	I32 bottomLeftWindowRightBottom = backWindowTop - windowSeparatorWidth;
	I32 bottomLeftWindowPoly[] = 
	{
		bottomLeftWindowTop,         bottomLeftWindowLeft,
		bottomLeftWindowTop,         bottomLeftWindowRight,
		bottomLeftWindowRightBottom, bottomLeftWindowRight,
		bottomLeftWindowLeftBottom,  bottomLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomLeftWindowPoly, borderColor);
	I32 bottomLeftWindowCenterRow = (bottomLeftWindowTop + bottomLeftWindowLeftBottom) / 2;
	I32 bottomLeftWindowCenterCol = (bottomLeftWindowLeft + bottomLeftWindowRight) / 2;
	FloodfillBitmap (carBitmap, bottomLeftWindowCenterRow, bottomLeftWindowCenterCol, windowColor, tmpArena);

	I32 topRightWindowLeft = frontWindowBottomRight + windowSeparatorWidth;
	I32 topRightWindowRight = frontWindowTopRight + windowSeparatorWidth;
	I32 topRightWindowLeftTop = frontWindowBottom + windowSeparatorWidth;
	I32 topRightWindowRightTop = frontWindowTop + windowSeparatorWidth;
	I32 topRightWindowBottom = topRightWindowLeftTop + 25;
	I32 topRightWindowPoly[] = 
	{
		topRightWindowLeftTop,  topRightWindowLeft,
		topRightWindowRightTop, topRightWindowRight,
		topRightWindowBottom,   topRightWindowRight,
		topRightWindowBottom,   topRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topRightWindowPoly, borderColor);
	I32 topRightWindowCenterRow = (topRightWindowRightTop + topRightWindowBottom) / 2;
	I32 topRightWindowCenterCol = (topRightWindowLeft + topRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, topRightWindowCenterRow, topRightWindowCenterCol, windowColor, tmpArena);

	I32 bottomRightWindowLeft        = topRightWindowLeft;
	I32 bottomRightWindowRight       = topRightWindowRight;
	I32 bottomRightWindowTop         = topRightWindowBottom + windowSeparatorWidth;
	I32 bottomRightWindowLeftBottom  = backWindowTop - windowSeparatorWidth;
	I32 bottomRightWindowRightBottom = backWindowBottom - windowSeparatorWidth;
	I32 bottomRightWindowPoly[] = 
	{
		bottomRightWindowTop,         bottomRightWindowLeft,
		bottomRightWindowTop,         bottomRightWindowRight,
		bottomRightWindowRightBottom, bottomRightWindowRight,
		bottomRightWindowLeftBottom,  bottomRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomRightWindowPoly, borderColor);
	I32 bottomRightWindowCenterRow = (bottomRightWindowTop + bottomRightWindowRightBottom) / 2;
	I32 bottomRightWindowCenterCol = (bottomRightWindowLeft + bottomRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, bottomRightWindowCenterRow, bottomRightWindowCenterCol, windowColor, tmpArena);

	I32 leftShadowPoly[] = 
	{
		frontHoodTop,      frontHoodTopLeft,
		frontHoodTop,      frontHoodTopLeft + 5,
		frontWindowTop,    frontWindowTopLeft,
		frontWindowBottom, frontWindowBottomLeft,
		backWindowTop,     backWindowTopLeft,
		backWindowBottom,  backWindowBottomLeft,
		backHoodBottom,    backHoodBottomLeft + 5,
		backHoodBottom,    backHoodBottomLeft,
		backHoodTop,       backHoodTopLeft,
		frontHoodBottom,   frontHoodBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 10, leftShadowPoly, shadowColor);
	FloodfillBitmap(carBitmap, frontHoodTop + 1, frontHoodTopLeft + 1, shadowColor, tmpArena);

	I32 rightShadowPoly[] = 
	{
		frontHoodTop, frontHoodTopRight - 5,
		frontHoodTop, frontHoodTopRight,
		frontHoodBottom, frontHoodBottomRight,
		backHoodTop, backHoodTopRight,
		backHoodBottom, backHoodBottomRight,
		backHoodBottom, backHoodBottomRight - 5,
		backWindowBottom, backWindowBottomRight,
		backWindowTop, backWindowTopRight,
		frontWindowBottom, frontWindowBottomRight,
		frontWindowTop, frontWindowTopRight
	};
	DrawBitmapPolyOutline(carBitmap, 10, rightShadowPoly, shadowColor);
	FloodfillBitmap(carBitmap, frontHoodTop + 1, frontHoodTopRight - 1, shadowColor, tmpArena);

	I32 frontShadowTopLeft     = frontHoodTopLeft + 10;
	I32 frontShadowBottomLeft  = frontHoodTopLeft;
	I32 frontShadowTopRight    = frontHoodTopRight - 10;
	I32 frontShadowBottomRight = frontHoodTopRight;
	I32 frontShadowTop         = frontHoodTop - 3;
	I32 frontShadowBottom      = frontHoodTop;
	I32 frontShadowPoly[] = 
	{
		frontShadowTop,    frontShadowTopLeft,
		frontShadowTop,    frontShadowTopRight,
		frontShadowBottom, frontShadowBottomRight,
		frontShadowBottom, frontShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontShadowPoly, shadowColor);
	I32 frontShadowCenterRow = (frontShadowTop + frontShadowBottom) / 2;
	I32 frontShadowCenterCol = (frontShadowTopLeft + frontShadowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontShadowCenterRow, frontShadowCenterCol, shadowColor, tmpArena);

	I32 backShadowTopLeft     = backHoodBottomLeft;
	I32 backShadowBottomLeft  = backHoodBottomLeft + 10;
	I32 backShadowTopRight    = backHoodBottomRight;
	I32 backShadowBottomRight = backHoodBottomRight - 10;
	I32 backShadowTop         = backHoodBottom;
	I32 backShadowBottom      = backHoodBottom + 3;
	I32 backShadowPoly[] = 
	{
		backShadowTop,    backShadowTopLeft,
		backShadowTop,    backShadowTopRight,
		backShadowBottom, backShadowBottomRight,
		backShadowBottom, backShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backShadowPoly, shadowColor);
	I32 backShadowCenterRow = (backShadowTop + backShadowBottom) / 2;
	I32 backShadowCenterCol = (backShadowTopLeft + backShadowTopRight) / 2;
	FloodfillBitmap (carBitmap, backShadowCenterRow, backShadowCenterCol, shadowColor, tmpArena);

	I32 mirrorWidth = IntRandom(8, 12);
	I32 mirrorThickness = IntRandom(2, 4);
	I32 mirrorRowDiff = IntRandom(3, 6);

	I32 leftMirrorRight       = frontHoodBottomLeft;
	I32 leftMirrorLeft        = leftMirrorRight - mirrorWidth;
	I32 leftMirrorRightTop    = frontHoodBottom + 2;
	I32 leftMirrorLeftTop     = leftMirrorRightTop + mirrorThickness;
	I32 leftMirrorRightBottom = leftMirrorRightTop + mirrorRowDiff;
	I32 leftMirrorLeftBottom  = leftMirrorLeftTop + mirrorRowDiff;
	I32 leftMirrorPoly[] = 
	{
		leftMirrorLeftTop,     leftMirrorLeft,
		leftMirrorRightTop,    leftMirrorRight,
		leftMirrorRightBottom, leftMirrorRight,
		leftMirrorLeftBottom,  leftMirrorLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, leftMirrorPoly, shadowColor);
	I32 leftMirrorCenterRow = (leftMirrorLeftTop + leftMirrorLeftBottom) / 2;
	I32 leftMirrorCenterCol = leftMirrorLeft + 2;
	FloodfillBitmap(carBitmap, leftMirrorCenterRow, leftMirrorCenterCol, shadowColor, tmpArena);

	I32 rightMirrorLeft        = frontHoodBottomRight;
	I32 rightMirrorRight       = rightMirrorLeft + mirrorWidth;
	I32 rightMirrorLeftTop     = frontHoodBottom + 2;
	I32 rightMirrorRightTop    = rightMirrorLeftTop + mirrorThickness;
	I32 rightMirrorLeftBottom  = rightMirrorLeftTop + mirrorRowDiff;
	I32 rightMirrorRightBottom = rightMirrorRightTop + mirrorRowDiff;
	I32 rightMirrorPoly[] = 
	{
		rightMirrorLeftTop,     rightMirrorLeft,
		rightMirrorRightTop,    rightMirrorRight,
		rightMirrorRightBottom, rightMirrorRight,
		rightMirrorLeftBottom,  rightMirrorLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, rightMirrorPoly, shadowColor);
	I32 rightMirrorCenterRow = (rightMirrorRightTop + rightMirrorRightBottom) / 2;
	I32 rightMirrorCenterCol = rightMirrorRight - 2;
	FloodfillBitmap(carBitmap, rightMirrorCenterRow, rightMirrorCenterCol, shadowColor, tmpArena);

	V4 frontLampColor = GetRandomFrontLampColor();
	I32 frontLampWidth  = IntRandom(15, 20);
	I32 frontLampHeight = IntRandom(2, 3);
	I32 frontLampSideDistance = IntRandom(7, 10);

	I32 frontLeftLampLeft   = frontHoodTopLeft + frontLampSideDistance;
	I32 frontLeftLampRight  = frontLeftLampLeft + frontLampWidth;
	I32 frontLeftLampTop    = frontHoodTop;
	I32 frontLeftLampBottom = frontLeftLampTop + frontLampHeight;
	I32 frontLeftLampPoly[] = 
	{
		frontLeftLampTop,    frontLeftLampLeft,
		frontLeftLampTop,    frontLeftLampRight,
		frontLeftLampBottom, frontLeftLampRight,
		frontLeftLampBottom, frontLeftLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontLeftLampPoly, frontLampColor);
	I32 frontLeftLampCenterRow = (frontLeftLampTop + frontLeftLampBottom) / 2;
	I32 frontLeftLampCenterCol = (frontLeftLampLeft + frontLeftLampRight) / 2;
	FloodfillBitmap(carBitmap, frontLeftLampCenterRow, frontLeftLampCenterCol, frontLampColor, tmpArena);

	I32 frontRightLampRight = frontHoodTopRight - frontLampSideDistance;
	I32 frontRightLampLeft = frontRightLampRight - frontLampWidth;
	I32 frontRightLampTop = frontHoodTop;
	I32 frontRightLampBottom = frontRightLampTop + frontLampHeight;
	I32 frontRightLampPoly[] = 
	{
		frontRightLampTop,    frontRightLampLeft,
		frontRightLampTop,    frontRightLampRight,
		frontRightLampBottom, frontRightLampRight,
		frontRightLampBottom, frontRightLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontRightLampPoly, frontLampColor);
	I32 frontRightLampCenterRow = (frontRightLampTop + frontRightLampBottom) / 2;
	I32 frontRightLampCenterCol = (frontRightLampLeft + frontRightLampRight) / 2;
	FloodfillBitmap(carBitmap, frontRightLampCenterRow, frontRightLampCenterCol, frontLampColor, tmpArena);

	V4 backLampColor = MakeColor (1.0f, 0.0f, 0.0f);
	I32 backLeftLampLeft   = backHoodBottomLeft + 10;
	I32 backLeftLampRight  = backLeftLampLeft + 7;
	I32 backLeftLampTop    = backHoodBottom;
	I32 backLeftLampBottom = backLeftLampTop + 2;
	I32 backLeftLampPoly[] = 
	{
		backLeftLampTop,    backLeftLampLeft,
		backLeftLampTop,    backLeftLampRight,
		backLeftLampBottom, backLeftLampRight,
		backLeftLampBottom, backLeftLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backLeftLampPoly, backLampColor);
	I32 backLeftLampCenterRow = (backLeftLampTop + backLeftLampBottom) / 2;
	I32 backLeftLampCenterCol = (backLeftLampLeft + backLeftLampRight) / 2;
	FloodfillBitmap(carBitmap, backLeftLampCenterRow, backLeftLampCenterCol, backLampColor, tmpArena);

	I32 backRightLampRight = backHoodBottomRight - 10;
	I32 backRightLampLeft = backRightLampRight - 7;
	I32 backRightLampBottom = backHoodBottom;
	I32 backRightLampTop = backRightLampBottom + 2;
	I32 backRightLampPoly[] = 
	{
		backRightLampTop,    backRightLampLeft,
		backRightLampTop,    backRightLampRight,
		backRightLampBottom, backRightLampRight,
		backRightLampBottom, backRightLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backRightLampPoly, backLampColor);
	I32 backRightLampCenterRow = (backRightLampTop + backRightLampBottom) / 2;
	I32 backRightLampCenterCol = (backRightLampLeft + backRightLampRight) / 2;
	FloodfillBitmap(carBitmap, backRightLampCenterRow, backRightLampCenterCol, backLampColor, tmpArena);
}

static void func MoveAutoCarToJunction(AutoCar* autoCar, Junction* junction, MemArena* tmpArena, PathPool* pathPool)
{
	MapElem targetElem = GetJunctionElem(autoCar->onJunction);
	MapElem nextElem = GetJunctionElem(junction);

	autoCar->moveNode = ConnectElems(autoCar->car.map, targetElem, nextElem, tmpArena, pathPool);

	if(autoCar->moveNode) 
	{
		autoCar->moveStartPoint = StartNodePoint(autoCar->moveNode);
		autoCar->moveEndPoint = NextNodePoint(autoCar->moveNode, autoCar->moveStartPoint);
		autoCar->moveBezier4 = TurnBezier4(autoCar->moveStartPoint, autoCar->moveEndPoint);
		autoCar->bezierRatio = 0.0f;
		autoCar->moveTargetJunction = junction;

		V4 point = Bezier4DirectedPoint(autoCar->moveBezier4, autoCar->bezierRatio);
		MoveCar(&autoCar->car, point);
	}
}

static B32 func IsAutoCarBeforeARedLight(AutoCar* autoCar)
{
	B32 result = false;
	PathNode* moveNode = autoCar->moveNode;
	if(moveNode) 
	{
		if(IsNodeEndPoint(moveNode, autoCar->moveEndPoint))
		{
			PathNode* nextNode = moveNode->next;
			if(nextNode) 
			{
				MapElem moveElem = moveNode->elem;
				MapElem nextElem = nextNode->elem;

				if(moveElem.type == MapElemRoad && nextElem.type == MapElemJunction) 
				{
					Road* road = moveElem.road;
					Junction* junction = nextElem.junction;
					TrafficLight* trafficLight = 0;
					for(I32 i = 0; i < junction->roadN; ++i) 
					{
						if(junction->roads[i] == road)
						{
							trafficLight = &junction->trafficLights[i];
						}
					}

					Assert(trafficLight != 0);
					if(trafficLight->color == TrafficLightRed || trafficLight->color == TrafficLightYellow) {
						F32 distanceLeft = Distance(autoCar->car.position, autoCar->moveEndPoint.position);
						if(distanceLeft < autoCar->car.length * 1.5f)
						{
							result = true;
						}
					}
				}
			}
		}
	}
	return result;
}

static void func UpdateAutoCar(AutoCar* autoCar, F32 seconds, MemArena* tmpArena, PathPool* pathPool)
{
	Car* car = &autoCar->car;

	if(IsAutoCarBeforeARedLight(autoCar))
	{
		autoCar->acceleration = -25.0f;
	}

	if(autoCar->moveTargetJunction) 
	{
		car->moveSpeed += seconds * autoCar->acceleration;
		car->moveSpeed = Clip(car->moveSpeed, 0.0f, car->maxSpeed);
		F32 distanceToGo = seconds * car->moveSpeed;
		while(distanceToGo > 0.0f) 
		{
			PathNode* moveNode = autoCar->moveNode;
			if(!moveNode) 
			{
				autoCar->onJunction = autoCar->moveTargetJunction;
				autoCar->moveTargetJunction = 0;
				break;
			}

			F32 bezierRatio = MoveOnBezier4(autoCar->moveBezier4, autoCar->bezierRatio, distanceToGo);
			if (bezierRatio == 1.0f) 
			{
				autoCar->moveStartPoint = autoCar->moveEndPoint;
				F32 distance = GetBezier4DistanceFromEnd(autoCar->moveBezier4, autoCar->bezierRatio);
				distanceToGo -= distance;
				if(IsNodeEndPoint(moveNode, autoCar->moveStartPoint)) 
				{
					moveNode = moveNode->next;
					FreePathNode(autoCar->moveNode, pathPool);
					autoCar->moveNode = moveNode;

					if(!moveNode)
					{
						continue;
					}
				}
				
				autoCar->bezierRatio = 0.0f;
				autoCar->moveEndPoint = NextNodePoint (moveNode, autoCar->moveStartPoint);
				autoCar->moveBezier4 = TurnBezier4 (autoCar->moveStartPoint, autoCar->moveEndPoint);
			} 
			else
			{
				distanceToGo = 0.0f;
				autoCar->bezierRatio = bezierRatio;
			}
		}
	} 
	else 
	{
		Junction* targetJunction = GetRandomJunction(car->map);
		MoveAutoCarToJunction(autoCar, targetJunction, tmpArena, pathPool);
	}

	V4 point = Bezier4DirectedPoint(autoCar->moveBezier4, autoCar->bezierRatio);
	MoveCar(car, point);

	Assert(IsBetween(autoCar->bezierRatio, 0.0f, 1.0f));
}

static V2 func GetCarCorner(Car* car, I32 cornerIndex)
{
	Assert(IsIntBetween(cornerIndex, 0, 3));

	F32 halfWidth = car->width * 0.5f;
	F32 halfLength = car->length * 0.5f;

	F32 angle = car->angle;
	V2 rotationUpDown = RotationVector(angle);
	V2 rotationLeftRight = RotationVector(angle + PI * 0.5f);
	V2 center = car->position;
	V2 frontCenter = center + (halfLength * rotationUpDown);
	V2 backCenter = center - (halfLength * rotationUpDown);

	V2 result = {};

	switch(cornerIndex)
	{
		case 0:
		{
			result = frontCenter - (halfWidth * rotationLeftRight);
			break;
		}
		case 1:
		{
			result = frontCenter + (halfWidth * rotationLeftRight);
			break;
		}
		case 2:
		{
			result = backCenter  + (halfWidth * rotationLeftRight);
			break;
		}
		case 3:
		{
			result = backCenter  - (halfWidth * rotationLeftRight);
			break;
		}
		default:
		{
			DebugBreak();
			break;
		}
	}
	return result;
}

static void func UpdatePlayerCarWithoutCollision(PlayerCar* car, F32 seconds)
{
	F32 speed = VectorLength(car->velocity);
	V2 carDirection = RotationVector(car->car.angle);

	F32 maxControlSpeed = 10.0f;
	F32 maxControlTurn = 1.0f;
	F32 turnRatio = 0.6f;
	if(speed > maxControlSpeed)
	{
		turnRatio = maxControlSpeed / speed;
	}

	F32 frontWheelTarget = turnRatio * car->frontWheelAngleTarget * maxControlTurn;
	if(car->frontWheelAngle < car->frontWheelAngleTarget) 
	{
		car->frontWheelAngle += seconds * maxControlTurn;
		car->frontWheelAngle = Min2(car->frontWheelAngle, frontWheelTarget);
	} 
	else 
	{
		car->frontWheelAngle -= seconds * maxControlTurn;
		car->frontWheelAngle = Max2(car->frontWheelAngle, frontWheelTarget);
	}

	V2 frontPositionStart = car->car.position + (0.5f * car->car.length * carDirection);
	F32 frontAngle = car->car.angle + car->frontWheelAngle;
	V2 frontDirection = RotationVector(frontAngle);
	V2 frontPositionEnd = frontPositionStart + seconds * speed * frontDirection;

	V2 backPositionStart = car->car.position - (0.5f * car->car.length * carDirection);
	F32 backAngle = car->car.angle;
	V2 backDirection = RotationVector(backAngle);
	V2 backPositionEnd = backPositionStart + seconds * speed * backDirection;

	// car->car.angle = LineAngle(backPositionEnd, frontPositionEnd);
	F32 turnRadius = 1.0f;
	F32 turnPeriphery = 2.0f * PI * turnRadius;
	F32 distance = seconds * speed;
	car->car.angle += (distance / turnPeriphery) * car->frontWheelAngle;

	V2 direction = RotationVector(car->car.angle);
	V2 tractionForce = ((car->engineForce) * CarMass * direction);

	F32 dragConstant = 0.05f;
	V2 drag = ((-dragConstant * speed) * CarMass * car->velocity);

	F32 sideResistanceConstant = 1.0f;
	F32 sideAngle = car->car.angle + PI * 0.5f;
	V2 sideVector = RotationVector(sideAngle);
	V2 sideResistance = (-sideResistanceConstant) * CarMass * DotProduct(car->velocity, sideVector) * sideVector;

	V2 oldPosition = car->car.position;
	car->acceleration = Invert (CarMass) *  (tractionForce + drag + sideResistance);
	car->velocity = car->velocity + (seconds * car->acceleration);
	car->car.position = car->car.position + (seconds * car->velocity);

	F32 angularResistanceConstant = 0.5f;
	F32 angularResistance = ((-angularResistanceConstant) * car->inertia * car->angularVelocity);

	car->angularAcceleration = Invert (car->inertia) * angularResistance;

	F32 oldAngle = car->car.angle;
	car->angularVelocity = car->angularVelocity + (seconds * car->angularAcceleration);
	car->car.angle = car->car.angle + (seconds * car->angularVelocity);
}

static CollisionInfo func operator+(CollisionInfo hit1, CollisionInfo hit2)
{
	CollisionInfo hit = {};

	hit.count = hit1.count + hit2.count;
	if(hit2.count > 0) 
	{
		hit.point = hit2.point;
		hit.normalVector = hit2.normalVector;
	} 
	else 
	{
		hit.point = hit1.point;
		hit.normalVector = hit1.normalVector;
	}

	return hit;
}

static CollisionInfo func GetCarLineCollisionInfo(Car* oldCar, Car* newCar, V2 point1, V2 point2)
{
	CollisionInfo hit = {};

	for(I32 i = 0; i < 4; i++) 
	{
		V2 oldCorner = GetCarCorner(oldCar, i);
		V2 newCorner = GetCarCorner(newCar, i);
		CollisionInfo hitCorner = {};
		if(DoLinesCross(point1, point2, oldCorner, newCorner)) 
		{
			V2 wallAngle = PointDirection(point1, point2);
			hitCorner.normalVector = TurnVectorToRight(wallAngle);
			hitCorner.point = oldCorner;
			hitCorner.count++;
		}
		hit = hit + hitCorner;
	}

	return hit;
}

static V2 func GetCarRelativeCoordinates(Car* car, V2 point)
{
	V2 result = {};
	V2 pointVector = (point - car->position);

	V2 frontDirection = RotationVector(car->angle);
	result.x = DotProduct(pointVector, frontDirection);

	V2 sideDirection = TurnVectorToRight(frontDirection);
	result.y = DotProduct(pointVector, sideDirection);
	return result;
}

static CollisionInfo func GetCarPointCollisionInfo(Car* oldCar, Car* newCar, V2 point)
{
	CollisionInfo hit = {};

	F32 halfLength = oldCar->length * 0.5f;
	F32 halfWidth  = oldCar->width * 0.5f;
	V2 topLeft     = MakePoint(-halfLength, -halfWidth);
	V2 topRight    = MakePoint(-halfLength, +halfWidth); 
	V2 bottomLeft  = MakePoint(+halfLength, -halfWidth);
	V2 bottomRight = MakePoint(+halfLength, +halfWidth);

	V2 oldRelativePoint = GetCarRelativeCoordinates(oldCar, point);
	V2 newRelativePoint = GetCarRelativeCoordinates(newCar, point);

	if(DoLinesCross(oldRelativePoint, newRelativePoint, topLeft, topRight)) 
	{
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle);
	}
	if(DoLinesCross(oldRelativePoint, newRelativePoint, bottomLeft, bottomRight)) 
	{
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle);
	}
	if(DoLinesCross(oldRelativePoint, newRelativePoint, topLeft, bottomLeft)) 
	{
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle + PI * 0.5f);
	}
	if(DoLinesCross(oldRelativePoint, newRelativePoint, topRight, bottomRight)) 
	{
		hit.count++;
		hit.point = point;
		hit.normalVector = RotationVector(oldCar->angle + PI * 0.5f);
	}

	return hit;
}

static CollisionInfo func GetCarPolyCollisionInfo(Car* oldCar, Car* newCar, V2* points, I32 pointN)
{
	CollisionInfo hit = {};

	for(I32 i = 0; i < pointN; i++) 
	{
		I32 j = i + 1;
		if(j == pointN)
		{
			j = 0;
		}

		CollisionInfo hitLine = GetCarLineCollisionInfo(oldCar, newCar, points[i], points[j]);
		hit = hit + hitLine;

		CollisionInfo hitPoint = GetCarPointCollisionInfo(oldCar, newCar, points[i]);
		hit = hit + hitPoint;
	}

	return hit;
}

static V2 func GetCarPointVelocity(PlayerCar* car, V2 point)
{
	V2 velocity = car->velocity;
	V2 pointVector = (point - car->car.position);
	V2 turnVelocity = car->angularVelocity * TurnVectorToRight(pointVector);
	velocity = velocity + turnVelocity;
	return velocity;
}

static void func UpdatePlayerCarCollision(PlayerCar* car, PlayerCar* oldCar, F32 seconds, CollisionInfo hit)
{
	if(hit.count > 1) 
	{
		car->velocity = MakeVector (0.0f, 0.0f);
		car->car.position = oldCar->car.position;

		car->angularVelocity = 0.0f;
		car->car.angle = oldCar->car.angle;
	}

	if(hit.count == 1) 
	{
		V2 hitPointVelocity = GetCarPointVelocity(car, hit.point);
		V2 carCenter = car->car.position;

		F32 reflectionRatio = 0.5f;
		V2 turnForce = TurnVectorToRight(hit.point - carCenter);

		F32 up = -(1.0f + reflectionRatio) * DotProduct(hitPointVelocity, hit.normalVector);
		F32 down = DotProduct(hit.normalVector, hit.normalVector) * Invert(CarMass) +
			Square(DotProduct(turnForce, hit.normalVector)) * Invert(car->inertia);
		F32 forceRatio = up / down;

		car->velocity = car->velocity + (forceRatio / CarMass) * hit.normalVector;
		car->angularVelocity = car->angularVelocity + 
									DotProduct(turnForce, forceRatio * hit.normalVector) / car->inertia;

		car->car.position = oldCar->car.position + seconds * car->velocity;
		car->car.angle = oldCar->car.angle + seconds * car->angularVelocity;
	}

	/*
	TODO: Check why this happens!
	for(int i = 0; i < 4; i++) 
	{
		V2 oldCorner = oldP[i];
		V2 newCorner = GetCorner(labState, i);
		Assert(!DoLinesCross(wall1, wall2, oldCorner, newCorner));
	}
	*/
}