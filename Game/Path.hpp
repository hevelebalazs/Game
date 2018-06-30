#pragma once

#include "Building.hpp"
#include "Draw.hpp"
#include "Map.hpp"
#include "Memory.hpp"
#include "Road.hpp"
#include "Type.hpp"

struct PathNode {
	// TODO: create a MapSubElem struct?
	MapElem elem;
	I32 subElemIndex;

	PathNode* next;
};

struct PathPool {
	PathNode* nodes;
	I32 nodeCount;
	I32 maxNodeCount;
	PathNode* firstFreeNode;
};

V4 StartNodePoint(PathNode* node);
V4 NextNodePoint(PathNode* node, V4 startPoint);
B32 IsNodeEndPoint(PathNode* node, V4 point);

void ResetPathPool(PathPool* pathPool);
PathNode* PrefixPath(MapElem elem, PathNode* firstNode, PathPool* pathPool);
PathNode* ConnectElems(Map* map, MapElem elemStart, MapElem elemEnd, MemArena* tmpArena, PathPool* pathPool);
PathNode* ConnectPedestrianElems(Map* map, MapElem startElem, I32 startSubIndex, MapElem endElem, I32 endSubIndex,
							   MemArena* arena, PathPool* pathPool);

void FreePathNode(PathNode* node, PathPool* pathPool);
void FreePath(PathNode* firstNode, PathPool* pathPool);

void DrawPath(Canvas canvas, PathNode* firstNode, V4 color, F32 lineWidth);
void DrawBezierPathFromPoint(Canvas canvas, PathNode* firstNode, V4 startPoint, V4 color, F32 lineWidth);
void DrawBezierPath(Canvas canvas, PathNode* firstNode, V4 color, F32 lineWidth);
