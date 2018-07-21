#pragma once

#include "Bitmap.hpp"
#include "Building.hpp"
#include "BuildingType.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Type.hpp"

#define GrassColor	MakeColor(0.0f, 0.5f, 0.0f)

#define MaxCachedMapTileN 64
#define MaxGenerateMapTileWorkListN MaxCachedMapTileN
#define MapTileSide 50.0f
#define MapTileBitmapWidth 1024
#define MapTileBitmapHeight 1024
#define GenerateMapTileWorkThreadN 2

struct MapTileIndex {
	I32 row;
	I32 col;
};

struct CachedMapTile {
	MapTileIndex index;
	Bitmap bitmap;
};

struct MapTextures {
	Texture grassTexture;
	Texture roadTexture;
	Texture sidewalkTexture;
	Texture stripeTexture;
};

struct Map;

struct GenerateMapTileWork {
	Map* map;
	MapTileIndex tileIndex;
	Bitmap* bitmap;
	MapTextures* mapTextures;
};

struct GenerateMapTileWorkList {
	GenerateMapTileWork works[MaxGenerateMapTileWorkListN];
	volatile I32 workDoneN;
	volatile I32 workPushedN;
	HANDLE semaphore;
};

#define MaxDrawMapTileWorkListN MaxCachedMapTileN
#define DrawMapTileWorkThreadN 10

struct DrawMapTileWork {
	Canvas canvas;
	Map* map;
	MapTileIndex tileIndex;
	MapTextures* textures;
};

struct DrawMapTileWorkList {
	DrawMapTileWork works[MaxDrawMapTileWorkListN];
	volatile I32 workN;
	volatile I32 firstWorkToDo;
	HANDLE semaphore;
	HANDLE semaphoreDone;
};

struct Map {
	Junction* junctions;
	I32 junctionN;

	Road* roads;
	I32 roadN;

	Building* buildings;
	I32 buildingN;

	F32 left;
	F32 right;
	F32 top;
	F32 bottom;

	I32 tileRowN;
	I32 tileColN;

	CachedMapTile cachedTiles[MaxCachedMapTileN];
	I32 cachedTileN;
	
	GenerateMapTileWorkList generateTileWorkList;
	DrawMapTileWorkList drawTileWorkList;
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

void GenerateMapTextures(MapTextures* textures, MemArena* tmpArena);

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
void DrawTexturedGroundElems(Canvas canvas, Map* map, MapTextures* textures);
void DrawAllTrafficLights(Canvas, Map* map);

MapElem GetRoadElem(Road* road);
MapElem GetRoadSidewalkElem(Road* road);
MapElem GetCrossingElem(Road* road);
MapElem GetJunctionElem(Junction* junction);
MapElem GetJunctionSidewalkElem(Junction* junction);
MapElem GetBuildingElem(Building* building);
MapElem GetBuildingConnectorElem(Building* building);
B32 AreMapElemsEqual(MapElem elem1, MapElem elem2);
void HighlightMapElem(Canvas canvas, MapElem mapElem, V4 color);

B32 IsMapTileIndexValid(Map* map, MapTileIndex tileIndex);
MapTileIndex GetMapTileIndexContainingPoint(Map* map, V2 point);
B32 IsMapTileCached(Map* map, MapTileIndex tileIndex);
V2 GetMapTileCenter(Map* map, MapTileIndex tileIndex);
CachedMapTile* GetFarthestCachedMapTile(Map* map, V2 point);
void GenerateMapTileBitmap(Map* map, MapTileIndex tileIndex, Bitmap* bitmap, MapTextures* mapTextures);
DWORD WINAPI GenerateMapTileWorkProc(LPVOID parameter);
void PushGenerateMapTileWork(GenerateMapTileWorkList* workList, GenerateMapTileWork work);
void CacheMapTile(Map* map, MapTileIndex tileIndex, MapTextures* textures);
Bitmap* GetCachedTileBitmap(Map* map, MapTileIndex tileIndex);
void DrawMapTile(Canvas canvas, Map* map, MapTileIndex tileIndex, MapTextures* textures);
void DrawVisibleMapTiles(Canvas canvas, Map* map, F32 left, F32 right, F32 top, F32 bottom, MapTextures* textures);

DWORD WINAPI DrawMapTileWorkProc(LPVOID parameter);
void PushDrawMapTileWork(DrawMapTileWorkList* workList, DrawMapTileWork work);