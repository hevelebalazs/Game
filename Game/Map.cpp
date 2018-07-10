#include "Game.hpp"
#include "Geometry.hpp"
#include "Map.hpp"
#include "Math.hpp"
#include "Type.hpp"

void GenerateMapTextures(MapTextures* textures, MemArena* tmpArena)
{
	textures->grassTexture    = GrassTexture(8, tmpArena);
	textures->roadTexture     = RandomGreyTexture(8, 100, 150);
	textures->sidewalkTexture = RandomGreyTexture(8, 80, 100);
	textures->stripeTexture   = RandomGreyTexture(8, 200, 255);
}

Junction* GetRandomJunction(Map* map)
{
	I32 junctionIndex = RandMod(map->junctionN);
	Junction* junction = &map->junctions[junctionIndex];
	return junction;
}

Junction* GetJunctionAtPoint(Map* map, V2 point)
{
	Junction* result = 0;
	for (I32 i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction)) {
			result = junction;
			break;
		}
	}
	return result;
}

MapElem GetClosestRoadElem(Map* map, V2 point)
{
	MapElem result = {};

	Road* closestRoad = 0;
	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->roadN; ++i) {
		Road* road = &map->roads[i];

		B32 betweenX =
			((road->endPoint1.x <= point.x) && (point.x <= road->endPoint2.x)) ||
			((road->endPoint2.x <= point.x) && (point.x <= road->endPoint1.x));
		B32 betweenY =
			((road->endPoint1.y <= point.y) && (point.y <= road->endPoint2.y)) ||
			((road->endPoint2.y <= point.y) && (point.y <= road->endPoint1.y));

		if (betweenX || betweenY) {
			F32 distanceSquare = DistanceSquareFromRoad(road, point);
			if (!closestRoad || distanceSquare < minDistanceSquare) {
				closestRoad = road;
				minDistanceSquare = distanceSquare;

				result = GetRoadElem(road);
			}
		}
	}

	Junction* closestJunction = 0;
	for (I32 i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];

		B32 betweenX = (Abs(junction->position.x - point.x) <= LaneWidth);
		B32 betweenY = (Abs(junction->position.y - point.y) <= LaneWidth);

		if (betweenX || betweenY) {
			F32 distanceSquare = DistanceSquare(junction->position, point);
			if ((!closestRoad && !closestJunction) || distanceSquare < minDistanceSquare) {
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

Building* GetRandomBuilding(Map* map)
{
	I32 buildingIndex = RandMod(map->buildingN);
	Building* building = &map->buildings[buildingIndex];
	return building;
}

Building* GetClosestBuilding(Map* map, V2 point, BuildingType type)
{
	Building* result = 0;

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) {
		Building* building = &map->buildings[i];
		if (building->type != type) 
			continue;

		V2 center = GetBuildingCenter(building);
		F32 distanceSquare = DistanceSquare(point, center);
		if (!result || distanceSquare < minDistanceSquare) {
			result = building;
			minDistanceSquare = distanceSquare;
		}
	}

	return result;
}

BuildingCrossInfo GetClosestExtBuildingCrossInfo(Map* map, F32 radius, V2 closePoint, V2 farPoint)
{
	BuildingCrossInfo result = {};

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) {
		BuildingCrossInfo crossInfo = ExtBuildingClosestCrossInfo(&map->buildings[i], radius, closePoint, farPoint);
		if (crossInfo.building) {
			// TODO: is it a problem that this distanceSquare is calculated twice?
			F32 distanceSquare = DistanceSquare(closePoint, crossInfo.crossPoint);
			if (!result.building || distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossInfo;
			}
		}
	}
	
	return result;
}

Building* GetClosestCrossedBuilding(Map* map, V2 pointClose, V2 pointFar, Building* excludedBuilding)
{
	Building* result = 0;

	F32 minDistanceSquare = 0.0f;
	for (I32 i = 0; i < map->buildingN; ++i) {
		Building* building = &map->buildings[i];
		if (building == excludedBuilding) 
			continue;

		if (IsBuildingCrossed(*building, pointClose, pointFar)) {
			V2 closestCrossPoint = ClosestBuildingCrossPoint(*building, pointClose, pointFar);

			F32 distanceSquare = DistanceSquare(pointClose, closestCrossPoint);
			if (result == 0 || distanceSquare < minDistanceSquare) {
				result = building;
				minDistanceSquare = distanceSquare;
			}
		}
	}

	return result;
}

Building* GetBuildingAtPoint(Map* map, V2 point)
{
	Building* result = 0;
	for (I32 i = 0; i < map->buildingN; ++i) {
		if (IsPointInBuilding(point, map->buildings[i])) 
			result = &map->buildings[i];
	}

	return result;
}

MapElem GetRoadElemAtPoint(Map* map, V2 point)
{
	MapElem result = {};
	result.type = MapElemNone;

	for (I32 i = 0; i < map->roadN; ++i) {
		Road* road = &map->roads[i];
		if (IsPointOnRoad(point, road)) {
			result = GetRoadElem(road);
			return result;
		}
	}

	for (I32 i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction)) {
			result = GetJunctionElem(junction);
			return result;
		}
	}

	for (I32 i = 0; i < map->buildingN; ++i) {
		if (IsPointInBuilding(point, map->buildings[i])) {
			result = GetBuildingElem(&map->buildings[i]);
			return result;
		}

		if (IsPointOnBuildingConnector(point, map->buildings[i])) {
			result = GetBuildingConnectorElem(&map->buildings[i]);
			return result;
		}
	}

	return result;
}

MapElem GetPedestrianElemAtPoint(Map* map, V2 point)
{
	MapElem result = {};

	for (I32 i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunctionSidewalk(point, junction)) {
			result = GetJunctionSidewalkElem(junction);
			return result;
		}
	}

	for (I32 i = 0; i < map->roadN; ++i) {
		Road* road = &map->roads[i];
		if (IsPointOnRoadSidewalk(point, road)) {
			result = GetRoadSidewalkElem(road);
			return result;
		}

		if (IsPointOnCrossing(point, road)) {
			result = GetCrossingElem(road);
			return result;
		}
	}

	return result;
}

void DrawGroundElems(Canvas canvas, Map* map)
{
	ClearScreen(canvas, GrassColor);

	for (I32 i = 0; i < map->roadN; ++i)
		DrawRoadSidewalk(canvas, map->roads + i);
	for (I32 i = 0; i < map->junctionN; ++i)
		DrawJunctionSidewalk(canvas, map->junctions + i);

	for (I32 i = 0; i < map->roadN; ++i)
		DrawRoad(canvas, map->roads + i);
	for (I32 i = 0; i < map->junctionN; ++i)
		DrawJunction(canvas, map->junctions + i);

	for (I32 i = 0; i < map->junctionN; ++i)
		DrawTrafficLights(canvas, map->junctions + i);
}

void DrawTexturedGroundElems(Canvas canvas, Map* map, MapTextures* textures)
{
	FillScreenWithWorldTexture(canvas, textures->grassTexture);
	
	for (I32 i = 0; i < map->roadN; ++i)
		DrawTexturedRoadSidewalk(canvas, &map->roads[i], textures->sidewalkTexture);
	for (I32 i = 0; i < map->junctionN; ++i)
		DrawTexturedJunctionSidewalk(canvas, &map->junctions[i], textures->sidewalkTexture);

	for (I32 i = 0; i < map->roadN; ++i)
		DrawTexturedRoad(canvas, &map->roads[i], textures->roadTexture, textures->stripeTexture);
	for (I32 i = 0; i < map->junctionN; ++i)
		DrawTexturedJunction(canvas, &map->junctions[i], textures->roadTexture, textures->stripeTexture);
}

// TODO: there are two instances of mergesort in the code
//       pull those two together somehow?
struct BuildingHelper {
	I32 buildingCount;
	Building* buildings;
	Building* tmpBuildings;
	MemArena* arena;
};

static B32 AreBuildingsInOrder(V2 center, Building building1, Building building2)
{
	V2 point1 = GetBuildingCenter(&building1);
	V2 point2 = GetBuildingCenter(&building2);

	F32 dist1 = CityDistance(center, point1);
	F32 dist2 = CityDistance(center, point2);

	if (dist1 > dist2)
		return true;
	else
		return false;
}

static void MergeBuildingArrays(BuildingHelper* helper, V2 center, I32 leftStart, I32 leftEnd, I32 rightStart, I32 rightEnd)
{
	I32 left = leftStart;
	I32 right = rightStart;

	for (I32 i = leftStart; i <= rightEnd; ++i) {
		B32 chooseLeft = false;
		B32 chooseRight = false;

		if (left > leftEnd)
			chooseRight = true;
		else if (right > rightEnd)
			chooseLeft = true;
		else if (AreBuildingsInOrder(center, helper->buildings[left], helper->buildings[right]))
			chooseLeft = true;
		else
			chooseRight = true;

		if (chooseLeft) {
			helper->tmpBuildings[i] = helper->buildings[left];
			left++;
		}

		if (chooseRight) {
			helper->tmpBuildings[i] = helper->buildings[right];
			right++;
		}
	}

	// TODO: use some version of memcpy here?
	for (I32 i = leftStart; i <= rightEnd; ++i)
		helper->buildings[i] = helper->tmpBuildings[i];
}

static void SortBuildings(BuildingHelper* helper, V2 center)
{
	I32 length = 1;
	while (length <= helper->buildingCount) {
		I32 leftStart = 0;
		while (leftStart < helper->buildingCount) {
			I32 leftEnd = leftStart + (length - 1);

			I32 rightStart = leftEnd + 1;
			if (rightStart >= helper->buildingCount)
				break;

			I32 rightEnd = rightStart + (length - 1);
			if (rightEnd >= helper->buildingCount)
				rightEnd = helper->buildingCount - 1;

			MergeBuildingArrays(helper, center, leftStart, leftEnd, rightStart, rightEnd);

			leftStart = rightEnd + 1;
		}

		length *= 2;
	}
}

// TODO: move the sorting logic to draw.cpp
void DrawBuildings(Canvas canvas, Map* map, MemArena* arena, GameAssets* assets)
{
	for (I32 i = 0; i < map->buildingN; ++i)
		DrawBuilding(canvas, map->buildings[i], assets);

	/*
	// NOTE: 3d buildings
	BuildingHelper helper = {};
	helper.buildingCount = map->buildingCount;
	helper.buildings = ArenaPushArray(arena, Building, helper.buildingCount);
	// TODO: use memcpy for this?
	for (I32 i = 0; i < helper.buildingCount; ++i)
		helper.buildings[i] = map->buildings[i];

	helper.tmpBuildings = ArenaPushArray(arena, Building, helper.buildingCount);

	SortBuildings(&helper, canvas.camera->center);
	
	for (I32 i = 0; i < helper.buildingCount; ++i)
		DrawBuilding(canvas, helper.buildings[i]);

	ArenaPopTo(arena, helper.buildings);
	*/
}

void DrawMap(Canvas canvas, Map* map, MemArena* arena, GameAssets* assets)
{
	DrawGroundElems(canvas, map);
	DrawBuildings(canvas, map, arena, assets);
}

void DrawAllTrafficLights(Canvas canvas, Map* map)
{
	for (I32 i = 0; i < map->junctionN; ++i)
		DrawTrafficLights(canvas, &map->junctions[i]);
}

MapElem GetRoadElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemRoad;
	result.road = road;

	return result;
}

MapElem GetRoadSidewalkElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemRoadSidewalk;
	result.road = road;

	return result;
}

MapElem GetCrossingElem(Road* road)
{
	MapElem result = {};
	result.type = MapElemCrossing;
	result.road = road;

	return result;
}

MapElem GetJunctionElem(Junction* junction)
{
	MapElem result = {};
	result.type = MapElemJunction;
	result.junction = junction;

	return result;
}

MapElem GetJunctionSidewalkElem(Junction* junction)
{
	MapElem result = {};
	result.type = MapElemJunctionSidewalk;
	result.junction = junction;

	return result;
}

MapElem GetBuildingElem(Building* building)
{
	MapElem result = {};
	result.type = MapElemBuilding;
	result.building = building;

	return result;
}

MapElem GetBuildingConnectorElem(Building* building)
{
	MapElem result = {};
	result.type = MapElemBuildingConnector;
	result.building = building;

	return result;
}

B32 AreMapElemsEqual(MapElem elem1, MapElem elem2)
{
	if (elem1.type != elem2.type) 
		return false;
	else if (elem1.type == MapElemNone) 
		return true;
	else
		return (elem1.address == elem2.address);
}

void HighlightMapElem(Canvas canvas, MapElem mapElem, V4 color)
{
	if (mapElem.type == MapElemBuilding)
		HighlightBuilding(canvas, *mapElem.building, color);
	else if (mapElem.type == MapElemBuildingConnector)
		HighlightBuildingConnector(canvas, *mapElem.building, color);
	else if (mapElem.type == MapElemJunction)
		HighlightJunction(canvas, mapElem.junction, color);
	else if (mapElem.type == MapElemJunctionSidewalk) 
		HighlightJunctionSidewalk(canvas, mapElem.junction, color);
	else if (mapElem.type == MapElemRoad)
		HighlightRoad(canvas, mapElem.road, color);
	else if (mapElem.type == MapElemRoadSidewalk)
		HighlightRoadSidewalk(canvas, mapElem.road, color);
}

B32 IsMapTileIndexValid(Map* map, MapTileIndex tileIndex)
{
	B32 isValid = true;
	if (tileIndex.row < 0 || tileIndex.row >= map->tileRowN)
		isValid = false;
	else if (tileIndex.col < 0 || tileIndex.col >= map->tileColN)
		isValid = false;
	return isValid;
}

MapTileIndex GetMapTileIndexContainingPoint(Map* map, V2 point)
{
	MapTileIndex tileIndex = {};
	tileIndex.col = Floor((point.x - map->left) / MapTileSide);
	tileIndex.row = Floor((point.y - map->top) / MapTileSide);
	return tileIndex;
}

B32 IsMapTileCached(Map* map, MapTileIndex tileIndex)
{
	B32 isCached = false;
	for (I32 i = 0; i < map->cachedTileN; ++i) {
		MapTileIndex cachedTileIndex = map->cachedTiles[i].index;
		if (cachedTileIndex.row == tileIndex.row && cachedTileIndex.col == tileIndex.col) {
			isCached = true;
			break;
		}
	}
	return isCached;
}

V2 GetMapTileCenter(Map* map, MapTileIndex tileIndex)
{
	Assert(IsMapTileIndexValid(map, tileIndex));
	V2 tileCenter = {};
	tileCenter.x = map->left + (tileIndex.col * MapTileSide) + (0.5f * MapTileSide);
	tileCenter.y = map->top  + (tileIndex.row * MapTileSide) + (0.5f * MapTileSide);
	return tileCenter;
}

CachedMapTile* GetFarthestCachedMapTile(Map* map, V2 point)
{
	Assert(map->cachedTileN > 0);
	CachedMapTile* cachedTile = 0;
	F32 maxDistance = 0.0f;
	for (I32 i = 0; i < map->cachedTileN; ++i) {
		MapTileIndex tileIndex = map->cachedTiles[i].index;
		V2 tileCenter = GetMapTileCenter(map, tileIndex);
		F32 distance = Distance(tileCenter, point);
		if (distance >= maxDistance) {
			maxDistance = distance;
			cachedTile = &map->cachedTiles[i];
		}
	}
	return cachedTile;
	Assert(cachedTile != 0);
}

void GenerateMapTileBitmap(Map* map, MapTileIndex tileIndex, Bitmap* bitmap, MapTextures* mapTextures)
{
	Camera camera = {};
	camera.center = GetMapTileCenter(map, tileIndex);
	camera.screenPixelSize.x = (F32)bitmap->width;
	camera.screenPixelSize.y = (F32)bitmap->height;
	camera.unitInPixels = (F32)bitmap->width / MapTileSide;

	Canvas canvas = {};
	canvas.bitmap = *bitmap;
	canvas.camera = &camera;
	DrawTexturedGroundElems(canvas, map, mapTextures);
}

DWORD WINAPI GenerateMapTileWorkProc(LPVOID parameter)
{
	GenerateMapTileWorkList* workList = (GenerateMapTileWorkList*)parameter;
	while (1) {
		if (workList->firstWorkToDo < workList->workN) {
			I32 workIndex = (I32)InterlockedIncrement((volatile U64*)&workList->firstWorkToDo) - 1;
			GenerateMapTileWork work = workList->works[workIndex];
			GenerateMapTileBitmap(work.map, work.tileIndex, work.bitmap, work.mapTextures);
		} else {
			WaitForSingleObjectEx(workList->semaphore, INFINITE, FALSE);
		}
	}
}

void PushGenerateMapTileWork(GenerateMapTileWorkList* workList, GenerateMapTileWork work)
{
	Assert(workList->workN < MaxGenerateMapTileWorkListN);
	workList->works[workList->workN] = work;
	workList->workN++;
	ReleaseSemaphore(workList->semaphore, 1, 0);
}

void CacheMapTile(Map* map, MapTileIndex tileIndex, MapTextures* textures)
{
	Assert(!IsMapTileCached(map, tileIndex));
	CachedMapTile* cachedTile = 0;
	if (map->cachedTileN < MaxCachedMapTileN) {
		cachedTile = map->cachedTiles + map->cachedTileN;
		ResizeBitmap(&cachedTile->bitmap, MapTileBitmapWidth, MapTileBitmapHeight);
		map->cachedTileN++;
	} else {
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
	PushGenerateMapTileWork(&map->workList, work);
}

Bitmap* GetCachedTileBitmap(Map* map, MapTileIndex tileIndex)
{
	Bitmap* bitmap = 0;
	for (I32 i = 0; i < map->cachedTileN; ++i) {
		CachedMapTile* cachedTile = map->cachedTiles + i;
		if (cachedTile->index.row == tileIndex.row && cachedTile->index.col == tileIndex.col) {
			bitmap = &cachedTile->bitmap;
			break;
		}
	}
	Assert(bitmap != 0);
	return bitmap;
}

void DrawMapTile(Canvas canvas, Map* map, MapTileIndex tileIndex, MapTextures* textures)
{
	F32 tileLeft   = map->left + (tileIndex.col * MapTileSide);
	F32 tileRight  = tileLeft + MapTileSide;
	F32 tileTop    = map->top + (tileIndex.row * MapTileSide);
	F32 tileBottom = tileTop + MapTileSide;

	if (!IsMapTileCached(map, tileIndex))
		CacheMapTile(map, tileIndex, textures);
		
	Bitmap* tileBitmap = GetCachedTileBitmap(map, tileIndex);
	DrawStretchedBitmap(canvas, tileBitmap, tileLeft, tileRight, tileTop, tileBottom);
}

void DrawVisibleMapTiles(Canvas canvas, Map* map, F32 left, F32 right, F32 top, F32 bottom, MapTextures* textures)
{
	V2 topLeft     = MakePoint(left, top);
	V2 bottomRight = MakePoint(right, bottom);
	MapTileIndex topLeftTileIndex     = GetMapTileIndexContainingPoint(map, topLeft);
	MapTileIndex bottomRightTileIndex = GetMapTileIndexContainingPoint(map, bottomRight);
	for (I32 row = topLeftTileIndex.row; row <= bottomRightTileIndex.row; ++row) {
		for (I32 col = topLeftTileIndex.col; col <= bottomRightTileIndex.col; ++col) {
			MapTileIndex tileIndex = MapTileIndex{row, col};
			if (IsMapTileIndexValid(map, tileIndex))
				DrawMapTile(canvas, map, tileIndex, textures);
		}
	}
}