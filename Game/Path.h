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

PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool);
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

void FreePathNode(PathNode* node, PathPool* pathPool);
void FreePath(PathNode* firstNode, PathPool* pathPool);

void DrawPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth);
void DrawBezierPathFromPoint(Renderer renderer, PathNode* firstNode, DirectedPoint startPoint, Color color, float lineWidth);
void DrawBezierPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth);