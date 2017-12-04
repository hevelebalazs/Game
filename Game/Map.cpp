#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "Geometry.h"
#include "Map.h"

Intersection* RandomIntersection(Map map) {
	int intersectionIndex = rand() % map.intersectionCount;

	return &map.intersections[intersectionIndex];
}

Intersection* IntersectionAtPoint(Map map, Point point, float maxDistance) {
	float maxDistanceSquare = maxDistance * maxDistance;

	Intersection* result = 0;

	for (int i = 0; i < map.intersectionCount; ++i) {
		float distanceSquare = DistanceSquare(point, map.intersections[i].coordinate);

		if (distanceSquare <= maxDistanceSquare) result = &map.intersections[i];
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

		bool betweenX = (fabsf(intersection->coordinate.x - point.x) <= halfRoadWidth);
		bool betweenY = (fabsf(intersection->coordinate.y - point.y) <= halfRoadWidth);

		if (betweenX || betweenY) {
			float distanceSquare = DistanceSquare(intersection->coordinate, point);

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
		if (IsPointInBuilding(point, map.buildings[i])) result = &map.buildings[i];
	}

	return result;
}

Building* RandomBuilding(Map map) {
	int buildingIndex = rand() % map.buildingCount;

	return &map.buildings[buildingIndex];
}

Building* ClosestBuilding(Map map, Point point, BuildingType type) {
	Building* result = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < map.buildingCount; ++i) {
		Building* building = &map.buildings[i];
		if (building->type != type) continue;

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

		if (building == excludedBuilding) continue;

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

void DrawMap(Renderer renderer, Map map) {
	Color color = { 0.0f, 1.0f, 0.0f };
	DrawRect(
		renderer,
		0, 0, map.height, map.width, 
		color
	);

	for (int i = 0; i < map.intersectionCount; ++i) {
		DrawIntersection(renderer, map.intersections[i]);
	}

	for (int i = 0; i < map.roadCount; ++i) {
		DrawRoad(renderer, map.roads[i]);
	}

	for (int i = 0; i < map.buildingCount; ++i) {
		DrawConnectRoad(renderer, map.buildings[i]);
		DrawBuilding(renderer, map.buildings[i]);
	}

	for (int i = 0; i < map.intersectionCount; ++i) {
		DrawTrafficLights(renderer, map.intersections[i]);
	}
}

