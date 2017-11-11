#pragma once
#include "Map.h"
#include "Road.h"
#include "Intersection.h"
#include "Building.h"
#include "Renderer.h"

enum PathNodeType {
	PATH_NODE_ROAD,
	PATH_NODE_INTERSECTION,
	PATH_NODE_BUILDING
};

// TODO: should this be a linked list element?
struct PathNode {
	PathNodeType type;

	union {
		Road* road;
		Intersection* intersection;
		Building* building;
	};
};

struct Path {
	int nodeCount;
	PathNode *nodes;
};

struct PathHelper {
	PathNode* nodes;
	int nodeCount;

	int* isIntersectionHelper;
	int* isRoadHelper;
	int* sourceIndex;
};

PathHelper PathHelperForMap(Map* map);

Path ConnectBuildings(Map* map, Building* buildingStart, Building* buildingEnd, PathHelper* helper);
void ClearPath(Path* path);

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth);