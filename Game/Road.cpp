#include <stdio.h>
#include <math.h>

#include "Game.h"
#include "Geometry.h"
#include "Math.h"
#include "Point.h"
#include "Road.h"

extern Color RoadColor       = {0.5f, 0.5f, 0.5f};
extern float LaneWidth       = 7.0f;

extern Color RoadStripeColor = {1.0f, 1.0f, 1.0f};
extern float RoadStripeWidth = 0.5f;

extern Color SidewalkColor   = {0.4f, 0.4f, 0.4f};
extern float SidewalkWidth   = 3.0f;
extern float CrossingWidth   = 10.0f;

extern float TrafficLightRadius = 2.0f;
extern float TrafficLightSwitchTime = 3.0f;
extern float TrafficLightYellowTime = 1.0f;

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadLeavePoint(Road road, int endPointIndex)
{
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position  = FromRoadCoord1(road, 0.0f, -LaneWidth * 0.5f); 
	} else {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position  = FromRoadCoord2(road, 0.0f, +LaneWidth * 0.5f);
	}

	return result;
}

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadEnterPoint(Road road, int endPointIndex)
{
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position  = FromRoadCoord1(road, 0.0f, +LaneWidth * 0.5f);
	} else {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position  = FromRoadCoord2(road, 0.0f, -LaneWidth * 0.5f);
	}

	return result;
}

// TODO: introduce road angle and use it for calculations
float DistanceSquareFromRoad(Road road, Point point)
{
	Point closest = ClosestRoadPoint(road, point);
	float result = DistanceSquare(point, closest);
	return result;
}

Point ClosestRoadPoint(Road road, Point point)
{
	Point result = {};
	Point resultRoadCoord = {};
	Point pointRoadCoord = ToRoadCoord(road, point);

	float roadLength = RoadLength(&road);
	if (pointRoadCoord.x < 0.0f)
		resultRoadCoord.x = 0.0f;
	else if (pointRoadCoord.x < roadLength)
		resultRoadCoord.x = pointRoadCoord.x;
	else
		resultRoadCoord.x = roadLength;

	resultRoadCoord.y = 0.0f;

	result = FromRoadCoord(road, resultRoadCoord);
	return result;
}

Point ClosestLanePoint(Road road, int laneIndex, Point point)
{
	Point pointRoadCoord = ToRoadCoord(road, point);
	if (laneIndex > 0)
		pointRoadCoord.y = +LaneWidth * 0.5f;
	else
		pointRoadCoord.y = -LaneWidth * 0.5f;
	Point result = FromRoadCoord(road, pointRoadCoord);
	return result;
}

// [R1]: Test function!
bool IsPointOnRoad(Point point, Road road)
{
	Point roadCoord = ToRoadCoord(road, point);
	bool result = false;
	float roadLength = RoadLength(&road);
	if (IsBetween(roadCoord.x, 0.0f, roadLength)) {
		if (IsBetween(roadCoord.y, -LaneWidth, +LaneWidth))
			result = true;
	}
	return result;
}

bool IsPointOnRoadSidewalk(Point point, Road road)
{
	Point pointRoadCoord = ToRoadCoord(road, point);

	float roadLength = RoadLength(&road);
	bool result = IsBetween(pointRoadCoord.x, 0.0f, roadLength) 
		       && IsBetween(Abs(pointRoadCoord.y), LaneWidth, LaneWidth + SidewalkWidth);

	return result;
}

// TODO: this is a bad idea because of floating point inaccuracies
// [R1]: Where is this called from?
bool IsPointOnRoadSide(Point point, Road road)
{
	bool result = false;

	if (road.endPoint1.x == road.endPoint2.x)
		result = ((point.x == road.endPoint1.x - LaneWidth) || (point.x == road.endPoint1.x + LaneWidth));
	else if (road.endPoint1.y == road.endPoint2.y)
		result = ((point.y == road.endPoint1.y - LaneWidth) || (point.y == road.endPoint1.y + LaneWidth));

	return result;
}

// [R1]: Test function!
void HighlightRoad(Renderer renderer, Road road, Color color)
{
	float side = LaneWidth + SidewalkWidth;
	float length = RoadLength(&road);
	Point rBL = Point{0.0f,   -side};
	Point rBR = Point{0.0f,   +side};
	Point rTL = Point{length, -side};
	Point rTR = Point{length, +side};

	Point pBL = FromRoadCoord(road, rBL);
	Point pBR = FromRoadCoord(road, rBR);
	Point pTL = FromRoadCoord(road, rTL);
	Point pTR = FromRoadCoord(road, rTR);

	Quad quad = {pBR, pBL, pTL, pTR};
	DrawQuad(renderer, quad, color);
}

int LaneIndex(Road road, Point point)
{
	int result = 0;
	bool turnsRight = TurnsRight(road.endPoint1, road.endPoint2, point);

	if (turnsRight)
		result = 1;
	else
		result = -1;

	return result;
}

Point LaneDirection(Road road, int laneIndex)
{
	Point result = {};

	if (laneIndex == 1)
		result = PointDirection(road.endPoint1, road.endPoint2);
	else if (laneIndex == -1)
		result = PointDirection(road.endPoint2, road.endPoint1);
	
	return result;
}

// TODO: can DistanceOnLane and TurnPointFromLane be merged?
float DistanceOnLane(Road road, int laneIndex, Point point)
{
	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	float length = VectorLength(parallelVector);

	return length;
}

DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point)
{
	DirectedPoint result = {};

	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointDiff(
		PointSum(startPoint.position, parallelVector),
		PointProd(LaneWidth * 0.5f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point)
{
		DirectedPoint result = {};

	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointSum(
		PointSum(startPoint.position, parallelVector),
		PointProd(LaneWidth * 0.5f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

void HighlightRoadSidewalk(Renderer renderer, Road road, Color color)
{
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (road.endPoint1.x == road.endPoint2.x) {
		left  -= LaneWidth;
		right += LaneWidth;

		DrawRect(renderer, top, left - SidewalkWidth, bottom, left, color);
		DrawRect(renderer, top, right, bottom, right + SidewalkWidth, color);
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		top    -= LaneWidth;
		bottom += LaneWidth;

		DrawRect(renderer, top - SidewalkWidth, left, top, right, color);
		DrawRect(renderer, bottom, left, bottom + SidewalkWidth, right, color);
	}
}

static void DrawRoadSidewalk(Renderer renderer, Road road, Texture sidewalkTexture)
{
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	float sideWidth = (LaneWidth + SidewalkWidth);

	Color separatorColor = {0.2f, 0.2f, 0.2f};
	if (road.endPoint1.x == road.endPoint2.x) {
		// DrawRect(renderer, top, left - sideWidth, bottom, left, SidewalkColor);
		WorldTextureRect(renderer, top, left - sideWidth, bottom, left, sidewalkTexture);
		// DrawRect(renderer, top, right, bottom, right + sideWidth, SidewalkColor);
		WorldTextureRect(renderer, top, right, bottom, right + sideWidth, sidewalkTexture);

		Point topLeft     = Point{left  - sideWidth, top    + SidewalkWidth};
		Point topRight    = Point{right + sideWidth, top    + SidewalkWidth};
		Point bottomLeft  = Point{left  - sideWidth, bottom - SidewalkWidth};
		Point bottomRight = Point{right + sideWidth, bottom - SidewalkWidth};

		/*
		Bresenham(renderer, topLeft, bottomLeft, separatorColor);
		Bresenham(renderer, topRight, bottomRight, separatorColor);
		*/
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		// DrawRect(renderer, top - sideWidth, left, top, right, SidewalkColor);
		WorldTextureRect(renderer, top - sideWidth, left, top, right, sidewalkTexture);
		// DrawRect(renderer, bottom, left, bottom + sideWidth, right, SidewalkColor);
		WorldTextureRect(renderer, bottom, left, bottom + sideWidth, right, sidewalkTexture);

		Point topLeft     = Point{left  + SidewalkWidth, top    - sideWidth};
		Point topRight    = Point{right - SidewalkWidth, top    - sideWidth};
		Point bottomLeft  = Point{left  + SidewalkWidth, bottom + sideWidth};
		Point bottomRight = Point{right - SidewalkWidth, bottom + sideWidth};

		/*
		Bresenham(renderer, topLeft, topRight, separatorColor);
		Bresenham(renderer, bottomLeft, bottomRight, separatorColor);
		*/
	}
}

// TODO: pass Road by pointer?
inline void DrawCrossing(Renderer renderer, Road road, Texture stripeTexture)
{
	Color stepColor = Color{1.0f, 1.0f, 1.0f};
	float stepSize = 2.0f;
	float stepDistance = -LaneWidth;
	bool drawStep = true;

	while (stepDistance < LaneWidth) {
		float newStepDistance = stepDistance + stepSize;
		if (newStepDistance > LaneWidth)
			newStepDistance = LaneWidth;

		if (drawStep) {
			Point point1 = FromRoadCoord1(road, road.crossingDistance - CrossingWidth * 0.5f, stepDistance);
			Point point2 = FromRoadCoord1(road, road.crossingDistance + CrossingWidth * 0.5f, newStepDistance);
			// DrawRect(renderer, point1.y, point1.x, point2.y, point2.x, stepColor);
			WorldTextureRect(renderer, point1.y, point1.x, point2.y, point2.x, stripeTexture);
		}

		drawStep = !drawStep;
		stepDistance = newStepDistance;
	}
}

void DrawRoad(Renderer renderer, Road road, GameAssets* assets)
{
	DrawRoadSidewalk(renderer, road, assets->sidewalkTexture);
	float roadLength = RoadLength(&road);

	Color roadColor = Color{0.5f, 0.5f, 0.5f};
	Point roadPoint1 = FromRoadCoord1(road, 0.0f, -LaneWidth);
	Point roadPoint2 = FromRoadCoord1(road, roadLength, LaneWidth);
	// DrawRect(renderer, roadPoint1.y, roadPoint1.x, roadPoint2.y, roadPoint2.x, roadColor);
	WorldTextureRect(renderer, roadPoint1.y, roadPoint1.x, roadPoint2.y, roadPoint2.x, assets->roadTexture);

	Color separatorColor = {0.2f, 0.2f, 0.2f};
	float left   = Min2(roadPoint1.x, roadPoint2.x);
	float right  = Max2(roadPoint1.x, roadPoint2.x);
	float top    = Min2(roadPoint1.y, roadPoint2.y);
	float bottom = Max2(roadPoint1.y, roadPoint2.y);

	Point topLeft     = Point{left, top};
	Point bottomLeft  = Point{left, bottom};
	Point topRight    = Point{right, top};
	Point bottomRight = Point{right, bottom};
	
	/*
	if (road.endPoint1.x == road.endPoint2.x) {
		// TODO: make version of Bresenham that go one pixel to the left/right/top/bottom
		Bresenham(renderer, topLeft, bottomLeft, separatorColor);
		Bresenham(renderer, topRight, bottomRight, separatorColor);
	} else if (road.endPoint1.y == road.endPoint2.y) {
		Bresenham(renderer, topLeft, topRight, separatorColor);
		Bresenham(renderer, bottomLeft, bottomRight, separatorColor);
	}
	*/

	Color stripeColor = Color{1.0f, 1.0f, 1.0f};
	float stripeWidth = LaneWidth * 0.1f;

	Point stripePoint1 = FromRoadCoord1(road, 0.0f, -stripeWidth * 0.5f);
	Point stripePoint2 = FromRoadCoord1(road, road.crossingDistance - CrossingWidth * 0.5f, stripeWidth * 0.5f);
	// DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);
	WorldTextureRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, assets->stripeTexture);

	stripePoint1 = FromRoadCoord1(road, road.crossingDistance + CrossingWidth * 0.5f, -stripeWidth * 0.5f);
	stripePoint2 = FromRoadCoord1(road, roadLength, stripeWidth * 0.5f);
	// DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);
	WorldTextureRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, assets->stripeTexture);

	DrawCrossing(renderer, road, assets->stripeTexture);
}

float RoadLength(Road* road)
{
	return Distance(road->endPoint1, road->endPoint2);
}

void GenerateCrossing(Road* road)
{
	float roadLength = RoadLength(road);
	road->crossingDistance = RandomBetween(CrossingWidth * 0.5f, roadLength - CrossingWidth * 0.5f);
}

bool IsPointOnCrossing(Point point, Road road)
{
	Point pointRoadCoord = ToRoadCoord(road, point);

	bool result = (
		(Abs(pointRoadCoord.x - road.crossingDistance) <= CrossingWidth * 0.5f) &&
		(Abs(pointRoadCoord.y) <= LaneWidth)
	);

	return result;
}

int RoadSidewalkIndex(Road road, Point point)
{
	int result = 0;
	Point pointRoadCoord = ToRoadCoord(road, point);
	if (pointRoadCoord.y < -LaneWidth)
		result = -1;
	else if (pointRoadCoord.y > LaneWidth)
		result = 1;

	return result;
}

Point FromRoadCoord1(Road road, float along, float side)
{
	Point roadDirection = PointDirection(road.endPoint1, road.endPoint2);
	Point sideDirection = TurnVectorToRight(roadDirection);

	Point addPoint = PointSum(
		PointProd(along, roadDirection),
		PointProd(side, sideDirection)
	);

	Point result = PointSum(road.endPoint1, addPoint);
	return result;
}

Point FromRoadCoord2(Road road, float along, float side)
{
	Point roadDirection = PointDirection(road.endPoint1, road.endPoint2);
	Point sideDirection = TurnVectorToRight(roadDirection);

	Point addPoint = PointSum(
		PointProd(along, roadDirection),
		PointProd(side, sideDirection)
	);

	Point result = PointSum(road.endPoint2, addPoint);
	return result;
}

Point FromRoadCoord(Road road, Point roadCoord1) 
{
	Point result = FromRoadCoord1(road, roadCoord1.x, roadCoord1.y);
	return result;
}

Point ToRoadCoord(Road road, Point point)
{
	Point roadDirection = PointDirection(road.endPoint1, road.endPoint2);	
	Point xy = PointDiff(point, road.endPoint1);
	Point result = XYToBase(xy, roadDirection);
	return result;
}

void DrawRoad(Renderer renderer, Road road)
{
	DrawLine(renderer, road.endPoint1, road.endPoint2, RoadColor, 2.0f * LaneWidth);
	DrawLine(renderer, road.endPoint1, road.endPoint2, RoadStripeColor, RoadStripeWidth);

	Point side1 = FromRoadCoord1(road, 0.0, -LaneWidth - SidewalkWidth * 0.5f);
	Point side2 = FromRoadCoord2(road, 0.0, -LaneWidth - SidewalkWidth * 0.5f);
	DrawLine(renderer, side1, side2, SidewalkColor, SidewalkWidth);

	side1 = FromRoadCoord1(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	side2 = FromRoadCoord2(road, 0.0f, +LaneWidth + SidewalkWidth * 0.5f);
	DrawLine(renderer, side1, side2, SidewalkColor, SidewalkWidth);
}

// Junction
int RandomQuarterIndex()
{
	return 1 + (rand() % 4);
}

TrafficLight* TrafficLightOfRoad(Junction* junction, Road* road)
{
	if (junction->leftRoad == road) 
		return &junction->leftTrafficLight;
	else if (junction->rightRoad == road)
		return &junction->rightTrafficLight;
	else if (junction->topRoad == road)
		return &junction->topTrafficLight;
	else if (junction->bottomRoad == road)
		return &junction->bottomTrafficLight;
	else 
		return 0;
}

bool IsPointOnJunction(Point point, Junction junction)
{
	float roadWidth = 2.0f * LaneWidth;
	
	float left   = junction.position.x - (roadWidth * 0.5f);
	float right  = junction.position.x + (roadWidth * 0.5f);
	float top    = junction.position.y - (roadWidth * 0.5f);
	float bottom = junction.position.y + (roadWidth * 0.5f);

	// TODO: create an IsPointInRect function?
	if (point.x < left || point.x > right) 
		return false;

	if (point.y < top || point.y > bottom) 
		return false;

	return true;
}

bool IsPointOnJunctionSidewalk(Point point, Junction junction)
{
	float roadWidth = 2.0f * LaneWidth;

	float left   = junction.position.x - roadWidth * 0.5f;
	float right  = junction.position.x + roadWidth * 0.5f;
	float top    = junction.position.y - roadWidth * 0.5f;
	float bottom = junction.position.y + roadWidth * 0.5f;

	if (!junction.topRoad) {
		if (IsPointInRect(point, left, right, top - SidewalkWidth, top))
			return true;
	}
	if (!junction.bottomRoad) {
		if (IsPointInRect(point, left, right, bottom, bottom + SidewalkWidth))
			return true;
	}
	if (!junction.leftRoad) {
		if (IsPointInRect(point, left - SidewalkWidth, left, top, bottom))
			return true;
	}
	if (!junction.rightRoad) {
		if (IsPointInRect(point, right, right + SidewalkWidth, top, bottom))
			return true;
	}

	float roadDistance = roadWidth * 0.5f;
	float sidewalkDistance = roadDistance + SidewalkWidth;
	float absX = Abs(point.x - junction.position.x);
	float absY = Abs(point.y - junction.position.y);

	if (IsBetween(absX, roadDistance, sidewalkDistance) && IsBetween(absY, roadDistance, sidewalkDistance))
		return true;

	return false;
}

static void StartTrafficLight(TrafficLight* trafficLight)
{
	trafficLight->color = TrafficLightGreen;
	trafficLight->timeLeft = TrafficLightSwitchTime;
}

static void UpdateTrafficLight(TrafficLight* trafficLight, float seconds)
{
	if (trafficLight->color == TrafficLightGreen) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->timeLeft += TrafficLightYellowTime;
			trafficLight->color = TrafficLightYellow;
		}
	}
	else if (trafficLight->color == TrafficLightYellow) {
		trafficLight->timeLeft -= seconds;

		if (trafficLight->timeLeft < 0.0f) {
			trafficLight->color = TrafficLightRed;
		}
	}
}

static void DrawTrafficLight(Renderer renderer, TrafficLight trafficLight)
{
	Color drawColor = {};

	switch (trafficLight.color) {
		case TrafficLightGreen: {
			drawColor = {0.0f, 1.0f, 0.0f}; 
			break;
		}
		case TrafficLightYellow: {
			drawColor = {1.0f, 1.0f, 0.0f};
			break;
		}
		case TrafficLightRed: {
			drawColor = {1.0f, 0.0f, 0.0f};
			break;
		}
	}

	Point position = trafficLight.position;
	float radius = TrafficLightRadius;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		drawColor
	);
}

void InitTrafficLights(Junction* junction)
{
	int roadCount = 0;
	if (junction->leftRoad)
		roadCount++;
	if (junction->rightRoad)
		roadCount++;
	if (junction->topRoad)
		roadCount++;
	if (junction->bottomRoad) 
		roadCount++;

	if (roadCount <= 2) 
		return;

	float roadWidth = 2.0f * LaneWidth;
	Point position = junction->position;

	if (junction->leftRoad)
		junction->leftTrafficLight.position = {
			position.x - roadWidth * 0.5f,  
			position.y + roadWidth * 0.25f
		};
	if (junction->rightRoad)
		junction->rightTrafficLight.position = {
			position.x + roadWidth * 0.5f,
			position.y - roadWidth * 0.25f
		};
	if (junction->topRoad)
		junction->topTrafficLight.position = {
			position.x - roadWidth * 0.25f, 
			position.y - roadWidth * 0.5f
		};
	if (junction->bottomRoad)
		junction->bottomTrafficLight.position = {
			position.x + roadWidth * 0.25f, 
			position.y + roadWidth * 0.5f
		};

	if (junction->leftRoad)
		junction->leftTrafficLight.color = TrafficLightRed;
	if (junction->rightRoad)
		junction->rightTrafficLight.color = TrafficLightRed;
	if (junction->topRoad)
		junction->topTrafficLight.color = TrafficLightRed;
	if (junction->bottomRoad) 
		junction->bottomTrafficLight.color = TrafficLightRed;

	if (junction->leftRoad)
		StartTrafficLight(&junction->leftTrafficLight);
	else if (junction->topRoad)
		StartTrafficLight(&junction->topTrafficLight);
	else if (junction->rightRoad)
		StartTrafficLight(&junction->rightTrafficLight);
	else if (junction->bottomRoad)
		StartTrafficLight(&junction->bottomTrafficLight);
}

void UpdateTrafficLights(Junction* junction, float seconds)
{
	int roadCount = 0;
	if (junction->leftRoad)
		roadCount++;
	if (junction->rightRoad)
		roadCount++;
	if (junction->topRoad)
		roadCount++;
	if (junction->bottomRoad)
		roadCount++;

	if (roadCount <= 2)
		return;

	// TODO: introduce an "active" member?
	// TODO: this is total nuts, update this to arrays as soon as possible!
	if (junction->leftRoad && junction->leftTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&junction->leftTrafficLight, seconds);
		if (junction->leftTrafficLight.color == TrafficLightRed) {
			if (junction->topRoad)
				StartTrafficLight(&junction->topTrafficLight);
			else if (junction->rightRoad)
				StartTrafficLight(&junction->rightTrafficLight);
			else if (junction->bottomRoad)
				StartTrafficLight(&junction->bottomTrafficLight);
		}
	}
	else if (junction->topRoad && junction->topTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&junction->topTrafficLight, seconds);
		if (junction->topTrafficLight.color == TrafficLightRed) {
			if (junction->rightRoad)
				StartTrafficLight(&junction->rightTrafficLight);
			else if (junction->bottomRoad)
				StartTrafficLight(&junction->bottomTrafficLight);
			else if (junction->leftRoad)
				StartTrafficLight(&junction->leftTrafficLight);
		}
	}
	else if (junction->rightRoad && junction->rightTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&junction->rightTrafficLight, seconds);
		if (junction->rightTrafficLight.color == TrafficLightRed) {
			if (junction->bottomRoad)
				StartTrafficLight(&junction->bottomTrafficLight);
			else if (junction->leftRoad)
				StartTrafficLight(&junction->leftTrafficLight);
			else if (junction->topRoad)
				StartTrafficLight(&junction->topTrafficLight);
		}
	}
	else if (junction->bottomRoad && junction->bottomTrafficLight.color != TrafficLightRed) {
		UpdateTrafficLight(&junction->bottomTrafficLight, seconds);
		if (junction->bottomTrafficLight.color == TrafficLightRed) {
			if (junction->leftRoad)
				StartTrafficLight(&junction->leftTrafficLight);
			else if (junction->topRoad)
				StartTrafficLight(&junction->topTrafficLight);
			else if (junction->rightRoad)
				StartTrafficLight(&junction->rightTrafficLight);
		}
	}
}

void DrawTrafficLights(Renderer renderer, Junction junction)
{
	int roadCount = 0;
	if (junction.leftRoad)
		roadCount++;
	if (junction.rightRoad)
		roadCount++;
	if (junction.topRoad)
		roadCount++;
	if (junction.bottomRoad)
		roadCount++;

	if (roadCount <= 2)
		return;

	if (junction.leftRoad)
		DrawTrafficLight(renderer, junction.leftTrafficLight);
	if (junction.rightRoad)
		DrawTrafficLight(renderer, junction.rightTrafficLight);
	if (junction.topRoad)
		DrawTrafficLight(renderer, junction.topTrafficLight);
	if (junction.bottomRoad)
		DrawTrafficLight(renderer, junction.bottomTrafficLight);
}

void HighlightJunction(Renderer renderer, Junction junction, Color color)
{
	Point center = junction.position;
	float side = LaneWidth + SidewalkWidth;
	Quad quad = {
		Point{center.x + side, center.y - side},
		Point{center.x + side, center.y + side},
		Point{center.x - side, center.y + side},
		Point{center.x - side, center.y - side}
	};
	DrawQuad(renderer, quad, color);
}

void HighlightJunctionSidewalk(Renderer renderer, Junction junction, Color color)
{
	float roadWidth = 2.0f * LaneWidth;
	Point position = junction.position;

	float left   = position.x - roadWidth * 0.5f;
	float right  = position.x + roadWidth * 0.5f;
	float top    = position.y - roadWidth * 0.5f;
	float bottom = position.y + roadWidth * 0.5f;

	if (!junction.topRoad)
		DrawRect(renderer, top - SidewalkWidth, left, top, right, color);

	if (!junction.bottomRoad)
		DrawRect(renderer, bottom, left, bottom + SidewalkWidth, right, color);

	if (!junction.topRoad)
		top -= SidewalkWidth;
	if (!junction.bottomRoad)
		bottom += SidewalkWidth;

	if (!junction.leftRoad)
		DrawRect(renderer, top, left - SidewalkWidth, bottom, left, color);
	
	if (!junction.rightRoad)
		DrawRect(renderer, top, right, bottom, right + SidewalkWidth, color);
}

static void DrawJunctionSidewalk(Renderer renderer, Junction junction, Texture sidewalkTexture)
{
	float roadWidth = 2.0f * LaneWidth;
	Point position = junction.position;

	float left   = position.x - roadWidth * 0.5f;
	float right  = position.x + roadWidth * 0.5f;
	float top    = position.y - roadWidth * 0.5f;
	float bottom = position.y + roadWidth * 0.5f;

	if (!junction.topRoad)
		// DrawRect(renderer, top - sidewalkWidth, left, top, right, sideWalkColor);
		WorldTextureRect(renderer, top - SidewalkWidth, left, top, right, sidewalkTexture);

	if (!junction.bottomRoad)
		// DrawRect(renderer, bottom, left, bottom + sidewalkWidth, right, sideWalkColor);
		WorldTextureRect(renderer, bottom, left, bottom + SidewalkWidth, right, sidewalkTexture);

	if (!junction.topRoad)
		top -= SidewalkWidth;
	if (!junction.bottomRoad)
		bottom += SidewalkWidth;

	if (!junction.leftRoad)
		// DrawRect(renderer, top, left - sidewalkWidth, bottom, left, sideWalkColor);
		WorldTextureRect(renderer, top, left - SidewalkWidth, bottom, left, sidewalkTexture);
	
	if (!junction.rightRoad)
		// DrawRect(renderer, top, right, bottom, right + sidewalkWidth, sideWalkColor);
		WorldTextureRect(renderer, top, right, bottom, right + SidewalkWidth, sidewalkTexture);
}

void DrawJunction(Renderer renderer, Junction junction, GameAssets* assets)
{
	DrawJunctionSidewalk(renderer, junction, assets->sidewalkTexture);

	float roadWidth = 2.0f * LaneWidth;
	Point coordinate = junction.position;

	float top    = coordinate.y - (roadWidth / 2.0f);
	float bottom = coordinate.y + (roadWidth / 2.0f);
	float left   = coordinate.x - (roadWidth / 2.0f);
	float right  = coordinate.x + (roadWidth / 2.0f);

	float midX = (left + right) / 2;
	float midY = (top + bottom) / 2;

	Color color = { 0.5f, 0.5f, 0.5f };
	// DrawRect(renderer, top, left, bottom, right, color);
	WorldTextureRect(renderer, top, left, bottom, right, assets->roadTexture);

	float stripeWidth = roadWidth / 20.0f;
	Color stripeColor = { 1.0f, 1.0f, 1.0f };

	int roadCount = 0;

	if (junction.topRoad)
		roadCount++;
	if (junction.leftRoad)
		roadCount++;
	if (junction.bottomRoad)
		roadCount++;
	if (junction.rightRoad)
		roadCount++;

	if (roadCount > 2) {
		if (junction.topRoad)
			// DrawRect(renderer, top, left, top + stripeWidth, right, stripeColor);
			WorldTextureRect(renderer, top, left, top + stripeWidth, right, assets->stripeTexture);
		if (junction.leftRoad)
			// DrawRect(renderer, top, left, bottom, left + stripeWidth, stripeColor);
			WorldTextureRect(renderer, top, left, bottom, left + stripeWidth, assets->stripeTexture);
		if (junction.bottomRoad)
			// DrawRect(renderer, bottom - stripeWidth, left, bottom, right, stripeColor);
			WorldTextureRect(renderer, bottom - stripeWidth, left, bottom, right, assets->stripeTexture);
		if (junction.rightRoad)
			// DrawRect(renderer, top, right - stripeWidth, bottom, right, stripeColor);
			WorldTextureRect(renderer, top, right - stripeWidth, bottom, right, assets->stripeTexture);
	}
	else {
		if (junction.topRoad)
			// DrawRect(renderer, top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
			WorldTextureRect(renderer, top, midX - (stripeWidth * 0.5f), midY + (stripeWidth * 0.5f), midX + (stripeWidth * 0.5f), assets->stripeTexture);
		if (junction.leftRoad)
			// DrawRect(renderer, midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
			WorldTextureRect(renderer, midY - (stripeWidth * 0.5f), left, midY + (stripeWidth * 0.5f), midX + (stripeWidth * 0.5f), assets->stripeTexture);
		if (junction.bottomRoad)
			// DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
			WorldTextureRect(renderer, midY - (stripeWidth * 0.5f), midX - (stripeWidth * 0.5f), bottom, midX + (stripeWidth * 0.5f), assets->stripeTexture);
		if (junction.rightRoad)
			// DrawRect(renderer, midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
			WorldTextureRect(renderer, midY - (stripeWidth * 0.5f), midX - (stripeWidth * 0.5f), midY + (stripeWidth * 0.5f), right, assets->stripeTexture);
	}
}

void DrawJunction(Renderer renderer, Junction junction)
{
	Point center = junction.position;
	float side = LaneWidth + SidewalkWidth;
	Quad quad = {
		Point{center.x + side, center.y - side},
		Point{center.x + side, center.y + side},
		Point{center.x - side, center.y + side},
		Point{center.x - side, center.y - side}
	};
	DrawQuad(renderer, quad, SidewalkColor);

	side = LaneWidth;
	quad = {
		Point{center.x + side, center.y - side},
		Point{center.x + side, center.y + side},
		Point{center.x - side, center.y + side},
		Point{center.x - side, center.y - side}
	};
	DrawQuad(renderer, quad, RoadColor);
}

int QuarterIndex(Junction* junction, Point point)
{
	int result = 0;

	if (point.y < junction->position.y) {
		if (point.x < junction->position.x)
			result = QuarterTopLeft;
		else
			result = QuarterTopRight;
	}
	else {
		if (point.x < junction->position.x)
			result = QuarterBottomLeft;
		else
			result = QuarterBottomRight;
	}

	return result;
}

Point JunctionSidewalkCorner(Junction* junction, int quarterIndex)
{
	Point result = junction->position;

	extern float LaneWidth;
	extern float SidewalkWidth;
	float distance = LaneWidth + SidewalkWidth * 0.5f;

	if (quarterIndex == QuarterTopLeft || quarterIndex == QuarterBottomLeft)
		result.x -= distance;
	else
		result.x += distance;

	if (quarterIndex == QuarterTopLeft || quarterIndex == QuarterTopRight)
		result.y -= distance;
	else
		result.y += distance;

	return result;
}