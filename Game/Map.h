#pragma once

#include "Bitmap.h"
#include "Building.h"
#include "BuildingType.h"
#include "MapElem.h"
#include "Memory.h"
#include "Renderer.h"

struct GameAssets;

struct Map {
	Junction* junctions;
	int junctionCount = 0;

	Road* roads;
	int roadCount = 0;

	Building* buildings;
	int buildingCount = 0;

	float width = 0;
	float height = 0;
};

Junction* RandomJunction(Map map);
Junction* GetJunctionAtPoint(Map* map, Point point);
Junction* JunctionAtPoint(Map map, Point point, float maxDistance);

MapElem ClosestRoadOrJunction(Map map, Point point);

Building* RandomBuilding(Map map);
Building* ClosestBuilding(Map map, Point point, BuildingType type);
BuildingCrossInfo ClosestExtBuildingCrossInfo(Map map, float radius, Point closePoint, Point farPoint);
Building* ClosestCrossedBuilding(Map map, Point point1, Point point2, Building* excludedBuilding);
Building* BuildingAtPoint(Map map, Point point);

MapElem RoadElemAtPoint(Map map, Point point);
MapElem PedestrianElemAtPoint(Map map, Point point);

void DrawGroundElems(Renderer renderer, Map* map, GameAssets* assets);
void DrawBuildings(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);
void DrawMap(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);