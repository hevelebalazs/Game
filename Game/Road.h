#pragma once

#include "Intersection.h"
#include "Point.h"
#include "Renderer.h"

extern Color roadColor;
extern Color sideWalkColor;
extern float sideWalkWidth;

struct Intersection;

struct Road {
	Point endPoint1;
	Point endPoint2;

	Intersection* intersection1;
	Intersection* intersection2;

	float width;
};

inline float RoadLength(Road* road) {
	return CityDistance(road->endPoint1, road->endPoint2);
}

inline Point XYFromPositiveRoadCoord(Road* road, float parallel, float perpendicular) {
	Point parallelDirection = PointDirection(road->endPoint1, road->endPoint2);
	Point perpendicularDirection = TurnVectorToRight(parallelDirection);

	Point addPoint = PointSum(
		PointProd(parallel, parallelDirection),
		PointProd(perpendicular, perpendicularDirection)
	);

	Point result = PointSum(
		road->endPoint1,
		addPoint
	);

	return result;
}

inline Point XYFromNegativeRoadCoord(Road* road, float parallel, float perpendicular) {
	Point parallelDirection = PointDirection(road->endPoint2, road->endPoint1);
	Point perpendicularDirection = TurnVectorToRight(parallelDirection);

	Point addPoint = PointSum(
		PointProd(parallel, parallelDirection),
		PointProd(perpendicular, perpendicularDirection)
	);

	Point result = PointSum(
		road->endPoint2,
		addPoint
	);

	return result;
}

DirectedPoint RoadLeavePoint(Road road, int endPointIndex);
DirectedPoint RoadEnterPoint(Road road, int endPointIndex);

float DistanceSquareFromRoad(Road road, Point point);
Point ClosestRoadPoint(Road road, Point point);
Point ClosestLanePoint(Road road, int laneIndex, Point point);

bool IsPointOnRoad(Point point, Road road);
bool IsPointOnRoadSide(Point point, Road road);
bool IsPointOnRoadSidewalk(Point point, Road road);

int LaneIndex(Road road, Point point);
Point LaneDirection(Road road, int laneIndex);
float DistanceOnLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point);

void HighlightRoadSidewalk(Renderer renderer, Road road, Color color);
void HighlightRoad(Renderer renderer, Road road, Color color);
void DrawRoad(Renderer renderer, Road road);