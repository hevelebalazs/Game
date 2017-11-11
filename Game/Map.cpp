#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Intersection* Map::GetRandomIntersection() {
	int intersectionIndex = rand() % intersectionCount;

	return &intersections[intersectionIndex];
}

Intersection* Map::GetIntersectionAtPoint(Point point, float maxDistance) {
	float maxDistanceSquare = maxDistance * maxDistance;

	Intersection* result = 0;

	for (int i = 0; i < intersectionCount; ++i) {
		float distanceSquare = Point::DistanceSquare(point, intersections[i].coordinate);

		if (distanceSquare <= maxDistanceSquare) result = &intersections[i];
	}

	return result;
};

// TODO: make this so that it returns an intersection if it is closer than the closest point
Road* Map::ClosestRoad(Point point) {
	Road* closestRoad = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < roadCount; ++i) {
		Road* road = &roads[i];
		float distanceSquare = road->DistanceSquareFrom(point);

		if (!closestRoad || distanceSquare < minDistanceSquare) {
			closestRoad = road;
			minDistanceSquare = distanceSquare;
		}
	}

	return closestRoad;
}

Building* Map::GetBuildingAtPoint(Point point) {
	Building* result = 0;

	for (int i = 0; i < buildingCount; ++i) {
		Building* building = &buildings[i];

		if (building->IsPointInside(point)) result = building;
	}

	return result;
}
 
// TODO: if there are severeal buildings, return the one closest to one of the points
Building* Map::CrossedBuilding(Point point1, Point point2, Building *excludedBuilding) {
	for (int i = 0; i < buildingCount; ++i) {
		Building *building = &buildings[i];

		if (building == excludedBuilding) continue;

		if (building->IsCrossed(point1, point2)) {
			return building;
		}
	}

	return 0;
}

void Map::Draw(Renderer renderer) {
	Color color = { 0.0f, 1.0f, 0.0f };
	renderer.DrawRect(0, 0, height, width, color);

	for (int i = 0; i < intersectionCount; ++i) {
		intersections[i].Draw(renderer);
	}

	for (int i = 0; i < roadCount; ++i) {
		roads[i].Draw(renderer);
	}

	for (int i = 0; i < buildingCount; ++i) {
		buildings[i].Draw(renderer);
	}
}

