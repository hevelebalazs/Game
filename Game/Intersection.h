#pragma once

#include "Road.h"

struct Road;

struct Intersection {
	Point coordinate;
	Road* leftRoad = 0;
	Road* rightRoad = 0;
	Road* topRoad = 0;
	Road* bottomRoad = 0;

	void highlight(Bitmap bitmap, Color color);
	void draw(Bitmap bitmap);
};