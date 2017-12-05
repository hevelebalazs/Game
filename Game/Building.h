#pragma once

#include "BuildingType.h"
#include "Geometry.h"
#include "MapElem.h"
#include "Point.h"
#include "Renderer.h"
#include "Road.h"

extern float entranceWidth;
extern float wallWidth;

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

struct Room {
	float left;
	float right;
	float top;
	float bottom;
};

struct BuildingInside {
	int wallCount;
	Line* walls;

	int doorCount;
	Line* doors;

	// TODO: create a Room struct?
	int roomCount;
	Room* rooms;

	Line entrance;
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

	BuildingInside* inside;
};

void GenerateBuildingInside(Building* building);
Room* GetRoom(Building* building, Point position);

void ConnectBuildingToElem(Building* building, MapElem elem);
bool IsPointInBuilding(Point point, Building building);
bool IsBuildingCrossed(Building building, Point point1, Point point2);
BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float extRadius, Point closePoint, Point farPoint);
Point ClosestBuildingCrossPoint(Building building, Point closePoint, Point farPoint);

void HighLightBuilding(Renderer renderer, Building building, Color color);
void DrawBuilding(Renderer renderer, Building building);
void DrawBuildingInside(Renderer renderer, Building building);
void HighLightRoom(Renderer renderer, Room room, Color color);
void DrawConnectRoad(Renderer renderer, Building building);