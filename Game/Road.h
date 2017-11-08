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

	Point EnterPoint(int endPointIndex);
	Point LeavePoint(int endPointIndex);

	float DistanceSquareFrom(Point point);
	Point ClosestPoint(Point point);

	Intersection* OtherIntersection(Intersection*);

	float width;
	
	void Draw(Renderer renderer);
};