#include <stdio.h>
#include <math.h>

#include "Geometry.h"
#include "Math.h"
#include "Point.h"
#include "Road.h"

// TODO: use this everywhere
extern Color roadColor = {0.5f, 0.5f, 0.5f};

extern Color sideWalkColor = {0.4f, 0.4f, 0.4f};
extern float sideWalkWidth = 3.0f;
extern float crossingWidth = 10.0f;

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadLeavePoint(Road road, int endPointIndex) {
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position = XYFromPositiveRoadCoord(&road, 0.0f, -road.width * 0.25f);
	}
	else {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position = XYFromNegativeRoadCoord(&road, 0.0f, -road.width * 0.25f);
	}

	return result;
}

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadEnterPoint(Road road, int endPointIndex) {
	DirectedPoint result = {};

	if (endPointIndex == 1) {
		result.direction = PointDirection(road.endPoint1, road.endPoint2);
		result.position = XYFromPositiveRoadCoord(&road, 0.0f, road.width * 0.25f);
	}
	else {
		result.direction = PointDirection(road.endPoint2, road.endPoint1);
		result.position = XYFromNegativeRoadCoord(&road, 0.0f, road.width * 0.25f);
	}

	return result;
}

// TODO: introduce road angle and use it for calculations
float DistanceSquareFromRoad(Road road, Point point) {
	Point closest = ClosestRoadPoint(road, point);

	return DistanceSquare(point, closest);
}

Point ClosestRoadPoint(Road road, Point point) {
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

Point ClosestLanePoint(Road road, int laneIndex, Point point) {
	Point pointRoadCoord = PointToRoadCoord(&road, point, laneIndex);
	pointRoadCoord.y = 0.25f * road.width;
	Point result = PointFromRoadCoord(&road, pointRoadCoord, laneIndex);
	return result;
}

bool IsPointOnRoad(Point point, Road road) {
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (left == right) {
		left  -= road.width * 0.5f;
		right += road.width * 0.5f;
	}

	if (top == bottom) {
		top    -= road.width * 0.5f;
		bottom += road.width * 0.5f;
	}

	if (point.x < left || point.x > right)
		return false;
	if (point.y < top || point.y > bottom)
		return false;

	return true;
}

bool IsPointOnRoadSidewalk(Point point, Road road) {
	Point pointRoadCoord = PointToPositiveRoadCoord(&road, point);

	float roadLength = RoadLength(&road);
	bool result = IsBetween(pointRoadCoord.x, 0.0f, roadLength)
		&& IsBetween(Abs(pointRoadCoord.y), road.width * 0.5f, road.width * 0.5f + sideWalkWidth);

	return result;
}

// TODO: this is a bad idea because of floating point inaccuracies
bool IsPointOnRoadSide(Point point, Road road) {
	bool result = false;

	if (road.endPoint1.x == road.endPoint2.x)
		result = ((point.x == road.endPoint1.x - road.width * 0.5f) || (point.x == road.endPoint1.x + road.width * 0.5f));
	else if (road.endPoint1.y == road.endPoint2.y)
		result = ((point.y == road.endPoint1.y - road.width * 0.5f) || (point.y == road.endPoint1.y + road.width * 0.5f));

	return result;
}

void HighlightRoad(Renderer renderer, Road road, Color color) {
	float left = Min2(road.endPoint1.x, road.endPoint2.x);
	float right = Max2(road.endPoint1.x, road.endPoint2.x);
	float top = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (left == right) {
		left -= road.width * 0.5f;
		right += road.width * 0.5f;
	}

	if (top == bottom) {
		top -= road.width * 0.5f;
		bottom += road.width * 0.5f;
	}

	DrawRect(renderer, top, left, bottom, right, color);
}

int LaneIndex(Road road, Point point) {
	int result = 0;
	bool turnsRight = TurnsRight(road.endPoint1, road.endPoint2, point);

	if (turnsRight)
		result = 1;
	else
		result = -1;

	return result;
}

Point LaneDirection(Road road, int laneIndex) {
	Point result = {};

	if (laneIndex == 1)
		result = PointDirection(road.endPoint1, road.endPoint2);
	else if (laneIndex == -1)
		result = PointDirection(road.endPoint2, road.endPoint1);
	
	return result;
}

// TODO: can DistanceOnLane and TurnPointFromLane be merged?
float DistanceOnLane(Road road, int laneIndex, Point point) {
	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	float length = VectorLength(parallelVector);

	return length;
}

DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point) {
	DirectedPoint result = {};

	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointDiff(
		PointSum(startPoint.position, parallelVector),
		PointProd(road.width * 0.25f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point) {
		DirectedPoint result = {};

	DirectedPoint startPoint = RoadEnterPoint(road, laneIndex);
	Point vector = PointDiff(point, startPoint.position);

	Point parallelVector = ParallelVector(vector, startPoint.direction);

	result.position = PointSum(
		PointSum(startPoint.position, parallelVector),
		PointProd(road.width * 0.25f, startPoint.direction)
	);
	result.direction = startPoint.direction;

	return result;
}

void HighlightRoadSidewalk(Renderer renderer, Road road, Color color) {
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	if (road.endPoint1.x == road.endPoint2.x) {
		left  -= (road.width * 0.5f);
		right += (road.width * 0.5f);

		DrawRect(renderer, top, left - sideWalkWidth, bottom, left, color);
		DrawRect(renderer, top, right, bottom, right + sideWalkWidth, color);
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		top    -= (road.width * 0.5f);
		bottom += (road.width * 0.5f);

		DrawRect(renderer, top - sideWalkWidth, left, top, right, color);
		DrawRect(renderer, bottom, left, bottom + sideWalkWidth, right, color);
	}
}

static inline void DrawRoadSidewalk(Renderer renderer, Road road) {
	float left   = Min2(road.endPoint1.x, road.endPoint2.x);
	float right  = Max2(road.endPoint1.x, road.endPoint2.x);
	float top    = Min2(road.endPoint1.y, road.endPoint2.y);
	float bottom = Max2(road.endPoint1.y, road.endPoint2.y);

	float sideWidth = (road.width * 0.5f + sideWalkWidth);

	if (road.endPoint1.x == road.endPoint2.x) {
		DrawRect(renderer, top, left - sideWidth, bottom, left, sideWalkColor);
		DrawRect(renderer, top, right, bottom, right + sideWidth, sideWalkColor);
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		DrawRect(renderer, top - sideWidth, left, top, right, sideWalkColor);
		DrawRect(renderer, bottom, left, bottom + sideWidth, right, sideWalkColor);
	}
}

// TODO: pass Road by pointer?
static inline void DrawCrossing(Renderer renderer, Road road) {
	Color stepColor = Color{1.0f, 1.0f, 1.0f};
	float stepSize = 2.0f;
	float stepDistance = -road.width * 0.5f;
	bool drawStep = true;

	while (stepDistance < road.width * 0.5f) {
		float newStepDistance = stepDistance + stepSize;
		if (newStepDistance > road.width * 0.5f)
			newStepDistance = road.width * 0.5f;

		if (drawStep) {
			Point point1 = XYFromPositiveRoadCoord(&road, road.crossingDistance - crossingWidth * 0.5f, stepDistance);
			Point point2 = XYFromPositiveRoadCoord(&road, road.crossingDistance + crossingWidth * 0.5f, newStepDistance);
			DrawRect(renderer, point1.y, point1.x, point2.y, point2.x, stepColor);
		}

		drawStep = !drawStep;
		stepDistance = newStepDistance;
	}
}

void DrawRoad(Renderer renderer, Road road) {
	DrawRoadSidewalk(renderer, road);
	float roadLength = RoadLength(&road);

	Color roadColor = Color{0.5f, 0.5f, 0.5f};
	Point roadPoint1 = XYFromPositiveRoadCoord(&road, 0.0f, -road.width * 0.5f);
	Point roadPoint2 = XYFromPositiveRoadCoord(&road, roadLength, road.width * 0.5f);
	DrawRect(renderer, roadPoint1.y, roadPoint1.x, roadPoint2.y, roadPoint2.x, roadColor);
	DrawRect(renderer, roadPoint1.y, roadPoint1.x, roadPoint2.y, roadPoint2.x, roadColor);

	Color stripeColor = Color{1.0f, 1.0f, 1.0f};
	float stripeWidth = road.width / 20.0f;

	Point stripePoint1 = XYFromPositiveRoadCoord(&road, 0.0f, -stripeWidth * 0.5f);
	Point stripePoint2 = XYFromPositiveRoadCoord(&road, road.crossingDistance - crossingWidth * 0.5f, stripeWidth * 0.5f);
	DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);

	stripePoint1 = XYFromPositiveRoadCoord(&road, road.crossingDistance + crossingWidth * 0.5f, -stripeWidth * 0.5f);
	stripePoint2 = XYFromPositiveRoadCoord(&road, roadLength, stripeWidth * 0.5f);
	DrawRect(renderer, stripePoint1.y, stripePoint1.x, stripePoint2.y, stripePoint2.x, stripeColor);

	DrawCrossing(renderer, road);
}