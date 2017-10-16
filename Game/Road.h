#pragma once

#include "Point.h"
#include "Bitmap.h"
#include "Intersection.h"
#include <Windows.h>

struct Intersection;

struct Road{
	Point endPoint1;
	Point endPoint2;

	Intersection *intersection1;
	Intersection *intersection2;

	Intersection *otherIntersection(Intersection*);

	float width;
	
	void draw(Bitmap bitmap);
};