#pragma once

#include "BuildingType.h"
#include "Renderer.h"
#include "Point.h"
#include "Road.h"
#include "MapElem.h"

struct MapElem;

struct BuildingCrossInfo {
	Building* building;
	Point crossPoint;
	Point corner1;
	Point corner2;
};

struct Building {
	static float connectRoadWidth;

	BuildingType type;

	// TODO: save four corner points instead
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

	void ConnectTo(MapElem elem);

	bool IsPointInside(Point point);
	bool IsCrossed(Point point1, Point point2);
	BuildingCrossInfo ExtClosestCrossInfo(Point closePoint, Point farPoint, float radius);
	Point ClosestCrossPoint(Point closePoint, Point farPoint);

	void HighLight(Renderer renderer, Color color);
	void Draw(Renderer renderer);
};