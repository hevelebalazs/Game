#pragma once

#include "Intersection.h"
#include "Math.h"
#include "Point.h"
#include "Renderer.h"

extern Color roadColor;
extern Color sideWalkColor;
extern float sideWalkWidth;
extern float crossingWidth;

struct Intersection;

struct Road {
	Point endPoint1;
	Point endPoint2;

	Intersection* intersection1;
	Intersection* intersection2;

	float width;

	float crossingDistance;
};

inline float RoadLength(Road* road) {
	return CityDistance(road->endPoint1, road->endPoint2);
}

inline void GenerateCrossing(Road* road) {
	float roadLength = RoadLength(road);
	road->crossingDistance = RandomBetween(crossingWidth * 0.5f, roadLength - crossingWidth * 0.5f);
}

// TODO: use road coordinates everywhere
inline Point XYFromPositiveRoadCoord(Road* road, float parallel, float perpendicular) {
	Point parallelDirection = PointDirection(road->endPoint1, road->endPoint2);
	Point perpendicularDirection = TurnVectorToRight(parallelDirection);

	Point addPoint = PointSum(
		PointProd(parallel, parallelDirection),
		PointProd(perpendicular, perpendicularDirection)
	);

	Point result = PointSum(road->endPoint1, addPoint);
	return result;
}

inline Point XYFromNegativeRoadCoord(Road* road, float parallel, float perpendicular) {
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
inline Point PointFromPositiveRoadCoord(Road* road, Point roadCoord) {
	Point result = XYFromPositiveRoadCoord(road, roadCoord.x, roadCoord.y);
	return result;
}

// TODO: keep only this version?
inline Point PointFromNegativeRoadCoord(Road* road, Point roadCoord) {
	Point result = XYFromNegativeRoadCoord(road, roadCoord.x, roadCoord.y);
	return result;
}

// TODO: add directions to road endpoints
inline Point PointToPositiveRoadCoord(Road* road, Point xy) {
	Point parallelDirection = PointDirection(road->endPoint1, road->endPoint2);	

	xy = PointDiff(xy, road->endPoint1);
	Point result = XYToBase(xy, parallelDirection);
	return result;
}

// TODO: add directions to road endpoints
inline Point PointToNegativeRoadCoord(Road* road, Point xy) {
	Point parallelDirection = PointDirection(road->endPoint2, road->endPoint1);	

	xy = PointDiff(xy, road->endPoint2);
	Point result = XYToBase(xy, parallelDirection);
	return result;
}

inline Point PointFromRoadCoord(Road* road, Point roadCoord, int laneIndex) {
	Point result = {};

	if (laneIndex > 0)
		result = PointFromPositiveRoadCoord(road, roadCoord);
	else if (laneIndex < 0)
		result = PointFromNegativeRoadCoord(road, roadCoord);

	return result;
}

inline Point PointToRoadCoord(Road* road, Point point, int laneIndex) {
	Point result = {};

	if (laneIndex > 0)
		result = PointToPositiveRoadCoord(road, point);
	else if (laneIndex < 0)
		result = PointToNegativeRoadCoord(road, point);

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