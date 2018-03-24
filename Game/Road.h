#pragma once

#include "Bitmap.h"
#include "Intersection.h"
#include "Math.h"
#include "Point.h"
#include "Renderer.h"
#include "Texture.h"

extern Color RoadColor;
extern float LaneWidth;
extern Color SideWalkColor;
extern float SideWalkWidth;
extern float CrossingWidth;

struct Intersection;
struct GameAssets;

struct Road {
	Point endPoint1;
	Point endPoint2;

	Intersection* intersection1;
	Intersection* intersection2;

	float crossingDistance;
};

Point PointFromPositiveRoadCoord(Road* road, Point roadCoord);
Point PointFromNegativeRoadCoord(Road* road, Point roadCoord);

float RoadLength(Road* road);
void GenerateCrossing(Road* road);
Point XYFromPositiveRoadCoord(Road* road, float parallel, float perpendicular);
Point XYFromNegativeRoadCoord(Road* road, float parallel, float perpendicular);
Point PointFromNegativeRoadCoord(Road* road, Point roadCoord);
Point PointToPositiveRoadCoord(Road* road, Point xy);
Point PointToNegativeRoadCoord(Road* road, Point xy);
Point PointFromRoadCoord(Road* road, Point roadCoord, int laneIndex);
Point PointToRoadCoord(Road* road, Point point, int laneIndex);

DirectedPoint RoadLeavePoint(Road road, int endPointIndex);
DirectedPoint RoadEnterPoint(Road road, int endPointIndex);

float DistanceSquareFromRoad(Road road, Point point);
Point ClosestRoadPoint(Road road, Point point);
Point ClosestLanePoint(Road road, int laneIndex, Point point);

bool IsPointOnRoad(Point point, Road road);
bool IsPointOnRoadSide(Point point, Road road);
bool IsPointOnRoadSidewalk(Point point, Road road);
bool IsPointOnCrossing(Point point, Road road);

int LaneIndex(Road road, Point point);
Point LaneDirection(Road road, int laneIndex);

int RoadSidewalkIndex(Road road, Point point);

float DistanceOnLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointFromLane(Road road, int laneIndex, Point point);
DirectedPoint TurnPointToLane(Road road, int laneIndex, Point point);

void HighlightRoadSidewalk(Renderer renderer, Road road, Color color);
void HighlightRoad(Renderer renderer, Road road, Color color);
void DrawRoad(Renderer renderer, Road road, GameAssets* assets);
void DrawRoad(Renderer renderer, Road road);