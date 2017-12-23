#pragma once

#include "Building.h"
#include "Intersection.h"
#include "Map.h"
#include "Renderer.h"
#include "Road.h"

struct PathNode {
	MapElem elem;
	PathNode* next;
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

DirectedPoint StartNodePoint(PathNode* node);
DirectedPoint NextNodePoint(PathNode* node, DirectedPoint startPoint);
bool IsNodeEndPoint(PathNode* node, DirectedPoint point);

PathHelper PathHelperForMap(Map* map);

Path ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper);

void ClearPath(Path* path);

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth);
void DrawBezierPath(Path* path, Renderer renderer, Color color, float lineWidth);