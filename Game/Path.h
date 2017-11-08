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

struct PathMemory {

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

Path ConnectBuildings(Map* map, Building* buildingStart, Building* buildingEnd);
void ClearPath(Path* path);

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth);