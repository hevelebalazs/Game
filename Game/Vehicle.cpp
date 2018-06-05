#include "Geometry.h"
#include "Math.h"
#include "Vehicle.h"

extern float MinVehicleSpeed = 5.0f;
extern float MaxVehicleSpeed = 10.0f;

#define CarBitmapWidth 140
#define CarBitmapHeight 300

// TODO: use a vehicle coordinate system?
static inline Quad GetVehicleCorners(Vehicle* vehicle) {
	Quad result = {};

	Point addWidth = PointProd((vehicle->width * 0.5f), RotationVector(vehicle->angle + PI * 0.5f));

	Point side1 = PointSum(vehicle->position, addWidth);
	Point side2 = PointDiff(vehicle->position, addWidth);

	Point addLength = PointProd((vehicle->length * 0.5f), RotationVector(vehicle->angle));

	result.points[0] = PointSum(side1, addLength);
	result.points[1] = PointDiff(side1, addLength);
	result.points[2] = PointDiff(side2, addLength); 
	result.points[3] = PointSum(side2, addLength);
	return result;
}

Quad GetVehicleStopArea(Vehicle* vehicle) {
	Point addWidth = PointProd((vehicle->width * 0.5f), RotationVector(vehicle->angle + PI * 0.5f));

	Point side1 = PointSum(vehicle->position, addWidth);
	Point side2 = PointDiff(vehicle->position, addWidth);

	float stopDistance = 5.0f;

	Point addClose = PointProd((vehicle->length * 0.5f), RotationVector(vehicle->angle));
	Point addFar = PointProd((vehicle->length * 0.5f + stopDistance), RotationVector(vehicle->angle));

	Quad result = {};
	result.points[0] = PointSum(side1, addClose);
	result.points[1] = PointSum(side2, addClose);
	result.points[2] = PointSum(side2, addFar); 
	result.points[3] = PointSum(side1, addFar);

	return result;
}

bool IsVehicleOnPoint(Vehicle* vehicle, Point point) {
	Quad vehicleCorners = GetVehicleCorners(vehicle);
	bool result = IsPointInQuad(vehicleCorners, point);
	return result;
}

void MoveVehicle(Vehicle* vehicle, DirectedPoint point) {
	vehicle->position = point.position;
	vehicle->angle = VectorAngle(point.direction);
}

void DrawVehicle(Renderer renderer, Vehicle vehicle)
{
	Assert(vehicle.bitmap != 0);
	DrawScaledRotatedBitmap(renderer, vehicle.bitmap, vehicle.position, vehicle.width, vehicle.length, vehicle.angle);
}

static Color GetRandomCarColor()
{
	float colorSum = RandomBetween(1.0f, 1.5f);
	float red = RandomBetween(0.0f, colorSum);
	red = Clip(red, 0.0f, 1.0f);
	float green = RandomBetween(0.0f, colorSum - red);
	green = Clip(green, 0.0f, 1.0f);
	float blue = RandomBetween(0.0f, colorSum - red - green);
	blue = Clip(blue, 0.0f, 1.0f);
	Color color = GetColor(red, green, blue);
	return color;
}

static Color GetRandomWindowColor()
{
	float red   = RandomBetween(0.0f, 0.05f);
	float green = RandomBetween(0.0f, 0.05f);
	float blue  = RandomBetween(0.0f, 0.25f);
	Color color = GetColor(red, green, blue);
	return color;
}

static Color GetShadowColor(Color color)
{
	float shadowRatio = 0.5f;
	float red   = shadowRatio * color.red;
	float green = shadowRatio * color.green;
	float blue  = shadowRatio * color.blue;
	Color shadowColor = GetColor(red, green, blue);
	return shadowColor;
}

static Color GetRandomFrontLampColor()
{
	float redGreenValue = RandomBetween(0.0f, 1.0f);
	float red   = redGreenValue;
	float green = redGreenValue;
	float blue  = RandomBetween(0.8f, 1.0f);
	Color color = GetColor(red, green, blue);
	return color;
}

void AllocateCarBitmap(Bitmap* carBitmap)
{
	ResizeBitmap(carBitmap, CarBitmapWidth, CarBitmapHeight);
}

void GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena)
{
	Assert(carBitmap != 0);

	Color backgroundColor = GetAlphaColor(0.2f, 0.2f, 0.2f, 0.0f);
	FillBitmapWithColor(carBitmap, backgroundColor);

	Color carColor = GetRandomCarColor();
	Color shadowColor = GetShadowColor(carColor);

	Color borderColor = GetColor(0.2f, 0.2f, 0.2f);

	int bitmapLeft   = 0;
	int bitmapRight  = carBitmap->width - 1;
	int bitmapTop    = 0;
	int bitmapBottom = carBitmap->height - 1;

	int frontHoodLength    = IntRandom(65, 70);
	int frontHoodWidthDiff = IntRandom(0, 5);

	int frontHoodTopLeft     = bitmapLeft + 20 + frontHoodWidthDiff;
	int frontHoodBottomLeft  = bitmapLeft + 20;
	int frontHoodTopRight    = bitmapRight - 20 - frontHoodWidthDiff;
	int frontHoodBottomRight = bitmapRight - 20;
	int frontHoodTop         = bitmapTop + 20;
	int frontHoodBottom      = frontHoodTop + frontHoodLength;

	int middlePartLength = IntRandom(135, 150);
	
	int backHoodLength    = IntRandom(20, 30);
	int backHoodWidthDiff = IntRandom(0, 5);

	int backHoodTopLeft     = bitmapLeft + 20;
	int backHoodBottomLeft  = bitmapLeft + 20 + backHoodWidthDiff;
	int backHoodTopRight    = bitmapRight - 20;
	int backHoodBottomRight = bitmapRight - 20 - backHoodWidthDiff;
	int backHoodTop         = frontHoodBottom + middlePartLength;
	int backHoodBottom      = backHoodTop + backHoodLength;

	int carPoly[] = {
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
	int carCenterRow = (frontHoodTop + backHoodBottom) / 2;
	int carCenterCol = (frontHoodBottomLeft + frontHoodBottomRight) / 2;
	FloodfillBitmap(carBitmap, carCenterRow, carCenterCol, carColor, tmpArena);

	int frontWindowLength    = IntRandom(40, 60);
	int windowSeparatorWidth = IntRandom(3, 5);
	int frontWindowWidthDiff = IntRandom(5, 15);

	Color windowColor = GetRandomWindowColor();
	int frontWindowTop         = frontHoodBottom;
	int frontWindowBottom      = frontWindowTop + frontWindowLength;
	int frontWindowTopLeft     = frontHoodBottomLeft + windowSeparatorWidth;
	int frontWindowTopRight    = frontHoodBottomRight - windowSeparatorWidth;
	int frontWindowBottomLeft  = frontWindowTopLeft + frontWindowWidthDiff;
	int frontWindowBottomRight = frontWindowTopRight - frontWindowWidthDiff;
	int frontWindowPoly[] = {
		frontWindowTop,    frontWindowTopLeft,
		frontWindowTop,    frontWindowTopRight,
		frontWindowBottom, frontWindowBottomRight,
		frontWindowBottom, frontWindowBottomLeft
	};

	DrawBitmapPolyOutline(carBitmap, 4, frontWindowPoly, borderColor);
	int frontWindowCenterRow = (frontWindowTop + frontWindowBottom) / 2;
	int frontWindowCenterCol = (frontWindowTopLeft + frontWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontWindowCenterRow, frontWindowCenterCol, windowColor, tmpArena);

	int backWindowLength = IntRandom(15, 25);
	int backWindowWidthDiff = IntRandom(5, 15);

	int backWindowBottom      = backHoodTop;
	int backWindowTop         = backWindowBottom - backWindowLength;
	int backWindowBottomLeft  = backHoodTopLeft + windowSeparatorWidth;
	int backWindowBottomRight = backHoodTopRight - windowSeparatorWidth;
	int backWindowTopLeft     = frontWindowBottomLeft;
	int backWindowTopRight    = frontWindowBottomRight;
	int backWindowPoly[] = {
		backWindowTop,    backWindowTopLeft,
		backWindowTop,    backWindowTopRight,
		backWindowBottom, backWindowBottomRight,
		backWindowBottom, backWindowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backWindowPoly, borderColor);
	int backWindowCenterRow = (backWindowTop + backWindowBottom) / 2;
	int backWindowCenterCol = (backWindowTopLeft + backWindowTopRight) / 2;
	FloodfillBitmap(carBitmap, backWindowCenterRow, backWindowCenterCol, windowColor, tmpArena);

	int topLeftWindowLeft     = frontWindowTopLeft - windowSeparatorWidth;
	int topLeftWindowRight    = frontWindowBottomLeft - windowSeparatorWidth;
	int topLeftWindowLeftTop  = frontWindowTop + windowSeparatorWidth;
	int topLeftWindowRightTop = frontWindowBottom + windowSeparatorWidth;
	int topLeftWindowBottom   = topLeftWindowRightTop + 25;
	int topLeftWindowPoly[] = {
		topLeftWindowLeftTop,  topLeftWindowLeft,
		topLeftWindowRightTop, topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowRight,
		topLeftWindowBottom,   topLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topLeftWindowPoly, borderColor);
	int topLeftWindowCenterRow = (topLeftWindowLeftTop + topLeftWindowBottom) / 2;
	int topLeftWindowCenterCol = (topLeftWindowLeft + topLeftWindowRight) / 2;
	FloodfillBitmap(carBitmap, topLeftWindowCenterRow, topLeftWindowCenterCol, windowColor, tmpArena);

	int bottomLeftWindowLeft        = topLeftWindowLeft;
	int bottomLeftWindowRight       = topLeftWindowRight;
	int bottomLeftWindowTop         = topLeftWindowBottom + windowSeparatorWidth;
	int bottomLeftWindowLeftBottom  = backWindowBottom - windowSeparatorWidth;
	int bottomLeftWindowRightBottom = backWindowTop - windowSeparatorWidth;
	int bottomLeftWindowPoly[] = {
		bottomLeftWindowTop,         bottomLeftWindowLeft,
		bottomLeftWindowTop,         bottomLeftWindowRight,
		bottomLeftWindowRightBottom, bottomLeftWindowRight,
		bottomLeftWindowLeftBottom,  bottomLeftWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomLeftWindowPoly, borderColor);
	int bottomLeftWindowCenterRow = (bottomLeftWindowTop + bottomLeftWindowLeftBottom) / 2;
	int bottomLeftWindowCenterCol = (bottomLeftWindowLeft + bottomLeftWindowRight) / 2;
	FloodfillBitmap(carBitmap, bottomLeftWindowCenterRow, bottomLeftWindowCenterCol, windowColor, tmpArena);

	int topRightWindowLeft = frontWindowBottomRight + windowSeparatorWidth;
	int topRightWindowRight = frontWindowTopRight + windowSeparatorWidth;
	int topRightWindowLeftTop = frontWindowBottom + windowSeparatorWidth;
	int topRightWindowRightTop = frontWindowTop + windowSeparatorWidth;
	int topRightWindowBottom = topRightWindowLeftTop + 25;
	int topRightWindowPoly[] = {
		topRightWindowLeftTop,  topRightWindowLeft,
		topRightWindowRightTop, topRightWindowRight,
		topRightWindowBottom,   topRightWindowRight,
		topRightWindowBottom,   topRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, topRightWindowPoly, borderColor);
	int topRightWindowCenterRow = (topRightWindowRightTop + topRightWindowBottom) / 2;
	int topRightWindowCenterCol = (topRightWindowLeft + topRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, topRightWindowCenterRow, topRightWindowCenterCol, windowColor, tmpArena);

	int bottomRightWindowLeft        = topRightWindowLeft;
	int bottomRightWindowRight       = topRightWindowRight;
	int bottomRightWindowTop         = topRightWindowBottom + windowSeparatorWidth;
	int bottomRightWindowLeftBottom  = backWindowTop - windowSeparatorWidth;
	int bottomRightWindowRightBottom = backWindowBottom - windowSeparatorWidth;
	int bottomRightWindowPoly[] = {
		bottomRightWindowTop,         bottomRightWindowLeft,
		bottomRightWindowTop,         bottomRightWindowRight,
		bottomRightWindowRightBottom, bottomRightWindowRight,
		bottomRightWindowLeftBottom,  bottomRightWindowLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, bottomRightWindowPoly, borderColor);
	int bottomRightWindowCenterRow = (bottomRightWindowTop + bottomRightWindowRightBottom) / 2;
	int bottomRightWindowCenterCol = (bottomRightWindowLeft + bottomRightWindowRight) / 2;
	FloodfillBitmap(carBitmap, bottomRightWindowCenterRow, bottomRightWindowCenterCol, windowColor, tmpArena);

	int leftShadowPoly[] = {
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

	int rightShadowPoly[] = {
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

	int frontShadowTopLeft     = frontHoodTopLeft + 10;
	int frontShadowBottomLeft  = frontHoodTopLeft;
	int frontShadowTopRight    = frontHoodTopRight - 10;
	int frontShadowBottomRight = frontHoodTopRight;
	int frontShadowTop         = frontHoodTop - 3;
	int frontShadowBottom      = frontHoodTop;
	int frontShadowPoly[] = {
		frontShadowTop,    frontShadowTopLeft,
		frontShadowTop,    frontShadowTopRight,
		frontShadowBottom, frontShadowBottomRight,
		frontShadowBottom, frontShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontShadowPoly, shadowColor);
	int frontShadowCenterRow = (frontShadowTop + frontShadowBottom) / 2;
	int frontShadowCenterCol = (frontShadowTopLeft + frontShadowTopRight) / 2;
	FloodfillBitmap(carBitmap, frontShadowCenterRow, frontShadowCenterCol, shadowColor, tmpArena);

	int backShadowTopLeft     = backHoodBottomLeft;
	int backShadowBottomLeft  = backHoodBottomLeft + 10;
	int backShadowTopRight    = backHoodBottomRight;
	int backShadowBottomRight = backHoodBottomRight - 10;
	int backShadowTop         = backHoodBottom;
	int backShadowBottom      = backHoodBottom + 3;
	int backShadowPoly[] = {
		backShadowTop,    backShadowTopLeft,
		backShadowTop,    backShadowTopRight,
		backShadowBottom, backShadowBottomRight,
		backShadowBottom, backShadowBottomLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backShadowPoly, shadowColor);
	int backShadowCenterRow = (backShadowTop + backShadowBottom) / 2;
	int backShadowCenterCol = (backShadowTopLeft + backShadowTopRight) / 2;
	FloodfillBitmap(carBitmap, backShadowCenterRow, backShadowCenterCol, shadowColor, tmpArena);

	int mirrorWidth = IntRandom(8, 12);
	int mirrorThickness = IntRandom(2, 4);
	int mirrorRowDiff = IntRandom(3, 6);

	int leftMirrorRight       = frontHoodBottomLeft;
	int leftMirrorLeft        = leftMirrorRight - mirrorWidth;
	int leftMirrorRightTop    = frontHoodBottom + 2;
	int leftMirrorLeftTop     = leftMirrorRightTop + mirrorThickness;
	int leftMirrorRightBottom = leftMirrorRightTop + mirrorRowDiff;
	int leftMirrorLeftBottom  = leftMirrorLeftTop + mirrorRowDiff;
	int leftMirrorPoly[] = {
		leftMirrorLeftTop,     leftMirrorLeft,
		leftMirrorRightTop,    leftMirrorRight,
		leftMirrorRightBottom, leftMirrorRight,
		leftMirrorLeftBottom,  leftMirrorLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, leftMirrorPoly, shadowColor);
	int leftMirrorCenterRow = (leftMirrorLeftTop + leftMirrorLeftBottom) / 2;
	int leftMirrorCenterCol = leftMirrorLeft + 2;
	FloodfillBitmap(carBitmap, leftMirrorCenterRow, leftMirrorCenterCol, shadowColor, tmpArena);

	int rightMirrorLeft        = frontHoodBottomRight;
	int rightMirrorRight       = rightMirrorLeft + mirrorWidth;
	int rightMirrorLeftTop     = frontHoodBottom + 2;
	int rightMirrorRightTop    = rightMirrorLeftTop + mirrorThickness;
	int rightMirrorLeftBottom  = rightMirrorLeftTop + mirrorRowDiff;
	int rightMirrorRightBottom = rightMirrorRightTop + mirrorRowDiff;
	int rightMirrorPoly[] = {
		rightMirrorLeftTop,     rightMirrorLeft,
		rightMirrorRightTop,    rightMirrorRight,
		rightMirrorRightBottom, rightMirrorRight,
		rightMirrorLeftBottom,  rightMirrorLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, rightMirrorPoly, shadowColor);
	int rightMirrorCenterRow = (rightMirrorRightTop + rightMirrorRightBottom) / 2;
	int rightMirrorCenterCol = rightMirrorRight - 2;
	FloodfillBitmap(carBitmap, rightMirrorCenterRow, rightMirrorCenterCol, shadowColor, tmpArena);

	Color frontLampColor = GetRandomFrontLampColor();
	int frontLampWidth  = IntRandom(15, 20);
	int frontLampHeight = IntRandom(2, 3);
	int frontLampSideDistance = IntRandom(7, 10);

	int frontLeftLampLeft   = frontHoodTopLeft + frontLampSideDistance;
	int frontLeftLampRight  = frontLeftLampLeft + frontLampWidth;
	int frontLeftLampTop    = frontHoodTop;
	int frontLeftLampBottom = frontLeftLampTop + frontLampHeight;
	int frontLeftLampPoly[] = {
		frontLeftLampTop,    frontLeftLampLeft,
		frontLeftLampTop,    frontLeftLampRight,
		frontLeftLampBottom, frontLeftLampRight,
		frontLeftLampBottom, frontLeftLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontLeftLampPoly, frontLampColor);
	int frontLeftLampCenterRow = (frontLeftLampTop + frontLeftLampBottom) / 2;
	int frontLeftLampCenterCol = (frontLeftLampLeft + frontLeftLampRight) / 2;
	FloodfillBitmap(carBitmap, frontLeftLampCenterRow, frontLeftLampCenterCol, frontLampColor, tmpArena);

	int frontRightLampRight = frontHoodTopRight - frontLampSideDistance;
	int frontRightLampLeft = frontRightLampRight - frontLampWidth;
	int frontRightLampTop = frontHoodTop;
	int frontRightLampBottom = frontRightLampTop + frontLampHeight;
	int frontRightLampPoly[] = {
		frontRightLampTop,    frontRightLampLeft,
		frontRightLampTop,    frontRightLampRight,
		frontRightLampBottom, frontRightLampRight,
		frontRightLampBottom, frontRightLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, frontRightLampPoly, frontLampColor);
	int frontRightLampCenterRow = (frontRightLampTop + frontRightLampBottom) / 2;
	int frontRightLampCenterCol = (frontRightLampLeft + frontRightLampRight) / 2;
	FloodfillBitmap(carBitmap, frontRightLampCenterRow, frontRightLampCenterCol, frontLampColor, tmpArena);

	Color backLampColor = GetColor(1.0f, 0.0f, 0.0f);
	int backLeftLampLeft   = backHoodBottomLeft + 10;
	int backLeftLampRight  = backLeftLampLeft + 7;
	int backLeftLampTop    = backHoodBottom;
	int backLeftLampBottom = backLeftLampTop + 2;
	int backLeftLampPoly[] = {
		backLeftLampTop,    backLeftLampLeft,
		backLeftLampTop,    backLeftLampRight,
		backLeftLampBottom, backLeftLampRight,
		backLeftLampBottom, backLeftLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backLeftLampPoly, backLampColor);
	int backLeftLampCenterRow = (backLeftLampTop + backLeftLampBottom) / 2;
	int backLeftLampCenterCol = (backLeftLampLeft + backLeftLampRight) / 2;
	FloodfillBitmap(carBitmap, backLeftLampCenterRow, backLeftLampCenterCol, backLampColor, tmpArena);

	int backRightLampRight = backHoodBottomRight - 10;
	int backRightLampLeft = backRightLampRight - 7;
	int backRightLampBottom = backHoodBottom;
	int backRightLampTop = backRightLampBottom + 2;
	int backRightLampPoly[] = {
		backRightLampTop,    backRightLampLeft,
		backRightLampTop,    backRightLampRight,
		backRightLampBottom, backRightLampRight,
		backRightLampBottom, backRightLampLeft
	};
	DrawBitmapPolyOutline(carBitmap, 4, backRightLampPoly, backLampColor);
	int backRightLampCenterRow = (backRightLampTop + backRightLampBottom) / 2;
	int backRightLampCenterCol = (backRightLampLeft + backRightLampRight) / 2;
	FloodfillBitmap(carBitmap, backRightLampCenterRow, backRightLampCenterCol, backLampColor, tmpArena);
}