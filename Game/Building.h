#pragma once
#include "Renderer.h"
#include "Point.h"

struct Building {
	float top;
	float left;
	float bottom;
	float right;

	Point connectRoad;

	Color color;
	void draw(Renderer renderer);
};