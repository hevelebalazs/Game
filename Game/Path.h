#pragma once
#include "Map.h"
#include "Intersection.h"
#include "Renderer.h"

struct IntersectionPath {
	Intersection** intersections;
	int intersectionCount;
};

struct IntersectionPathHelper {
	int* indexes;
	int* isHelper;
	int* source;
	int count;
};

Road* NextRoadOnPath(Map map, Intersection* start, Intersection* finish, IntersectionPathHelper* pathHelper);
Intersection *NextIntersectionOnPath(Map map, Intersection* start, Intersection* finish, IntersectionPathHelper* pathHelper);
IntersectionPath FindConnectingPath(Map map, Intersection* start, Intersection* finish, IntersectionPathHelper* pathHelper);
void DrawIntersectionPath(IntersectionPath path, Renderer renderer, float pathWidth);