#pragma once

#include "BuildingType.hpp"
#include "Geometry.hpp"
#include "Memory.hpp"
#include "Point.hpp"
#include "Renderer.hpp"
#include "Road.hpp"

extern float connectRoadWidth;
extern float entranceWidth;
extern float wallWidth;

struct GameAssets;

enum CrossType {
	CrossNone,
	CrossWall,
	CrossEntrance
};

struct Building;
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
	// TODO: remove this
	BuildingType type;

	// TODO: save four corner points instead
	float top;
	float left;
	float bottom;
	float right;

	float height;

	bool roadAround;

	Point connectPointClose;
	Point connectPointFarShow;
	Point connectPointFar;

	Point entrancePoint1;
	Point entrancePoint2;

	int connectTreeHeight;

	Road* connectRoad;

	BuildingInside* inside;
};

void GenerateBuildingInside(Building* building, MemArena* arena, MemArena* tmpArena);

void ConnectBuildingToRoad(Building* building, Road* road);
bool IsPointInBuilding(Point point, Building building);
bool IsPointInExtBuilding(Point point, Building building, float radius);
bool IsBuildingCrossed(Building building, Point point1, Point point2);
BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, float extRadius, Point closePoint, Point farPoint);
BuildingCrossInfo ExtBuildingInsideClosestCrossInfo(Building* uilding, float extRadius, Point closePoint, Point farPoint);
Point ClosestBuildingCrossPoint(Building uilding, Point closePoint, Point farPoint);

bool IsPointOnBuildingConnector(Point point, Building building);

void HighlightBuilding(Renderer renderer, Building building, Color color);
void DrawBuilding(Renderer renderer, Building building, GameAssets* assets);
void DrawBuildingInside(Renderer renderer, Building building);

void DrawVisibleAreaInBuilding(Renderer renderer, Building building, Point center, MemArena* tmpArena);

void HighlightBuildingConnector(Renderer renderer, Building building, Color color);
void DrawConnectRoad(Renderer renderer, Building building, Texture roadTexture);