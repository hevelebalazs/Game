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

Road* Map::ClosestRoad(Point point) {
	Road* closestRoad = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < roadCount; ++i) {
		Road* road = &roads[i];

		bool betweenX =
			((road->endPoint1.x <= point.x) && (point.x <= road->endPoint2.x)) ||
			((road->endPoint2.x <= point.x) && (point.x <= road->endPoint1.x));

		bool betweenY =
			((road->endPoint1.y <= point.y) && (point.y <= road->endPoint2.y)) ||
			((road->endPoint2.y <= point.y) && (point.y <= road->endPoint1.y));

		if (betweenX || betweenY) {
			float distanceSquare = road->DistanceSquareFrom(point);

			if (!closestRoad || distanceSquare < minDistanceSquare) {
				closestRoad = road;
				minDistanceSquare = distanceSquare;
			}
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
 
Building* Map::ClosestCrossedBuilding(Point pointClose, Point pointFar, Building *excludedBuilding) {
	Building* result = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < buildingCount; ++i) {
		Building *building = &buildings[i];

		if (building == excludedBuilding) continue;

		if (building->IsCrossed(pointClose, pointFar)) {
			Point closestCrossPoint = building->ClosestCrossPoint(pointClose, pointFar);

			float distanceSquare = Point::DistanceSquare(pointClose, closestCrossPoint);

			if (result == 0 || distanceSquare < minDistanceSquare) {
				result = building;
				minDistanceSquare = distanceSquare;
			}
		}
	}

	return result;
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

