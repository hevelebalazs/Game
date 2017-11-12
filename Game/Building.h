#pragma once
#include "Renderer.h"
#include "Point.h"
#include "Road.h"

struct Building {
	static float connectRoadWidth;

	float top;
	float left;
	float bottom;
	float right;

	bool roadAround;

	Point connectPointClose;
	Point connectPointFarShow;
	Point connectPointFar;

	int connectTreeHeight;

	Road* connectRoad;
	Building* connectBuilding;

	Color color;


	Road* GetConnectedRoad();

	bool IsPointInside(Point point);
	bool IsCrossed(Point point1, Point point2);
	Point ClosestCrossPoint(Point closePoint, Point farPoint);

	void HighLight(Renderer renderer, Color color);
	void Draw(Renderer renderer);
};