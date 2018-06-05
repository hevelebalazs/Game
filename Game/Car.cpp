#include "Car.h"
#include "Geometry.h"
#include "Math.h"

extern float MinCarSpeed = 5.0f;
extern float MaxCarSpeed = 10.0f;

#define CarBitmapWidth 140
#define CarBitmapHeight 300

static inline Quad GetCarCorners(Car* car)
{
	Quad result = {};

	Point addWidth = PointProd((car->width * 0.5f), RotationVector(car->angle + PI * 0.5f));

	Point side1 = PointSum(car->position, addWidth);
	Point side2 = PointDiff(car->position, addWidth);

	Point addLength = PointProd((car->length * 0.5f), RotationVector(car->angle));

	result.points[0] = PointSum(side1, addLength);
	result.points[1] = PointDiff(side1, addLength);
	result.points[2] = PointDiff(side2, addLength); 
	result.points[3] = PointSum(side2, addLength);
	return result;
}

Quad GetCarStopArea(Car* car)
{
	Point toFrontUnitVector = RotationVector(car->angle);
	Point toRightUnitVector = RotationVector(car->angle + PI * 0.5f);

	Point addWidth = PointProd(car->width * 0.5f, toRightUnitVector);

	Point side1 = PointSum(car->position, addWidth);
	Point side2 = PointDiff(car->position, addWidth);

	float stopDistance = 5.0f;

	Point addClose = PointProd(car->length * 0.5f, toFrontUnitVector);
	Point addFar = PointProd(car->length * 0.5f + stopDistance, toFrontUnitVector);

	Quad result = {};
	result.points[0] = PointSum(side1, addClose);
	result.points[1] = PointSum(side2, addClose);
	result.points[2] = PointSum(side2, addFar); 
	result.points[3] = PointSum(side1, addFar);

	return result;
}

bool IsCarOnPoint(Car* car, Point point)
{
	Quad carCorners = GetCarCorners(car);
	bool result = IsPointInQuad(carCorners, point);
	return result;
}

void MoveCar(Car* car, DirectedPoint point)
{
	car->position = point.position;
	car->angle = VectorAngle(point.direction);
}

void DrawCar(Renderer renderer, Car car)
{
	Assert(car.bitmap != 0);
	DrawScaledRotatedBitmap(renderer, car.bitmap, car.position, car.width, car.length, car.angle);
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

void InitAutoCarMovement(AutoCar* autoCar)
{
	autoCar->moveBezier4 = TurnBezier4(autoCar->moveStartPoint, autoCar->moveEndPoint);

	// TODO: is this distance close enough?
	float moveDistance = Distance(autoCar->moveStartPoint.position, autoCar->moveEndPoint.position);

	if (autoCar->car.moveSpeed > 0.0f)
		autoCar->moveTotalSeconds = (moveDistance / autoCar->car.moveSpeed);
	else
		autoCar->moveTotalSeconds = 0.0f;
	autoCar->moveSeconds = 0.0f;
}

void MoveAutoCarToJunction(AutoCar* autoCar, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	MapElem targetElem = JunctionElem(autoCar->onJunction);
	MapElem nextElem = JunctionElem(junction);

	autoCar->moveNode = ConnectElems(autoCar->car.map, targetElem, nextElem, tmpArena, pathPool);

	if (autoCar->moveNode) {
		autoCar->moveStartPoint = StartNodePoint(autoCar->moveNode);
		autoCar->moveEndPoint = NextNodePoint(autoCar->moveNode, autoCar->moveStartPoint);

		InitAutoCarMovement(autoCar);

		autoCar->moveTargetJunction = junction;
	}
}

void UpdateAutoCar(AutoCar* autoCar, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool)
{
	Car* car = &autoCar->car;

	if (car->moveSpeed == 0.0f)
		return;

	if (autoCar->moveTargetJunction) {
		// TODO: should there be a limit on the iteration number?
		while (seconds > 0.0f) {
			PathNode* moveNode = autoCar->moveNode;

			if (!moveNode) {
				autoCar->onJunction = autoCar->moveTargetJunction;
				autoCar->moveTargetJunction = 0;
				break;
			}

			if (moveNode) {
				bool stop = false;

				if (IsNodeEndPoint(moveNode, autoCar->moveEndPoint)) {
					PathNode* nextNode = moveNode->next;

					if (nextNode) {
						MapElem moveElem = moveNode->elem;
						MapElem nextElem = nextNode->elem;

						if (moveElem.type == MapElemRoad && nextElem.type == MapElemJunction) {
							Road* road = moveElem.road;
							Junction* junction = nextElem.junction;
							TrafficLight* trafficLight = 0;

							for (int i = 0; i < junction->roadN; ++i) {
								if (junction->roads[i] == road)
									trafficLight = &junction->trafficLights[i];
							}

							if (trafficLight && (trafficLight->color == TrafficLightRed || trafficLight->color == TrafficLightYellow))
								stop = true;
						}
					}
				}

				if (stop) {
					// TODO: use distance square here?
					float distanceLeft = Distance(car->position, autoCar->moveEndPoint.position);

					// TODO: introduce a "stopDistance" variable?
					if (distanceLeft < car->length * 0.5f)
						break;
				}

				autoCar->moveSeconds += seconds;

				if (autoCar->moveSeconds >= autoCar->moveTotalSeconds) {
					autoCar->moveStartPoint = autoCar->moveEndPoint;

					seconds = autoCar->moveSeconds - autoCar->moveTotalSeconds;

					if (IsNodeEndPoint(moveNode, autoCar->moveStartPoint)) {
						moveNode = moveNode->next;

						FreePathNode(autoCar->moveNode, pathPool);
						autoCar->moveNode = moveNode;

						if (!moveNode)
							continue;
					}
					else {
						autoCar->moveEndPoint = NextNodePoint(moveNode, autoCar->moveStartPoint);

						InitAutoCarMovement(autoCar);
					}
				}
				else {
					seconds = 0.0f;

					float moveRatio = (autoCar->moveSeconds / autoCar->moveTotalSeconds);

					DirectedPoint position = Bezier4DirectedPoint(autoCar->moveBezier4, moveRatio);
					MoveCar(car, position);
				}
			}
		}
	} else {
		Junction* targetJunction = RandomJunction(*car->map);
		MoveAutoCarToJunction(autoCar, targetJunction, arena, tmpArena, pathPool);
	}
}

void UpdatePlayerCar(PlayerCar* playerCar, float seconds)
{
	Car* car = &playerCar->car;

	float speed = VectorLength(playerCar->velocity);

	Point direction = RotationVector(car->angle);

	Point frontWheel = PointProd(+car->length * 0.5f, direction);
	Point rearWheel  = PointProd(-car->length * 0.5f, direction);

	float maxControlSpeed = 4.0f;
	float controlTurnAngle = PI * 0.6f;

	float turnAngle = controlTurnAngle * playerCar->turnDirection;
	if (speed > maxControlSpeed)
		turnAngle *= (maxControlSpeed / speed);
	
	bool backwards = false;
	if (DotProduct(direction, playerCar->velocity) < 0.0)
		backwards = true;

	Point turnDirection = {};
	if (backwards)
		turnDirection = RotationVector(car->angle - turnAngle);
    else
		turnDirection = RotationVector(car->angle + turnAngle);

	frontWheel = PointSum(frontWheel, PointProd(seconds * speed, turnDirection));
	rearWheel  = PointSum(rearWheel, PointProd(seconds * speed, direction));

	car->angle = atan2f(frontWheel.y - rearWheel.y, frontWheel.x - rearWheel.x);

	float cDrag = 0.4257f;
	float cRR = 24.8f;

	Point fTraction = PointProd(playerCar->engineForce, direction);
	Point fDrag = PointProd(-cDrag * speed, playerCar->velocity);
	Point fRR = PointProd(-cRR, playerCar->velocity);

	Point force = PointSum(PointSum(fTraction, fDrag), fRR);

	force = PointProd((1.0f / 50.0f), force);

	playerCar->velocity = PointSum(playerCar->velocity, PointProd(seconds, force));

	Point parallel = PointProd(DotProduct(playerCar->velocity, direction), direction);
	Point perpendicular = PointDiff(playerCar->velocity, parallel);

	playerCar->velocity = PointSum(parallel, PointProd(0.5f, perpendicular));

	car->position = PointSum(car->position, PointProd(seconds, playerCar->velocity));
}