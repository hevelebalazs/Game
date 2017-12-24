#pragma once

#include "Human.h"
#include "Memory.h"
#include "Path.h"

struct AutoHuman {
	Human human;

	float needRedSpeed;
	float needGreenSpeed;
	float needBlueSpeed;

	PathNode* moveNode;
	Building* moveTargetBuilding;
	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoHumanToBuilding(AutoHuman* autoHuman, Building* building, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);