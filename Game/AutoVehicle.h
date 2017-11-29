#pragma once

#include "Bezier.h"
#include "Path.h"
#include "Vehicle.h"

struct AutoVehicle {
	Vehicle vehicle;

	Building* inBuilding;

	Path movePath;
	// TODO: does this belong to this struct?
	//       or should it be passed to Update?
	PathHelper* moveHelper;
	PathNode* moveNode;
	Building* moveTargetBuilding;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	Bezier4 moveBezier4;
	float moveTotalSeconds;
	float moveSeconds;

	void MoveToBuilding(Building* building);
	void InitMovement();

	void Update(float seconds);
};