#pragma once

#include "BuildingType.h"
#include "Geometry.h"
#include "MapElem.h"
#include "Point.h"
#include "Renderer.h"
#include "Road.h"

extern float connectRoadWidth;
extern float entranceWidth;
extern float wallWidth;

struct MapElem;

struct WallHelper {
	int maxWallCount;
	int wallCount;
	Line* walls;
	bool* hasDoor;
	Line* doors;
};

enum CrossType {
	CrossNone,
	CrossWall,
	CrossEntrance
};

struct BuildingCrossInfo {
	Building* building;
	Point crossPoint;
	Point corner1;
	Point corner2;
	CrossType type;
};

struct BuildingInside {
	int wallCount;
	Line* walls;
};

struct Building {
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

	BuildingInside* inside;
};

void GenerateBuildingInside(Building* building, WallHelper* wallHelper);

void ConnectBuildingToElem(Building* building, MapElem elem);
bool IsPointInBuilding(Point point, Building building);
bool IsPointInExtBuilding(Point point, Building building, float radius);
bool IsBuildingCrossed(Building building, Point point1, Point point2);
BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float extRadius, Point closePoint, Point farPoint);
BuildingCrossInfo ExtBuildingInsideClosestCrossInfo(Building* building, float extRadius, Point closePoint, Point farPoint);
Point ClosestBuildingCrossPoint(Building building, Point closePoint, Point farPoint);

bool IsPointOnBuildingConnector(Point point, Building building);

void HighlightBuilding(Renderer renderer, Building building, Color color);
void DrawBuilding(Renderer renderer, Building building);
void DrawBuildingInside(Renderer renderer, Building building);

void DrawVisibleAreaInBuilding(Renderer renderer, Building building, Point center, FillHelper fillHelper);

void HighlightBuildingConnector(Renderer renderer, Building building, Color color);
void DrawConnectRoad(Renderer renderer, Building building);