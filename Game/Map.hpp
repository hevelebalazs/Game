#pragma once

#include "Bitmap.hpp"
#include "Building.hpp"
#include "BuildingType.hpp"
#include "Memory.hpp"
#include "Renderer.hpp"

extern Color GrassColor;

struct GameAssets;

struct Map {
	Junction* junctions;
	int junctionN;

	Road* roads;
	int roadN;

	Building* buildings;
	int buildingN;
};

enum MapElemType {
	MapElemNone,
	MapElemRoad,
	MapElemRoadSidewalk,
	MapElemCrossing,
	MapElemJunction,
	MapElemJunctionSidewalk,
	MapElemBuilding,
	MapElemBuildingConnector
};

struct MapElem {
	MapElemType type;

	union {
		Building* building;
		Junction* junction;
		Road* road;
		void* address;
	};
};

Junction* GetRandomJunction(Map* map);
Junction* GetJunctionAtPoint(Map* map, Point point);

MapElem GetClosestRoadElem(Map* map, Point point);

Building* GetRandomBuilding(Map* map);
Building* GetClosestBuilding(Map* map, Point point, BuildingType type);
BuildingCrossInfo GetClosestExtBuildingCrossInfo(Map* map, float radius, Point closePoint, Point farPoint);
Building* GetClosestCrossedBuilding(Map* map, Point point1, Point point2, Building* excludedBuilding);
Building* GetBuildingAtPoint(Map* map, Point point);

MapElem GetRoadElemAtPoint(Map* map, Point point);
MapElem GetPedestrianElemAtPoint(Map* map, Point point);

void DrawGroundElems(Renderer renderer, Map* map);
void DrawBuildings(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);
void DrawMap(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets);

MapElem GetRoadElem(Road* road);
MapElem GetRoadSidewalkElem(Road* road);
MapElem GetCrossingElem(Road* road);
MapElem GetJunctionElem(Junction* junction);
MapElem GetJunctionSidewalkElem(Junction* junction);
MapElem GetBuildingElem(Building* building);
MapElem GetBuildingConnectorElem(Building* building);
bool AreMapElemsEqual(MapElem elem1, MapElem elem2);
void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color);