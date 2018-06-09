#pragma once

#include "Bitmap.hpp"
#include "Building.hpp"
#include "BuildingType.hpp"
#include "MapElem.hpp"
#include "Memory.hpp"
#include "Renderer.hpp"

extern Color GrassColor;

struct GameAssets;

struct Map {
	Junction* junctions;
	int junctionCount;

	Road* roads;
	int roadCount;

	Building* buildings;
	int buildingCount;
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

void DrawGroundElems(Renderer renderer, Map* map);
void DrawBuildings(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);
void DrawMap(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);