#pragma once

#include "Point.h"
#include "Renderer.h"
#include "Intersection.h"
#include <Windows.h>

struct Intersection;

struct Road{
	Point endPoint1;
	Point endPoint2;

	Intersection *intersection1;
	Intersection *intersection2;

	Point enterPoint(int endPointIndex);
	Point leavePoint(int endPointIndex);

	float distanceSquareFrom(Point point);
	Point closestPoint(Point point);

	Intersection *otherIntersection(Intersection*);

	float width;
	
	void draw(Renderer renderer);
};