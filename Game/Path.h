#pragma once
#include "Map.h"
#include "Intersection.h"

struct IntersectionPath {
	Intersection **intersections;
	int intersectionCount;
};

struct IntersectionPathHelper {
	int *indexes;
	int *isHelper;
	int *source;
	int count;
};

Intersection *nextIntersectionOnPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper);
IntersectionPath findConnectingPath(Map map, Intersection *start, Intersection *finish, IntersectionPathHelper *pathHelper);
void drawIntersectionPath(IntersectionPath path, Bitmap bitmap, float pathWidth);