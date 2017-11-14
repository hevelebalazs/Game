#pragma once

#include "Intersection.h"
#include "Building.h"
#include "Renderer.h"
#include "MapElem.h"

struct Map {
	Intersection* intersections;
	int intersectionCount = 0;

	Road* roads;
	int roadCount = 0;

	Building* buildings;
	int buildingCount = 0;

	float width = 0;
	float height = 0;

	Intersection* GetRandomIntersection();
	Intersection* GetIntersectionAtPoint(Point point, float maxDistance);

	MapElem ClosestRoadOrIntersection(Point point);

	Building* GetRandomBuilding();
	Building* ClosestCrossedBuilding(Point point1, Point point2, Building* excludedBuilding);
	Building* GetBuildingAtPoint(Point point);

	void Draw(Renderer renderer);
};