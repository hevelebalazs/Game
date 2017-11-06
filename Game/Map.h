#pragma once

#include "Intersection.h"
#include "Building.h"
#include "Renderer.h"

struct Map
{
	Intersection* intersections;
	int intersectionCount = 0;

	Road* roads;
	int roadCount = 0;

	Building *buildings;
	int buildingCount = 0;

	float width = 0;
	float height = 0;

	Intersection *getRandomIntersection();
	Intersection* getIntersectionAtPoint(Point point, float maxDistance);

	Road *closestRoad(Point point);

	void draw(Renderer renderer);
};