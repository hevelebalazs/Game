#pragma once

#include "Bezier.h"
#include "Memory.h"
#include "Path.h"
#include "Vehicle.h"

struct AutoVehicle {
	Vehicle vehicle;

	Building* inBuilding;

	PathNode* moveNode;
	Building* moveTargetBuilding;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	Bezier4 moveBezier4;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoVehicleToBuilding(AutoVehicle* autoVehicle, Building* building, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoVehicle(AutoVehicle* autoVehicle, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);