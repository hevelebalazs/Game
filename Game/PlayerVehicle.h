#pragma once

#include "Vehicle.h"

struct PlayerVehicle {
	Vehicle vehicle;

	float mass;

	float engineForce;
	float maxEngineForce;
	float breakForce;

	float turnDirection;

	Point velocity;
	Color defaultColor;

	float secondsRed;
};

void TurnPlayerVehicleRed(PlayerVehicle* playerVehicle, float seconds);
void UpdatePlayerVehicle(PlayerVehicle* playerVehicle, float seconds);