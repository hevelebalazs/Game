#include "Game.hpp"
#include "Geometry.hpp"
#include "Map.hpp"
#include "Math.hpp"

extern Color GrassColor = Color{0.0f, 0.8f, 0.0f};

Junction* GetRandomJunction(Map* map)
{
	int junctionIndex = RandMod(map->junctionN);
	Junction* junction = &map->junctions[junctionIndex];
	return junction;
}

Junction* GetJunctionAtPoint(Map* map, Point point)
{
	Junction* result = 0;
	for (int i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction)) {
			result = junction;
			break;
		}
	}
	return result;
}

MapElem GetClosestRoadElem(Map* map, Point point)
{
	MapElem result = {};

	Road* closestRoad = 0;
	float minDistanceSquare = 0.0f;
	for (int i = 0; i < map->roadN; ++i) {
		Road* road = &map->roads[i];

		bool betweenX =
			((road->endPoint1.x <= point.x) && (point.x <= road->endPoint2.x)) ||
			((road->endPoint2.x <= point.x) && (point.x <= road->endPoint1.x));
		bool betweenY =
			((road->endPoint1.y <= point.y) && (point.y <= road->endPoint2.y)) ||
			((road->endPoint2.y <= point.y) && (point.y <= road->endPoint1.y));

		if (betweenX || betweenY) {
			float distanceSquare = DistanceSquareFromRoad(road, point);
			if (!closestRoad || distanceSquare < minDistanceSquare) {
				closestRoad = road;
				minDistanceSquare = distanceSquare;

				result = GetRoadElem(road);
			}
		}
	}

	Junction* closestJunction = 0;
	for (int i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];

		bool betweenX = (Abs(junction->position.x - point.x) <= LaneWidth);
		bool betweenY = (Abs(junction->position.y - point.y) <= LaneWidth);

		if (betweenX || betweenY) {
			float distanceSquare = DistanceSquare(junction->position, point);

			if ((!closestRoad && !closestJunction) || distanceSquare < minDistanceSquare) {
				closestJunction = junction;
				minDistanceSquare = distanceSquare;

				result = GetJunctionElem(junction);
			}
		}
	}
	return result;
}

static Point GetBuildingCenter(Building* building)
{
	Point result = {};
	result.x = (building->left + building->right) * 0.5f;
	result.y = (building->top + building->bottom) * 0.5f;
	return result;
}

Building* GetRandomBuilding(Map* map)
{
	int buildingIndex = RandMod(map->buildingN);
	Building* building = &map->buildings[buildingIndex];
	return building;
}

Building* GetClosestBuilding(Map* map, Point point, BuildingType type)
{
	Building* result = 0;

	float minDistanceSquare = 0.0f;
	for (int i = 0; i < map->buildingN; ++i) {
		Building* building = &map->buildings[i];
		if (building->type != type) 
			continue;

		Point center = GetBuildingCenter(building);
		float distanceSquare = DistanceSquare(point, center);
		if (!result || distanceSquare < minDistanceSquare) {
			result = building;
			minDistanceSquare = distanceSquare;
		}
	}

	return result;
}

BuildingCrossInfo GetClosestExtBuildingCrossInfo(Map* map, float radius, Point closePoint, Point farPoint)
{
	BuildingCrossInfo result = {};

	float minDistanceSquare = 0.0f;
	for (int i = 0; i < map->buildingN; ++i) {
		BuildingCrossInfo crossInfo = ExtBuildingClosestCrossInfo(&map->buildings[i], radius, closePoint, farPoint);
		if (crossInfo.building) {
			// TODO: is it a problem that this distanceSquare is calculated twice?
			float distanceSquare = DistanceSquare(closePoint, crossInfo.crossPoint);
			if (!result.building || distanceSquare < minDistanceSquare) {
				minDistanceSquare = distanceSquare;
				result = crossInfo;
			}
		}
	}
	
	return result;
}

Building* GetClosestCrossedBuilding(Map* map, Point pointClose, Point pointFar, Building* excludedBuilding)
{
	Building* result = 0;

	float minDistanceSquare = 0.0f;
	for (int i = 0; i < map->buildingN; ++i) {
		Building* building = &map->buildings[i];
		if (building == excludedBuilding) 
			continue;

		if (IsBuildingCrossed(*building, pointClose, pointFar)) {
			Point closestCrossPoint = ClosestBuildingCrossPoint(*building, pointClose, pointFar);

			float distanceSquare = DistanceSquare(pointClose, closestCrossPoint);
			if (result == 0 || distanceSquare < minDistanceSquare) {
				result = building;
				minDistanceSquare = distanceSquare;
			}
		}
	}

	return result;
}

Building* GetBuildingAtPoint(Map* map, Point point)
{
	Building* result = 0;
	for (int i = 0; i < map->buildingN; ++i) {
		if (IsPointInBuilding(point, map->buildings[i])) 
			result = &map->buildings[i];
	}

	return result;
}

MapElem GetRoadElemAtPoint(Map* map, Point point)
{
	MapElem result = {};
	result.type = MapElemNone;

	for (int i = 0; i < map->roadN; ++i) {
		Road* road = &map->roads[i];
		if (IsPointOnRoad(point, road)) {
			result = GetRoadElem(road);
			return result;
		}
	}

	for (int i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunction(point, junction)) {
			result = GetJunctionElem(junction);
			return result;
		}
	}

	for (int i = 0; i < map->buildingN; ++i) {
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

MapElem GetPedestrianElemAtPoint(Map* map, Point point)
{
	MapElem result = {};

	for (int i = 0; i < map->junctionN; ++i) {
		Junction* junction = &map->junctions[i];
		if (IsPointOnJunctionSidewalk(point, junction)) {
			result = GetJunctionSidewalkElem(junction);
			return result;
		}
	}

	for (int i = 0; i < map->roadN; ++i) {
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

void DrawGroundElems(Renderer renderer, Map* map)
{
	ClearScreen(renderer, GrassColor);

	for (int i = 0; i < map->roadN; ++i)
		DrawRoadSidewalk(renderer, map->roads + i);
	for (int i = 0; i < map->junctionN; ++i)
		DrawJunctionSidewalk(renderer, map->junctions + i);

	for (int i = 0; i < map->roadN; ++i)
		DrawRoad(renderer, map->roads + i);
	for (int i = 0; i < map->junctionN; ++i)
		DrawJunction(renderer, map->junctions + i);

	for (int i = 0; i < map->junctionN; ++i)
		DrawTrafficLights(renderer, map->junctions + i);
}

// TODO: there are two instances of mergesort in the code
//       pull those two together somehow?
struct BuildingHelper {
	int buildingCount;
	Building* buildings;
	Building* tmpBuildings;
	MemArena* arena;
};

inline bool AreBuildingsInOrder(Point center, Building building1, Building building2)
{
	Point point1 = GetBuildingCenter(&building1);
	Point point2 = GetBuildingCenter(&building2);

	float dist1 = CityDistance(center, point1);
	float dist2 = CityDistance(center, point2);

	if (dist1 > dist2)
		return true;
	else
		return false;
}

inline void MergeBuildingArrays(BuildingHelper* helper, Point center, int leftStart, int leftEnd, int rightStart, int rightEnd)
{
	int left = leftStart;
	int right = rightStart;

	for (int i = leftStart; i <= rightEnd; ++i) {
		bool chooseLeft = false;
		bool chooseRight = false;

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
	for (int i = leftStart; i <= rightEnd; ++i)
		helper->buildings[i] = helper->tmpBuildings[i];
}

inline void SortBuildings(BuildingHelper* helper, Point center)
{
	int length = 1;
	while (length <= helper->buildingCount) {
		int leftStart = 0;
		while (leftStart < helper->buildingCount) {
			int leftEnd = leftStart + (length - 1);

			int rightStart = leftEnd + 1;
			if (rightStart >= helper->buildingCount)
				break;

			int rightEnd = rightStart + (length - 1);
			if (rightEnd >= helper->buildingCount)
				rightEnd = helper->buildingCount - 1;

			MergeBuildingArrays(helper, center, leftStart, leftEnd, rightStart, rightEnd);

			leftStart = rightEnd + 1;
		}

		length *= 2;
	}
}

// TODO: move the sorting logic to renderer
void DrawBuildings(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets)
{
	for (int i = 0; i < map->buildingN; ++i)
		DrawBuilding(renderer, map->buildings[i], assets);

	/*
	// NOTE: 3d buildings
	BuildingHelper helper = {};
	helper.buildingCount = map->buildingCount;
	helper.buildings = ArenaPushArray(arena, Building, helper.buildingCount);
	// TODO: use memcpy for this?
	for (int i = 0; i < helper.buildingCount; ++i)
		helper.buildings[i] = map->buildings[i];

	helper.tmpBuildings = ArenaPushArray(arena, Building, helper.buildingCount);

	SortBuildings(&helper, renderer.camera->center);
	
	for (int i = 0; i < helper.buildingCount; ++i)
		DrawBuilding(renderer, helper.buildings[i]);

	ArenaPopTo(arena, helper.buildings);
	*/
}

void DrawMap(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets)
{
	DrawGroundElems(renderer, map);
	DrawBuildings(renderer, map, arena, assets);
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

bool AreMapElemsEqual(MapElem elem1, MapElem elem2)
{
	if (elem1.type != elem2.type) 
		return false;
	else if (elem1.type == MapElemNone) 
		return true;
	else
		return (elem1.address == elem2.address);
}

void HighlightMapElem(Renderer renderer, MapElem mapElem, Color color)
{
	if (mapElem.type == MapElemBuilding)
		HighlightBuilding(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemBuildingConnector)
		HighlightBuildingConnector(renderer, *mapElem.building, color);
	else if (mapElem.type == MapElemJunction)
		HighlightJunction(renderer, mapElem.junction, color);
	else if (mapElem.type == MapElemJunctionSidewalk) 
		HighlightJunctionSidewalk(renderer, mapElem.junction, color);
	else if (mapElem.type == MapElemRoad)
		HighlightRoad(renderer, mapElem.road, color);
	else if (mapElem.type == MapElemRoadSidewalk)
		HighlightRoadSidewalk(renderer, mapElem.road, color);
}
