#pragma once

#include "Intersection.h"
#include "Point.h"
#include "Renderer.h"

struct Intersection;

struct Road {
	Point endPoint1;
	Point endPoint2;

	Intersection* intersection1;
	Intersection* intersection2;

	float width;
};

DirectedPoint RoadLeavePoint(Road road, int endPointIndex);
DirectedPoint RoadEnterPoint(Road road, int endPointIndex);

float DistanceSquareFromRoad(Road road, Point point);
Point ClosestRoadPoint(Road road, Point point);
Point ClosestLanePoint(Road road, int laneIndex, Point point);

bool IsPointOnRoad(Point point, Road road);
bool IsPointOnRoadSide(Point point, Road road);

int LaneIndex(Road road, Point point);
Point LaneDirection(Road road, int laneIndex);
float DistanceOnLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point);

void HighlightRoad(Renderer renderer, Road road, Color color);
void DrawRoad(Renderer renderer, Road road);