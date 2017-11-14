#pragma once

#include "Renderer.h"
#include "Point.h"
#include "Road.h"
#include "Map.h"
#include "MapElem.h"

struct MapElem;

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

	MapElem connectElem;

	Color color;

	void ConnectTo(MapElem elem);

	bool IsPointInside(Point point);
	bool IsCrossed(Point point1, Point point2);
	Point ClosestCrossPoint(Point closePoint, Point farPoint);

	void HighLight(Renderer renderer, Color color);
	void Draw(Renderer renderer);
};