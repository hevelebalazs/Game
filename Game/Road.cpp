#include <stdio.h>
#include <math.h>

#include "Game.h"
#include "Geometry.h"
#include "Math.h"
#include "Point.h"
#include "Road.h"

extern Color RoadColor     = {0.5f, 0.5f, 0.5f};
extern float LaneWidth     = 7.0f;
extern Color SideWalkColor = {0.4f, 0.4f, 0.4f};
extern float SideWalkWidth = 3.0f;
extern float CrossingWidth = 10.0f;

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadLeavePoint(Road road, int endPointIndex)
{
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position  = XYFromPositiveRoadCoord(&road, 0.0f, -LaneWidth * 0.5f);
	} else {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position  = XYFromNegativeRoadCoord(&road, 0.0f, -LaneWidth * 0.5f);
	}

	return result;
}

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadEnterPoint(Road road, int endPointIndex)
{
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position = XYFromPositiveRoadCoord(&road, 0.0f, LaneWidth * 0.5f);
	} else {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position = XYFromNegativeRoadCoord(&road, 0.0f, LaneWidth * 0.5f);
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

	Point pointRoadCoord = PointToPositiveRoadCoord(&road, point);

	float roadLength = RoadLength(&road);
	if (pointRoadCoord.x < 0.0f)
		resultRoadCoord.x = 0.0f;
	else if (pointRoadCoord.x < roadLength)
		resultRoadCoord.x = pointRoadCoord.x;
	else
		resultRoadCoord.x = roadLength;

	resultRoadCoord.y = 0.0f;

	result = PointFromPositiveRoadCoord(&road, resultRoadCoord);
	return result;
}

Point ClosestLanePoint(Road road, int laneIndex, Point point)
{
	Point pointRoadCoord = PointToRoadCoord(&road, point, laneIndex);
	pointRoadCoord.y = LaneWidth * 0.5f;
	Point result = PointFromRoadCoord(&road, pointRoadCoord, laneIndex);
	return result;
}

// [R1]: Test function!
bool IsPointOnRoad(Point point, Road road)
{
	Point roadCoord = PointToRoadCoord(&road, point, 1);
	bool result = false;
	float roadLength = RoadLength(&road);
	if ((roadCoord.x >= 0) && (roadCoord.x <= roadLength)) {
		if ((roadCoord.y >= -LaneWidth) && (roadCoord.y <= LaneWidth))
			result = true;
	}
	return result;
}

bool IsPointOnRoadSidewalk(Point point, Road road)
{
	Point pointRoadCoord = PointToPositiveRoadCoord(&road, point);

	float roadLength = RoadLength(&road);
	bool result = IsBetween(pointRoadCoord.x, 0.0f, roadLength) 
		       && IsBetween(Abs(pointRoadCoord.y), LaneWidth, LaneWidth + SideWalkWidth);

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
	float length = RoadLength(&road);
	Point rBL = Point{0.0f,   -LaneWidth};
	Point rBR = Point{0.0f,   +LaneWidth};
	Point rTL = Point{length, -LaneWidth};
	Point rTR = Point{length, +LaneWidth};

	Point pBL = PointFromRoadCoord(&road, rBL, 1);
	Point pBR = PointFromRoadCoord(&road, rBR, 1);
	Point pTL = PointFromRoadCoord(&road, rTL, 1);
	Point pTR = PointFromRoadCoord(&road, rTR, 1);

	Quad quad = {pBL, pBR, pTR, pTL};
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

		DrawRect(renderer, top, left - SideWalkWidth, bottom, left, color);
		DrawRect(renderer, top, right, bottom, right + SideWalkWidth, color);
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		top    -= LaneWidth;
		bottom += LaneWidth;

		DrawRect(renderer, top - SideWalkWidth, left, top, right, color);
		DrawRect(renderer, bottom, left, bottom + SideWalkWidth, right, color);
	}
}

// TODO: rename "sideWalk" to "sidewalk" everywhere
static void DrawRoadSidewalk(Renderer renderer, Road road, Texture sidewalkTexture)
{
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	float sideWidth = (LaneWidth + SideWalkWidth);

	Color separatorColor = {0.2f, 0.2f, 0.2f};
	if (road.endPoint1.x == road.endPoint2.x) {
		// DrawRect(renderer, top, left - sideWidth, bottom, left, sideWalkColor);
		WorldTextureRect(renderer, top, left - sideWidth, bottom, left, sidewalkTexture);
		// DrawRect(renderer, top, right, bottom, right + sideWidth, sideWalkColor);
		WorldTextureRect(renderer, top, right, bottom, right + sideWidth, sidewalkTexture);

		Point topLeft     = Point{left  - sideWidth, top    + SideWalkWidth};
		Point topRight    = Point{right + sideWidth, top    + SideWalkWidth};
		Point bottomLeft  = Point{left  - sideWidth, bottom - SideWalkWidth};
		Point bottomRight = Point{right + sideWidth, bottom - SideWalkWidth};

		/*
		Bresenham(renderer, topLeft, bottomLeft, separatorColor);
		Bresenham(renderer, topRight, bottomRight, separatorColor);
		*/
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		// DrawRect(renderer, top - sideWidth, left, top, right, sideWalkColor);
		WorldTextureRect(renderer, top - sideWidth, left, top, right, sidewalkTexture);
		// DrawRect(renderer, bottom, left, bottom + sideWidth, right, sideWalkColor);
		WorldTextureRect(renderer, bottom, left, bottom + sideWidth, right, sidewalkTexture);

		Point topLeft     = Point{left  + SideWalkWidth, top    - sideWidth};
		Point topRight    = Point{right - SideWalkWidth, top    - sideWidth};
		Point bottomLeft  = Point{left  + SideWalkWidth, bottom + sideWidth};
		Point bottomRight = Point{right - SideWalkWidth, bottom + sideWidth};

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
			Point point1 = XYFromPositiveRoadCoord(&road, road.crossingDistance - CrossingWidth * 0.5f, stepDistance);
			Point point2 = XYFromPositiveRoadCoord(&road, road.crossingDistance + CrossingWidth * 0.5f, newStepDistance);
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
	Point roadPoint1 = XYFromPositiveRoadCoord(&road, 0.0f, -LaneWidth);
	Point roadPoint2 = XYFromPositiveRoadCoord(&road, roadLength, LaneWidth);
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

	Point stripePoint1 = XYFromPositiveRoadCoord(&road, 0.0f, -stripeWidth * 0.5f);
	Point stripePoint2 = XYFromPositiveRoadCoord(&road, road.crossingDistance - CrossingWidth * 0.5f, stripeWidth * 0.5f);
	// DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);
	WorldTextureRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, assets->stripeTexture);

	stripePoint1 = XYFromPositiveRoadCoord(&road, road.crossingDistance + CrossingWidth * 0.5f, -stripeWidth * 0.5f);
	stripePoint2 = XYFromPositiveRoadCoord(&road, roadLength, stripeWidth * 0.5f);
	// DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);
	WorldTextureRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, assets->stripeTexture);

	DrawCrossing(renderer, road, assets->stripeTexture);
}

float RoadLength(Road* road)
{
	return CityDistance(road->endPoint1, road->endPoint2);
}

void GenerateCrossing(Road* road)
{
	float roadLength = RoadLength(road);
	road->crossingDistance = RandomBetween(CrossingWidth * 0.5f, roadLength - CrossingWidth * 0.5f);
}

// TODO: use road coordinates everywhere
Point XYFromPositiveRoadCoord(Road* road, float parallel, float perpendicular)
{
	Point parallelDirection = PointDirection(road->endPoint1, road->endPoint2);
	Point perpendicularDirection = TurnVectorToRight(parallelDirection);

	Point addPoint = PointSum(
		PointProd(parallel, parallelDirection),
		PointProd(perpendicular, perpendicularDirection)
	);

	Point result = PointSum(road->endPoint1, addPoint);
	return result;
}

Point XYFromNegativeRoadCoord(Road* road, float parallel, float perpendicular)
{
	Point parallelDirection = PointDirection(road->endPoint2, road->endPoint1);
	Point perpendicularDirection = TurnVectorToRight(parallelDirection);

	Point addPoint = PointSum(
		PointProd(parallel, parallelDirection),
		PointProd(perpendicular, perpendicularDirection)
	);

	Point result = PointSum(road->endPoint2, addPoint);
	return result;
}

// TODO: keep only this version?
Point PointFromPositiveRoadCoord(Road* road, Point roadCoord)
{
	Point result = XYFromPositiveRoadCoord(road, roadCoord.x, roadCoord.y);
	return result;
}

Point PointFromNegativeRoadCoord(Road* road, Point roadCoord)
{
	Point result = XYFromNegativeRoadCoord(road, roadCoord.x, roadCoord.y);
	return result;
}

// TODO: add directions to road endpoints
Point PointToPositiveRoadCoord(Road* road, Point xy)
{
	Point parallelDirection = PointDirection(road->endPoint1, road->endPoint2);	

	xy = PointDiff(xy, road->endPoint1);
	Point result = XYToBase(xy, parallelDirection);
	return result;
}

// TODO: add directions to road endpoints
Point PointToNegativeRoadCoord(Road* road, Point xy)
{
	Point parallelDirection = PointDirection(road->endPoint2, road->endPoint1);	

	xy = PointDiff(xy, road->endPoint2);
	Point result = XYToBase(xy, parallelDirection);
	return result;
}

Point PointFromRoadCoord(Road* road, Point roadCoord, int laneIndex)
{
	Point result = {};

	if (laneIndex > 0)
		result = PointFromPositiveRoadCoord(road, roadCoord);
	else if (laneIndex < 0)
		result = PointFromNegativeRoadCoord(road, roadCoord);

	return result;
}

Point PointToRoadCoord(Road* road, Point point, int laneIndex)
{
	Point result = {};

	if (laneIndex > 0)
		result = PointToPositiveRoadCoord(road, point);
	else if (laneIndex < 0)
		result = PointToNegativeRoadCoord(road, point);

	return result;
}

bool IsPointOnCrossing(Point point, Road road)
{
	Point pointRoadCoord = PointToPositiveRoadCoord(&road, point);

	bool result = (
		(Abs(pointRoadCoord.x - road.crossingDistance) <= CrossingWidth * 0.5f) &&
		(Abs(pointRoadCoord.y) <= LaneWidth)
	);

	return result;
}

int RoadSidewalkIndex(Road road, Point point)
{
	int result = 0;
	Point pointRoadCoord = PointToPositiveRoadCoord(&road, point);
	if (pointRoadCoord.y < -LaneWidth)
		result = -1;
	else if (pointRoadCoord.y > LaneWidth)
		result = 1;

	return result;
}

void DrawRoad(Renderer renderer, Road road)
{
	Point p1 = XYFromPositiveRoadCoord(&road, 0.0f, -LaneWidth);
	Point p2 = XYFromPositiveRoadCoord(&road, 0.0f, +LaneWidth);
	Point p3 = XYFromNegativeRoadCoord(&road, 0.0f, -LaneWidth);
	Point p4 = XYFromNegativeRoadCoord(&road, 0.0f, +LaneWidth);
	Quad quad = {p1, p4, p3, p2};
	DrawQuad(renderer, quad, RoadColor);
}