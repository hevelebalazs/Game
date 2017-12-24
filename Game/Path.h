#pragma once

#include "Building.h"
#include "Intersection.h"
#include "Map.h"
#include "Memory.h"
#include "Renderer.h"
#include "Road.h"

struct PathNode {
	MapElem elem;
	PathNode* next;
};

struct PathPool {
	PathNode* nodes;
	int nodeCount;
	int maxNodeCount;
	PathNode* firstFreeNode;
};

DirectedPoint StartNodePoint(PathNode* node);
DirectedPoint NextNodePoint(PathNode* node, DirectedPoint startPoint);
bool IsNodeEndPoint(PathNode* node, DirectedPoint point);

PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

void FreePathNode(PathNode* node, PathPool* pathPool);
void FreePath(PathNode* firstNode, PathPool* pathPool);

void DrawPath(PathNode* firstNode, Renderer renderer, Color color, float lineWidth);
void DrawBezierPath(PathNode* firstNode, Renderer renderer, Color color, float lineWidth);