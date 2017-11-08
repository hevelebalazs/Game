#pragma once

#include "Road.h"
#include "Renderer.h"

struct Road;

struct Intersection {
	Point coordinate;
	Road* leftRoad = 0;
	Road* rightRoad = 0;
	Road* topRoad = 0;
	Road* bottomRoad = 0;

	float GetRoadWidth();

	void Highlight(Renderer renderer, Color color);
	void Draw(Renderer renderer);
};