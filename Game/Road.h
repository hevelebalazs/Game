#pragma once

#include "Point.h"
#include "Renderer.h"
#include "Intersection.h"
#include <Windows.h>

struct Intersection;

struct Road{
	Point endPoint1;
	Point endPoint2;

	Intersection* intersection1;
	Intersection* intersection2;

	float width;
};

float DistanceSquareFromRoad(Road road, Point point);
Point ClosestRoadPoint(Road road, Point point);

void DrawRoad(Renderer renderer, Road road);