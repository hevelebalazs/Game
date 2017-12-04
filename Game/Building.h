#pragma once

#include "BuildingType.h"
#include "Renderer.h"
#include "Point.h"
#include "Road.h"
#include "MapElem.h"

extern float entranceWidth;

struct MapElem;

enum EntranceInfo {
	EntranceNone,
	EntranceOut,
	EntranceIn,
	EntranceSide
};

struct BuildingCrossInfo {
	Building* building;
	Point crossPoint;
	Point corner1;
	Point corner2;
	EntranceInfo entrance;
};

struct Building {
	// TODO: remove global from struct
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

	Point entrancePoint1;
	Point entrancePoint2;

	int connectTreeHeight;

	MapElem connectElem;
};

void ConnectBuildingToElem(Building* building, MapElem elem);
bool IsPointInBuilding(Point point, Building building);
bool IsBuildingCrossed(Building building, Point point1, Point point2);
BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float extRadius, Point closePoint, Point farPoint);
Point ClosestBuildingCrossPoint(Building building, Point closePoint, Point farPoint);

void HighLightBuilding(Renderer renderer, Building building, Color color);
void DrawBuilding(Renderer renderer, Building building);
void DrawConnectRoad(Renderer renderer, Building building);