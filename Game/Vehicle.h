#pragma once
#include "Bitmap.h"
#include "Intersection.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"

struct Vehicle {
	Point position;
	float angle;

	Color color;

	float length;
	float width;

	float maxSpeed;

	Intersection *targetIntersection;

	Intersection *onIntersection;
	Road *nextRoad;

	Road *onRoad;
	Intersection *nextIntersection;

	Point startPoint;
	float startAngle;
	Point targetPoint;
	float targetAngle;

	float spentSeconds;
	float totalSeconds;

	Map *map;
	IntersectionPathHelper *pathHelper;

	void update(float seconds);
	void draw(Bitmap bitmap);
};