#pragma once
#include "Map.h"
#include "Road.h"
#include "Intersection.h"
#include "Building.h"
#include "Renderer.h"

// TODO: should this be a linked list element?
struct PathNode {
	MapElem elem;

	PathNode* next;

	Point StartPoint();
	Point NextPoint(Point pointFrom);
	DirectedPoint StartDirectedPoint();
	DirectedPoint NextDirectedPoint(DirectedPoint pointFrom);

	bool IsEndPoint(Point point);
	bool IsEndDirectedPoint(DirectedPoint point);
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

Path ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, PathHelper* helper);

void ClearPath(Path* path);

void DrawPath(Path* path, Renderer renderer, Color color, float lineWidth);
void DrawBezierPath(Path* path, Renderer renderer, Color color, float lineWidth);