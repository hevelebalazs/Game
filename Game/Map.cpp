#include "Map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Intersection* Map::getRandomIntersection() {
	int intersectionIndex = rand() % intersectionCount;

	return &intersections[intersectionIndex];
}

Intersection* Map::getIntersectionAtPoint(Point point, float maxDistance) {
	float maxDistanceSquare = maxDistance * maxDistance;

	Intersection *result = 0;

	for (int i = 0; i < intersectionCount; ++i) {
		float distanceSquare = Point::distanceSquare(point, intersections[i].coordinate);

		if (distanceSquare <= maxDistanceSquare) result = &intersections[i];
	}

	return result;
};

Road *Map::closestRoad(Point point) {
	Road *closestRoad = 0;
	float minDistanceSquare = 0.0f;

	for (int i = 0; i < roadCount; ++i) {
		Road *road = &roads[i];
		float distanceSquare = road->distanceSquareFrom(point);

		if (!closestRoad || distanceSquare < minDistanceSquare) {
			closestRoad = road;
			minDistanceSquare = distanceSquare;
		}
	}

	return closestRoad;
}

void Map::draw(Renderer renderer) {
	Color color = { 0.0f, 1.0f, 0.0f };
	renderer.drawRect(0, 0, height, width, color);

	for (int i = 0; i < intersectionCount; ++i) {
		intersections[i].draw(renderer);
	}

	for (int i = 0; i < roadCount; ++i) {
		roads[i].draw(renderer);
	}

	for (int i = 0; i < buildingCount; ++i) {
		buildings[i].draw(renderer);
	}
}

