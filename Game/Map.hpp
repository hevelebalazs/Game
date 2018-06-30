#pragma once

#include "Bitmap.hpp"
#include "Building.hpp"
#include "BuildingType.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

#define GrassColor	GetColor(0.0f, 0.8f, 0.0f)

struct GameAssets;

struct Map {
	Junction* junctions;
	I32 junctionN;

	Road* roads;
	I32 roadN;

	Building* buildings;
	I32 buildingN;
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
Junction* GetJunctionAtPoint(Map* map, V2 point);

MapElem GetClosestRoadElem(Map* map, V2 point);

Building* GetRandomBuilding(Map* map);
Building* GetClosestBuilding(Map* map, V2 point, BuildingType type);
BuildingCrossInfo GetClosestExtBuildingCrossInfo(Map* map, F32 radius, V2 closePoint, V2 farPoint);
Building* GetClosestCrossedBuilding(Map* map, V2 point1, V2 point2, Building* excludedBuilding);
Building* GetBuildingAtPoint(Map* map, V2 point);

MapElem GetRoadElemAtPoint(Map* map, V2 point);
MapElem GetPedestrianElemAtPoint(Map* map, V2 point);

void DrawGroundElems(Canvas canvas, Map* map);
void DrawTexturedGroundElems(Canvas canvas, Map* map, Texture grassTexture, Texture roadTexture, Texture sidewalkTexture, Texture stripeTexture);
void DrawBuildings(Canvas canvas, Map* map, MemArena* arena, GameAssets* assets);
void DrawMap(Canvas canvas, Map* map, MemArena* arena, GameAssets* assets);

MapElem GetRoadElem(Road* road);
MapElem GetRoadSidewalkElem(Road* road);
MapElem GetCrossingElem(Road* road);
MapElem GetJunctionElem(Junction* junction);
MapElem GetJunctionSidewalkElem(Junction* junction);
MapElem GetBuildingElem(Building* building);
MapElem GetBuildingConnectorElem(Building* building);
B32 AreMapElemsEqual(MapElem elem1, MapElem elem2);
void HighlightMapElem(Canvas canvas, MapElem mapElem, V4 color);
