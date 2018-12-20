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

struct MapTileIndex 
{
	I32 row;
	I32 col;
};

struct CachedMapTile 
{
	MapTileIndex index;
	Bitmap bitmap;
};

struct MapTextures 
{
	Texture grassTexture;
	Texture roadTexture;
	Texture sidewalkTexture;
	Texture stripeTexture;
};

struct Map;

struct GenerateMapTileWork 
{
	Map* map;
	MapTileIndex tileIndex;
	Bitmap* bitmap;
	MapTextures* mapTextures;
};

struct GenerateMapTileWorkList 
{
	GenerateMapTileWork works[MaxGenerateMapTileWorkListN];
	volatile I32 workDoneN;
	volatile I32 workPushedN;
	HANDLE semaphore;
};

#define MaxDrawMapTileWorkListN MaxCachedMapTileN
#define DrawMapTileWorkThreadN 10

struct DrawMapTileWork 
{
	Canvas canvas;
	Map* map;
	MapTileIndex tileIndex;
	MapTextures* textures;
};

struct DrawMapTileWorkList 
{
	DrawMapTileWork works[MaxDrawMapTileWorkListN];
	volatile I32 workN;
	volatile I32 firstWorkToDo;
	HANDLE semaphore;
	HANDLE semaphoreDone;
};

struct Map 
{
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

enum MapElemType 
{
	MapElemNone,
	MapElemRoad,
	MapElemRoadSidewalk,
	MapElemCrossing,
	MapElemJunction,
	MapElemJunctionSidewalk,
	MapElemBuilding,
	MapElemBuildingConnector
};

struct MapElem 
{
	MapElemType type;

	union 
	{
		Building* building;
		Junction* junction;
		Road* road;
		void* address;
	};
};

static void GenerateMapTextures(MapTextures* textures, MemArena* tmpArena)
{
	textures->grassTexture    = GrassTexture(8, tmpArena);
	textures->roadTexture     = RandomGreyTexture(8, 100, 150);
	textures->sidewalkTexture = RandomGreyTexture(8, 80, 100);
	textures->stripeTexture   = RandomGreyTexture(8, 200, 255);
}

static Junction* GetRandomJunction(Map* map)
{
	I32 junctionIndex = RandMod(map->junctionN);
	Junction* junction = &map->junctions[junctionIndex];
	return junction;
}

static Junction* GetJunctionAtPoint(Map* map, V2 point)
{
	Junction* result = 0;
	for (I32 i = 0; i < map->junctionN; ++i) 
	{
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction)) 
		{
			result = junction;
			break;
		}
	}
	return result;
}

static MapElem GetRoadElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemRoad;
	result.road = road;

	return result;
}

static MapElem GetJunctionElem(Junction* junction)
{
	MapElem result = {};
	result.type = MapElemJunction;
	result.junction = junction;

	return result;
}

static MapElem GetClosestRoadElem(Map* map, V2 point)
{
	MapElem result = {};

	Road* closestRoad = 0;
	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->roadN; ++i) 
	{
		Road* road = &map->roads[i];

		B32 betweenX =
			((road->endPoint1.x <= point.x) && (point.x <= road->endPoint2.x)) ||
			((road->endPoint2.x <= point.x) && (point.x <= road->endPoint1.x));
		B32 betweenY =
			((road->endPoint1.y <= point.y) && (point.y <= road->endPoint2.y)) ||
			((road->endPoint2.y <= point.y) && (point.y <= road->endPoint1.y));

		if (betweenX || betweenY) 
		{
			F32 distanceSquare = DistanceSquareFromRoad(road, point);
			if (!closestRoad || distanceSquare < minDistanceSquare) 
			{
				closestRoad = road;
				minDistanceSquare = distanceSquare;

				result = GetRoadElem(road);
			}
		}
	}

	Junction* closestJunction = 0;
	for (I32 i = 0; i < map->junctionN; ++i) 
	{
		Junction* junction = &map->junctions[i];

		B32 betweenX = (Abs(junction->position.x - point.x) <= LaneWidth);
		B32 betweenY = (Abs(junction->position.y - point.y) <= LaneWidth);

		if (betweenX || betweenY) 
		{
			F32 distanceSquare = DistanceSquare(junction->position, point);
			if ((!closestRoad && !closestJunction) || distanceSquare < minDistanceSquare) 
			{
				closestJunction = junction;
				minDistanceSquare = distanceSquare;

				result = GetJunctionElem(junction);
			}
		}
	}
	return result;
}

static V2 GetBuildingCenter(Building* building)
{
	V2 result = {};
	result.x = (building->left + building->right) * 0.5f;
	result.y = (building->top + building->bottom) * 0.5f;
	return result;
}

static Building* GetRandomBuilding(Map* map)
{
	I32 buildingIndex = RandMod(map->buildingN);
	Building* building = &map->buildings[buildingIndex];
	return building;
}

static Building* GetClosestBuilding(Map* map, V2 point, BuildingType type)
{
	Building* result = 0;

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) 
	{
		Building* building = &map->buildings[i];
		if (building->type != type) 
		{
			continue;
		}

		V2 center = GetBuildingCenter(building);
		F32 distanceSquare = DistanceSquare(point, center);
		if (!result || distanceSquare < minDistanceSquare) 
		{
			result = building;
			minDistanceSquare = distanceSquare;
		}
	}

	return result;
}

static BuildingCrossInfo GetClosestExtBuildingCrossInfo(Map* map, F32 radius, V2 closePoint, V2 farPoint)
{
	BuildingCrossInfo result = {};

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) 
	{
		BuildingCrossInfo crossInfo = ExtBuildingClosestCrossInfo(&map->buildings[i], radius, closePoint, farPoint);
		if (crossInfo.building) 
		{
			// TODO: is it a problem that this distanceSquare is calculated twice?
			F32 distanceSquare = DistanceSquare(closePoint, crossInfo.crossPoint);
			if (!result.building || distanceSquare < minDistanceSquare) 
			{
				minDistanceSquare = distanceSquare;
				result = crossInfo;
			}
		}
	}
	
	return result;
}

static Building* GetClosestCrossedBuilding(Map* map, V2 pointClose, V2 pointFar, Building* excludedBuilding)
{
	Building* result = 0;

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) 
	{
		Building* building = &map->buildings[i];
		if (building == excludedBuilding) 
		{
			continue;
		}

		if (IsBuildingCrossed(*building, pointClose, pointFar)) 
		{
			V2 closestCrossPoint = ClosestBuildingCrossPoint(*building, pointClose, pointFar);

			F32 distanceSquare = DistanceSquare(pointClose, closestCrossPoint);
			if (result == 0 || distanceSquare < minDistanceSquare) 
			{
				result = building;
				minDistanceSquare = distanceSquare;
			}
		}
	}

	return result;
}

static Building* GetBuildingAtPoint(Map* map, V2 point)
{
	Building* result = 0;
	for (I32 i = 0; i < map->buildingN; ++i) 
	{
		if (IsPointInBuilding(point, map->buildings[i])) 
		{
			result = &map->buildings[i];
		}
	}

	return result;
}

static MapElem GetBuildingElem(Building* building)
{
	MapElem result = {};
	result.type = MapElemBuilding;
	result.building = building;

	return result;
}

static MapElem GetBuildingConnectorElem(Building* building)
{
	MapElem result = {};
	result.type = MapElemBuildingConnector;
	result.building = building;

	return result;
}

static MapElem GetRoadElemAtPoint(Map* map, V2 point)
{
	MapElem result = {};
	result.type = MapElemNone;

	for (I32 i = 0; i < map->roadN; ++i) 
	{
		Road* road = &map->roads[i];
		if (IsPointOnRoad(point, road)) 
		{
			result = GetRoadElem(road);
			return result;
		}
	}

	for (I32 i = 0; i < map->junctionN; ++i) 
	{
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction))
		{
			result = GetJunctionElem(junction);
			return result;
		}
	}

	for (I32 i = 0; i < map->buildingN; ++i) 
	{
		if (IsPointInBuilding(point, map->buildings[i])) 
		{
			result = GetBuildingElem(&map->buildings[i]);
			return result;
		}

		if (IsPointOnBuildingConnector(point, map->buildings[i])) 
		{
			result = GetBuildingConnectorElem(&map->buildings[i]);
			return result;
		}
	}

	return result;
}

static MapElem GetJunctionSidewalkElem(Junction* junction)
{
	MapElem result = {};
	result.type = MapElemJunctionSidewalk;
	result.junction = junction;

	return result;
}

static MapElem GetRoadSidewalkElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemRoadSidewalk;
	result.road = road;

	return result;
}

static MapElem GetCrossingElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemCrossing;
	result.road = road;

	return result;
}

static MapElem GetPedestrianElemAtPoint(Map* map, V2 point)
{
	MapElem result = {};

	for (I32 i = 0; i < map->junctionN; ++i) 
	{
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunctionSidewalk(point, junction)) 
		{
			result = GetJunctionSidewalkElem(junction);
			return result;
		}
	}

	for (I32 i = 0; i < map->roadN; ++i) 
	{
		Road* road = &map->roads[i];
		if (IsPointOnRoadSidewalk(point, road)) 
		{
			result = GetRoadSidewalkElem(road);
			return result;
		}

		if (IsPointOnCrossing(point, road)) 
		{
			result = GetCrossingElem(road);
			return result;
		}
	}

	return result;
}

static void DrawGroundElems(Canvas* canvas, Map* map)
{
	ClearScreen(canvas, GrassColor);

	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawRoadSidewalk(canvas, map->roads + i);
	}
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawJunctionSidewalk(canvas, map->junctions + i);
	}

	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawRoad(canvas, map->roads + i);
	}
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawJunction(canvas, map->junctions + i);
	}
}

static void DrawTexturedGroundElems(Canvas* canvas, Map* map, MapTextures* textures)
{
	FillScreenWithWorldTexture(canvas, textures->grassTexture);
	
	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawTexturedRoadSidewalk(canvas, &map->roads[i], textures->sidewalkTexture);
	}
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawTexturedJunctionSidewalk(canvas, &map->junctions[i], textures->sidewalkTexture);
	}

	for (I32 i = 0; i < map->roadN; ++i)
	{
		DrawTexturedRoad(canvas, &map->roads[i], textures->roadTexture, textures->stripeTexture);
	}
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawTexturedJunction(canvas, &map->junctions[i], textures->roadTexture, textures->stripeTexture);
	}
}

static void DrawAllTrafficLights(Canvas* canvas, Map* map)
{
	for (I32 i = 0; i < map->junctionN; ++i)
	{
		DrawTrafficLights(canvas, &map->junctions[i]);
	}
}

static B32 AreMapElemsEqual(MapElem elem1, MapElem elem2)
{
	if (elem1.type != elem2.type) 
	{
		return false;
	}
	else if (elem1.type == MapElemNone) 
	{
		return true;
	}
	else
	{
		return (elem1.address == elem2.address);
	}
}

static void HighlightMapElem(Canvas* canvas, MapElem mapElem, V4 color)
{
	if (mapElem.type == MapElemBuilding)
	{
		HighlightBuilding(canvas, *mapElem.building, color);
	}
	else if (mapElem.type == MapElemBuildingConnector)
	{
		HighlightBuildingConnector(canvas, *mapElem.building, color);
	}
	else if (mapElem.type == MapElemJunction)
	{
		HighlightJunction(canvas, mapElem.junction, color);
	}
	else if (mapElem.type == MapElemJunctionSidewalk) 
	{
		HighlightJunctionSidewalk(canvas, mapElem.junction, color);
	}
	else if (mapElem.type == MapElemRoad)
	{
		HighlightRoad(canvas, mapElem.road, color);
	}
	else if (mapElem.type == MapElemRoadSidewalk)
	{
		HighlightRoadSidewalk(canvas, mapElem.road, color);
	}
}

static B32 IsMapTileIndexValid(Map* map, MapTileIndex tileIndex)
{
	B32 isValid = true;
	if (tileIndex.row < 0 || tileIndex.row >= map->tileRowN)
	{
		isValid = false;
	}
	else if (tileIndex.col < 0 || tileIndex.col >= map->tileColN)
	{
		isValid = false;
	}
	return isValid;
}

static MapTileIndex GetMapTileIndexContainingPoint(Map* map, V2 point)
{
	MapTileIndex tileIndex = {};
	tileIndex.col = Floor((point.x - map->left) / MapTileSide);
	tileIndex.row = Floor((point.y - map->top) / MapTileSide);
	return tileIndex;
}

static B32 IsMapTileCached(Map* map, MapTileIndex tileIndex)
{
	B32 isCached = false;
	for (I32 i = 0; i < map->cachedTileN; ++i) 
	{
		MapTileIndex cachedTileIndex = map->cachedTiles[i].index;
		if (cachedTileIndex.row == tileIndex.row && cachedTileIndex.col == tileIndex.col) 
		{
			isCached = true;
			break;
		}
	}
	return isCached;
}

static V2 GetMapTileCenter(Map* map, MapTileIndex tileIndex)
{
	Assert(IsMapTileIndexValid(map, tileIndex));
	V2 tileCenter = {};
	tileCenter.x = map->left + (tileIndex.col * MapTileSide) + (0.5f * MapTileSide);
	tileCenter.y = map->top  + (tileIndex.row * MapTileSide) + (0.5f * MapTileSide);
	return tileCenter;
}

static CachedMapTile* GetFarthestCachedMapTile(Map* map, V2 point)
{
	Assert(map->cachedTileN > 0);
	CachedMapTile* cachedTile = 0;
	F32 maxDistance = 0.0f;
	for (I32 i = 0; i < map->cachedTileN; ++i) 
	{
		MapTileIndex tileIndex = map->cachedTiles[i].index;
		V2 tileCenter = GetMapTileCenter(map, tileIndex);
		F32 distance = Distance(tileCenter, point);
		if (distance >= maxDistance) 
		{
			maxDistance = distance;
			cachedTile = &map->cachedTiles[i];
		}
	}

	Assert(cachedTile != 0);
	return cachedTile;
}

static void GenerateMapTileBitmap(Map* map, MapTileIndex tileIndex, Bitmap* bitmap, MapTextures* mapTextures)
{
	Camera camera = {};
	camera.center = GetMapTileCenter(map, tileIndex);
	camera.screenPixelSize.x = (F32)bitmap->width;
	camera.screenPixelSize.y = (F32)bitmap->height;
	camera.unitInPixels = (F32)bitmap->width / MapTileSide;

	Canvas canvas = {};
	canvas.bitmap = *bitmap;
	canvas.camera = &camera;
	DrawGroundElems(&canvas, map);
	// DrawTexturedGroundElems(canvas, map, mapTextures);
}

static DWORD WINAPI GenerateMapTileWorkProc(LPVOID parameter)
{
	GenerateMapTileWorkList* workList = (GenerateMapTileWorkList*)parameter;
	while (1) 
	{
		WaitForSingleObjectEx(workList->semaphore, INFINITE, FALSE);
		I32 workIndex = (I32)InterlockedIncrement((volatile U64*)&workList->workDoneN) - 1;
		workIndex = (workIndex + MaxGenerateMapTileWorkListN) % MaxGenerateMapTileWorkListN;
		Assert(workIndex != workList->workPushedN % MaxGenerateMapTileWorkListN);
		GenerateMapTileWork work = workList->works[workIndex];
		GenerateMapTileBitmap(work.map, work.tileIndex, work.bitmap, work.mapTextures);
	}
}

static void PushGenerateMapTileWork(GenerateMapTileWorkList* workList, GenerateMapTileWork work)
{
	workList->works[workList->workPushedN % MaxGenerateMapTileWorkListN] = work;
	workList->workPushedN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
}

static void CacheMapTile(Map* map, MapTileIndex tileIndex, MapTextures* textures)
{
	Assert(!IsMapTileCached(map, tileIndex));
	CachedMapTile* cachedTile = 0;
	if (map->cachedTileN < MaxCachedMapTileN) 
	{
		cachedTile = map->cachedTiles + map->cachedTileN;
		ResizeBitmap(&cachedTile->bitmap, MapTileBitmapWidth, MapTileBitmapHeight);
		map->cachedTileN++;
	} 
	else 
	{
		V2 tileCenter = GetMapTileCenter(map, tileIndex);
		cachedTile = GetFarthestCachedMapTile(map, tileCenter);
	}
	Assert(cachedTile != 0);
	cachedTile->index = tileIndex;
	GenerateMapTileWork work = {};
	work.map = map;
	work.tileIndex = tileIndex;
	work.bitmap = &cachedTile->bitmap;
	work.mapTextures = textures;
	PushGenerateMapTileWork(&map->generateTileWorkList, work);
}

static Bitmap* GetCachedTileBitmap(Map* map, MapTileIndex tileIndex)
{
	Bitmap* bitmap = 0;
	for (I32 i = 0; i < map->cachedTileN; ++i) 
	{
		CachedMapTile* cachedTile = map->cachedTiles + i;
		if (cachedTile->index.row == tileIndex.row && cachedTile->index.col == tileIndex.col) 
		{
			bitmap = &cachedTile->bitmap;
			break;
		}
	}

	Assert(bitmap != 0);
	return bitmap;
}

static void DrawMapTile(Canvas* canvas, Map* map, MapTileIndex tileIndex, MapTextures* textures)
{
	F32 tileLeft   = map->left + (tileIndex.col * MapTileSide);
	F32 tileRight  = tileLeft + MapTileSide;
	F32 tileTop    = map->top + (tileIndex.row * MapTileSide);
	F32 tileBottom = tileTop + MapTileSide;

	Bitmap* tileBitmap = GetCachedTileBitmap(map, tileIndex);
	DrawStretchedBitmap(canvas, tileBitmap, tileLeft, tileRight, tileTop, tileBottom);
}

static void PushDrawMapTileWork(DrawMapTileWorkList* workList, DrawMapTileWork work)
{
	Assert(workList->workN < MaxDrawMapTileWorkListN);
	workList->works[workList->workN] = work;
	workList->workN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
}

static void DrawVisibleMapTiles(Canvas* canvas, Map* map, F32 left, F32 right, F32 top, F32 bottom, MapTextures* textures)
{
	V2 topLeft     = MakePoint(left, top);
	V2 bottomRight = MakePoint(right, bottom);
	MapTileIndex topLeftTileIndex     = GetMapTileIndexContainingPoint(map, topLeft);
	MapTileIndex bottomRightTileIndex = GetMapTileIndexContainingPoint(map, bottomRight);

	for (I32 row = topLeftTileIndex.row; row <= bottomRightTileIndex.row; ++row) 
	{
		for (I32 col = topLeftTileIndex.col; col <= bottomRightTileIndex.col; ++col) 
		{
			MapTileIndex tileIndex = MapTileIndex{row, col};

			if (!IsMapTileIndexValid(map, tileIndex))
			{
				continue;
			}

			if (!IsMapTileCached(map, tileIndex))
			{
				CacheMapTile(map, tileIndex, textures);
			}

			if (IsMapTileIndexValid(map, tileIndex)) 
			{
				DrawMapTileWork work = {};
				work.canvas = *canvas;
				work.map = map;
				work.tileIndex = tileIndex;
				work.textures = textures;
				PushDrawMapTileWork(&map->drawTileWorkList, work);
			}
		}
	}

	for (I32 i = 0; i < map->drawTileWorkList.workN; ++i)
	{
		WaitForSingleObjectEx(map->drawTileWorkList.semaphoreDone, INFINITE, FALSE);
	}
	map->drawTileWorkList.workN = 0;
	map->drawTileWorkList.firstWorkToDo = 0;
}

static DWORD WINAPI DrawMapTileWorkProc(LPVOID parameter)
{
	DrawMapTileWorkList* workList = (DrawMapTileWorkList*)parameter;
	while (1) {
		WaitForSingleObjectEx(workList->semaphore, INFINITE, FALSE);
		Assert (workList->firstWorkToDo < workList->workN);
		I32 workIndex = (I32)InterlockedIncrement((volatile U64*)&workList->firstWorkToDo) - 1;
		DrawMapTileWork work = workList->works[workIndex];
		DrawMapTile(&work.canvas, work.map, work.tileIndex, work.textures);
		ReleaseSemaphore(workList->semaphoreDone, 1, 0);
	}
}