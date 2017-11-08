#pragma once
#include "Renderer.h"
#include "Point.h"

struct Building {
	static float connectRoadWidth;

	float top;
	float left;
	float bottom;
	float right;

	bool roadAround;

	Point connectRoad;
	Point connectBuilding;

	Color color;

	bool IsCrossed(Point point1, Point point2);
	Point ClosestCrossPoint(Point closePoint, Point farPoint);

	void Draw(Renderer renderer);
};