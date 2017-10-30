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

	Map *map;

	void draw(Bitmap bitmap);
};