#pragma once

#include "Building.h"
#include "BuildingType.h"
#include "Intersection.h"
#include "MapElem.h"
#include "Renderer.h"

struct Map {
	Intersection* intersections;
	int intersectionCount = 0;

	Road* roads;
	int roadCount = 0;

	Building* buildings;
	int buildingCount = 0;

	float width = 0;
	float height = 0;
};

Intersection* RandomIntersection(Map map);
Intersection* IntersectionAtPoint(Map map, Point point, float maxDistance);

MapElem ClosestRoadOrIntersection(Map map, Point point);

Building* RandomBuilding(Map map);
Building* ClosestBuilding(Map map, Point point, BuildingType type);
BuildingCrossInfo ClosestExtBuildingCrossInfo(Map map, float radius, Point closePoint, Point farPoint);
Building* ClosestCrossedBuilding(Map map, Point point1, Point point2, Building* excludedBuilding);
Building* BuildingAtPoint(Map map, Point point);

MapElem RoadElemAtPoint(Map map, Point point);
MapElem PedestrianElemAtPoint(Map map, Point point);

void DrawMap(Renderer renderer, Map map);