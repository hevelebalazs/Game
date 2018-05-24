#pragma once

#include "Building.h"
#include "Map.h"
#include "Memory.h"
#include "Renderer.h"
#include "Road.h"

struct PathNode {
	// TODO: create a MapSubElem struct?
	MapElem elem;
	int subElemIndex;

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

void ResetPathPool(PathPool* pathPool);
PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool);
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* tmpArena, PathPool* pathPool);
PathNode* ConnectPedestrianElems(Map* map, MapElem startElem, int startSubIndex, MapElem endElem, int endSubIndex,
							   MemArena* arena, PathPool* pathPool);

void FreePathNode(PathNode* node, PathPool* pathPool);
void FreePath(PathNode* firstNode, PathPool* pathPool);

void DrawPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth);
void DrawBezierPathFromPoint(Renderer renderer, PathNode* firstNode, DirectedPoint startPoint, Color color, float lineWidth);
void DrawBezierPath(Renderer renderer, PathNode* firstNode, Color color, float lineWidth);