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

// TODO: change endPointIndex to laneIndex when there are more lanes for a direction
DirectedPoint RoadLeavePoint(Road road, int endPointIndex) {
	DirectedPoint result = {};

	Point startPoint = {};
	Point endPoint = {};

	if (endPointIndex == 1) {
		startPoint = road.endPoint2;
		endPoint = road.endPoint1;
	}
	else {
		startPoint = road.endPoint1;
		endPoint = road.endPoint2;
	}

	Point moveUnitVector = PointDirection(startPoint, endPoint);
	Point sideUnitVector = Point{-moveUnitVector.y, moveUnitVector.x};

	result.position = PointSum(endPoint, PointProd(road.width * 0.25f, sideUnitVector));
	result.direction = moveUnitVector;

	return result;
}

DirectedPoint RoadEnterPoint(Road road, int endPointIndex) {
	DirectedPoint result = {};

	Point startPoint = {};
	Point endPoint = {};

	if (endPointIndex == 1) {
		startPoint = road.endPoint1;
		endPoint = road.endPoint2;
	}
	else {
		startPoint = road.endPoint2;
		endPoint = road.endPoint1;
	}

	Point moveUnitVector = PointDirection(startPoint, endPoint);
	Point sideUnitVector = Point{-moveUnitVector.y, moveUnitVector.x};

	result.position = PointSum(startPoint, PointProd(road.width * 0.25f, sideUnitVector));
	result.direction = moveUnitVector;

	return result;
}

// TODO: introduce road angle and use it for calculations
float DistanceSquareFromRoad(Road road, Point point) {
	Point closest = ClosestRoadPoint(road, point);

	return DistanceSquare(point, closest);
}

Point ClosestRoadPoint(Road road, Point point) {
	Point result = {};

	if (road.endPoint1.x == road.endPoint2.x) {
		float minY = road.endPoint1.y;
		float maxY = road.endPoint2.y;

		if (minY > maxY) {
			float tmp = minY;
			minY = maxY;
			maxY = tmp;
		}

		result.x = road.endPoint1.x;

		if (minY <= point.y && point.y <= maxY)
			result.y = point.y;
		else if (point.y < minY)
			result.y = minY;
		else
			result.y = maxY;
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		float minX = road.endPoint1.x;
		float maxX = road.endPoint2.x;

		if (minX > maxX) {
			float tmp = minX;
			minX = maxX;
			maxX = tmp;
		}

		result.y = road.endPoint1.y;

		if (minX <= point.x && point.x <= maxX)
			result.x = point.x;
		else if (point.x < minX)
			result.x = minX;
		else
			result.y = maxX;
	}

	return result;
}

Point ClosestLanePoint(Road road, int laneIndex, Point point) {
	Point result = {};

	if (road.endPoint1.x == road.endPoint2.x) {
		float x = road.endPoint1.x;

		if (road.endPoint1.y < road.endPoint2.y) {
			if (laneIndex > 0)
				x -= road.width * 0.25f;
			else
				x += road.width * 0.25f;
		}
		else {
			if (laneIndex > 0)
				x += road.width * 0.25f;
			else
				x -= road.width * 0.25f;
		}

		result.x = x;
		result.y = point.y;
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		float y = road.endPoint1.y;

		if (road.endPoint1.x < road.endPoint2.x) {
			if (laneIndex > 0)
				y += road.width * 0.25f;
			else
				y -= road.width * 0.25f;
		}
		else {
			if (laneIndex > 0)
				y -= road.width * 0.25f;
			else
				y -= road.width * 0.25f;
		}

		result.x = point.x;
		result.y = y;
	}

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

static inline void DrawRoadSide(Renderer renderer, Road road) {
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

void DrawRoad(Renderer renderer, Road road) {
	DrawRoadSide(renderer, road);

	float stripeWidth = road.width / 20.0f;

	float top = 0.0f;
	float left = 0.0f;
	float bottom = 0.0f;
	float right = 0.0f;

	float stripeTop = 0.0f;
	float stripeLeft = 0.0f;
	float stripeBottom = 0.0f;
	float stripeRight = 0.0f;

	if (road.endPoint1.x == road.endPoint2.x) {
		left   = road.endPoint1.x - (road.width / 2.0f);
		right  = road.endPoint2.x + (road.width / 2.0f);
		top    = road.endPoint1.y;
		bottom = road.endPoint2.y;

		stripeLeft   = road.endPoint1.x - (stripeWidth / 2.0f);
		stripeRight  = road.endPoint2.x + (stripeWidth / 2.0f);
		stripeTop    = road.endPoint1.y;
		stripeBottom = road.endPoint2.y;
	}
	else if (road.endPoint1.y == road.endPoint2.y) {
		left   = road.endPoint1.x;
		right  = road.endPoint2.x;
		top    = road.endPoint1.y - (road.width / 2.0f);
		bottom = road.endPoint2.y + (road.width / 2.0f);

		stripeLeft   = road.endPoint1.x;
		stripeRight  = road.endPoint2.x;
		stripeTop    = road.endPoint1.y - (stripeWidth / 2.0f);
		stripeBottom = road.endPoint2.y + (stripeWidth / 2.0f);
	}

	Color roadColor = { 0.5f, 0.5f, 0.5f };
	DrawRect(renderer, top, left, bottom, right, roadColor);

	Color stripeColor = { 1.0f, 1.0f, 1.0f };
	DrawRect(renderer, stripeTop, stripeLeft, stripeBottom, stripeRight, stripeColor);
}