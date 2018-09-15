#pragma once

#include "BuildingType.hpp"
#include "Draw.hpp"
#include "Geometry.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Road.hpp"
#include "Type.hpp"

#define ConnectRoadWidth	5.0f
#define EntranceWidth		3.0f
#define WallWidth			0.5f

#define DoorWidth			3.0f
#define MinRoomSide			5.0f
#define MaxRoomSide			20.0f

enum CrossType 
{
	CrossNone,
	CrossWall,
	CrossEntrance
};

struct Building;
struct BuildingCrossInfo 
{
	Building* building;
	V2 crossPoint;
	V2 corner1;
	V2 corner2;
	CrossType type;
};

struct BuildingInside 
{
	I32 wallCount;
	Line* walls;
};

struct Building 
{
	// TODO: remove this
	BuildingType type;

	// TODO: save four corner points instead
	F32 top;
	F32 left;
	F32 bottom;
	F32 right;

	F32 height;

	B32 roadAround;

	V2 connectPointClose;
	V2 connectPointFarShow;
	V2 connectPointFar;

	V2 entrancePoint1;
	V2 entrancePoint2;

	I32 connectTreeHeight;

	Road* connectRoad;

	BuildingInside* inside;
};

void GenerateBuildingInside(Building* building, MemArena* arena, MemArena* tmpArena);

void ConnectBuildingToRoad(Building* building, Road* road);
B32 IsPointInBuilding(V2 point, Building building);
B32 IsPointInExtBuilding(V2 point, Building building, F32 radius);
B32 IsBuildingCrossed(Building building, V2 point1, V2 point2);
BuildingCrossInfo ExtBuildingClosestCrossInfo(Building* building, F32 extRadius, V2 closePoint, V2 farPoint);
BuildingCrossInfo ExtBuildingInsideClosestCrossInfo(Building* uilding, F32 extRadius, V2 closePoint, V2 farPoint);
V2 ClosestBuildingCrossPoint(Building uilding, V2 closePoint, V2 farPoint);

B32 IsPointOnBuildingConnector(V2 point, Building building);

void HighlightBuilding(Canvas canvas, Building building, V4 color);
void DrawBuildingInside(Canvas canvas, Building building);

void DrawVisibleAreaInBuilding(Canvas canvas, Building building, V2 center, MemArena* tmpArena);

void HighlightBuildingConnector(Canvas canvas, Building building, V4 color);
void DrawConnectRoad(Canvas canvas, Building building, Texture roadTexture);
