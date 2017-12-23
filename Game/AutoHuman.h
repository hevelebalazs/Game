#pragma once

#include "Human.h"

struct AutoHuman {
	Human human;

	float needRedSpeed;
	float needGreenSpeed;
	float needBlueSpeed;

	Path movePath;
	// TODO: does this belong to this struct?
	//       or should it be passed to Update?
	PathHelper* moveHelper;
	PathNode* moveNode;
	Building* moveTargetBuilding;
	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoHumanToBuilding(AutoHuman* autoHuman, Building* building);