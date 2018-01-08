#pragma once

#include "Human.h"
#include "Memory.h"
#include "Path.h"

struct AutoHuman {
	Human human;

	Intersection* onIntersection;

	PathNode* moveNode;
	Intersection* moveTargetIntersection;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoHumanToIntersection(AutoHuman* autoHuman, Intersection* intersection, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void MoveAutoHumanToBuilding(AutoHuman* autoHuman, Building* building, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);