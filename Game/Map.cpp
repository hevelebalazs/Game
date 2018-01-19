#include "Game.h"
#include "Geometry.h"
#include "Map.h"
#include "Math.h"

Intersection* RandomIntersection(Map map) {
	int intersectionIndex = RandMod(map.intersectionCount);

	return &map.intersections[intersectionIndex];
}

Intersection* IntersectionAtPoint(Map map, Point point, float maxDistance) {
	float maxDistanceSquare = maxDistance * maxDistance;

	Intersection* result = 0;

	for (int i = 0; i < map.intersectionCount; ++i) {
		float distanceSquare = DistanceSquare(point, map.intersections[i].position);

		if (distanceSquare <= maxDistanceSquare) 
			result = &map.intersections[i];
	}

	return result;
};

MapElem ClosestRoadOrIntersection(Map map, Point point) {
	MapElem result = {};
	result.type = MapElemNone;

	Road* closestRoad = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < map.roadCount; ++i) {
		Road* road = &map.roads[i];

		bool betweenX =
			((road->endPoint1.x <= point.x) && (point.x <= road->endPoint2.x)) ||
			((road->endPoint2.x <= point.x) && (point.x <= road->endPoint1.x));

		bool betweenY =
			((road->endPoint1.y <= point.y) && (point.y <= road->endPoint2.y)) ||
			((road->endPoint2.y <= point.y) && (point.y <= road->endPoint1.y));

		if (betweenX || betweenY) {
			float distanceSquare = DistanceSquareFromRoad(*road, point);

			if (!closestRoad || distanceSquare < minDistanceSquare) {
				closestRoad = road;
				minDistanceSquare = distanceSquare;

				result.type = MapElemRoad;
				result.road = road;
			}
		}
	}

	Intersection* closestIntersection = 0;
	for (int i = 0; i < map.intersectionCount; ++i) {
		Intersection* intersection = &map.intersections[i];

		float halfRoadWidth = GetIntersectionRoadWidth(*intersection) * 0.5f;

		bool betweenX = (Abs(intersection->position.x - point.x) <= halfRoadWidth);
		bool betweenY = (Abs(intersection->position.y - point.y) <= halfRoadWidth);

		if (betweenX || betweenY) {
			float distanceSquare = DistanceSquare(intersection->position, point);

			if ((!closestRoad && !closestIntersection) || distanceSquare < minDistanceSquare) {
				closestIntersection = intersection;
				minDistanceSquare = distanceSquare;

				result.type = MapElemIntersection;
				result.intersection = intersection;
			}
		}
	}

	return result;
}

Building* BuildingAtPoint(Map map, Point point) {
	Building* result = 0;

	for (int i = 0; i < map.buildingCount; ++i) {
		if (IsPointInBuilding(point, map.buildings[i])) 
			result = &map.buildings[i];
	}

	return result;
}

Building* RandomBuilding(Map map) {
	int buildingIndex = RandMod(map.buildingCount);

	return &map.buildings[buildingIndex];
}

Building* ClosestBuilding(Map map, Point point, BuildingType type) {
	Building* result = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];
		if (building->type != type) 
			continue;

		// TODO: make a function for this, since it is used a lot?
		Point center = {
			(building->left + building->right) * 0.5f,
			(building->top + building->bottom) * 0.5f
		};

		float distanceSquare = DistanceSquare(point, center);

		if (!result || distanceSquare < minDistanceSquare) {
			result = building;
			minDistanceSquare = distanceSquare;
		}
	}

	return result;
}

BuildingCrossInfo ClosestExtBuildingCrossInfo(Map map, float radius, Point closePoint, Point farPoint) {
	BuildingCrossInfo result = {};
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		BuildingCrossInfo crossInfo = ExtBuildingClosestCrossInfo(&map.buildings[i], radius, closePoint, farPoint);

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

Building* ClosestCrossedBuilding(Map map, Point pointClose, Point pointFar, Building* excludedBuilding) {
	Building* result = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];
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

MapElem RoadElemAtPoint(Map map, Point point) {
	MapElem result = {};
	result.type = MapElemNone;

	for (int i = 0; i < map.roadCount; ++i) {
		if (IsPointOnRoad(point, map.roads[i])) {
			result = RoadElem(&map.roads[i]);
			return result;
		}
	}

	for (int i = 0; i < map.intersectionCount; ++i) {
		if (IsPointOnIntersection(point, map.intersections[i])) {
			result = IntersectionElem(&map.intersections[i]);
			return result;
		}
	}

	for (int i = 0; i < map.buildingCount; ++i) {
		if (IsPointInBuilding(point, map.buildings[i])) {
			result = BuildingElem(&map.buildings[i]);
			return result;
		}

		if (IsPointOnBuildingConnector(point, map.buildings[i])) {
			result = BuildingConnectorElem(&map.buildings[i]);
			return result;
		}
	}

	return result;
}

MapElem PedestrianElemAtPoint(Map map, Point point) {
	MapElem result = {};
	result.type = MapElemNone;

	for (int i = 0; i < map.intersectionCount; ++i) {
		if (IsPointOnIntersectionSidewalk(point, map.intersections[i])) {
			result = IntersectionSidewalkElem(&map.intersections[i]);
			return result;
		}
	}

	for (int i = 0; i < map.roadCount; ++i) {
		if (IsPointOnRoadSidewalk(point, map.roads[i])) {
			result = RoadSidewalkElem(&map.roads[i]);
			return result;
		}

		if (IsPointOnCrossing(point, map.roads[i])) {
			result = CrossingElem(&map.roads[i]);
			return result;
		}
	}

	return result;
}

void DrawGroundElems(Renderer renderer, Map* map, GameAssets* assets) {
	Color color = Color{0.0f, 1.0f, 0.0f};
	WorldTextureRect(renderer, 0, 0, map->height, map->width, assets->grassTexture);

	for (int i = 0; i < map->intersectionCount; ++i)
		DrawIntersection(renderer, map->intersections[i], assets);

	for (int i = 0; i < map->roadCount; ++i)
		DrawRoad(renderer, map->roads[i], assets);

	for (int i = 0; i < map->buildingCount; ++i)
		DrawConnectRoad(renderer, map->buildings[i], assets->roadTexture);

	for (int i = 0; i < map->intersectionCount; ++i)
		DrawTrafficLights(renderer, map->intersections[i]);
}

// TODO: there are two instances of mergesort in the code
//       pull those two together somehow?
struct BuildingHelper {
	int buildingCount;
	Building* buildings;
	Building* tmpBuildings;
	MemArena* arena;
};

inline Point BuildingCenter(Building* building) {
	Point result = {};
	result.x = (building->left + building->right) * 0.5f;
	result.y = (building->top + building->bottom) * 0.5f;
	return result;
}

inline bool AreBuildingsInOrder(Point center, Building building1, Building building2) {
	Point point1 = BuildingCenter(&building1);
	Point point2 = BuildingCenter(&building2);

	float dist1 = CityDistance(center, point1);
	float dist2 = CityDistance(center, point2);

	if (dist1 > dist2)
		return true;
	else
		return false;
}

inline void MergeBuildingArrays(BuildingHelper* helper, Point center, int leftStart, int leftEnd, int rightStart, int rightEnd) {
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

inline void SortBuildings(BuildingHelper* helper, Point center) {
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
void DrawBuildings(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets) {
	for (int i = 0; i < map->buildingCount; ++i)
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

void DrawMap(Renderer renderer, Map* map, MemArena* arena, GameAssets* assets) {
	DrawGroundElems(renderer, map, assets);
	DrawBuildings(renderer, map, arena, assets);
}

