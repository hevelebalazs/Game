#pragma once

#include "Human.h"

struct AutoHuman {
	Human human;

	float needRedSpeed;
	float needGreenSpeed;
	float needBlueSpeed;

	Building* inBuilding;

	Path movePath;
	// TODO: does this belong to this struct?
	//       or should it be passed to Update?
	PathHelper* moveHelper;
	PathNode* moveNode;
	Building* moveTargetBuilding;
	Point moveStartPoint;
	Point moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;

	void MoveToBuilding(Building* building);
	void InitMovement();

	void Update(float seconds);
};