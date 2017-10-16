#pragma once
#include "Bitmap.h"
#include "Intersection.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"

struct Vehicle {
	Point position;
	Color color;

	float length;
	float width;

	float speed;
	float maxSpeed;
	
	int orientationx;
	int orientationy;

	Intersection *targetIntersection;
	Road *onRoad;
	int onRoadTarget;
	float onRoadLength;

	Intersection *onIntersection;

	Map *map;
	IntersectionPathHelper *pathHelper;

	void update(float seconds);
	void draw(Bitmap bitmap);
};