#pragma once

#include "Bezier.h"
#include "Memory.h"
#include "Path.h"
#include "Vehicle.h"

struct AutoVehicle {
	Vehicle vehicle;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	Bezier4 moveBezier4;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoVehicleToJunction(AutoVehicle* autoVehicle, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoVehicle(AutoVehicle* autoVehicle, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);