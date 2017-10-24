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

void Map::draw(Bitmap bitmap) {
	Color color = { 0.0f, 1.0f, 0.0f };
	bitmap.drawRect(0, 0, height, width, color);

	for (int i = 0; i < intersectionCount; ++i) {
		intersections[i].draw(bitmap);
	}

	for (int i = 0; i < roadCount; ++i) {
		roads[i].draw(bitmap);
	}

	for (int i = 0; i < buildingCount; ++i) {
		buildings[i].draw(bitmap);
	}
}

