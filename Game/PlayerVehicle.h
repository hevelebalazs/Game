#pragma once

#include "Vehicle.h"

struct PlayerVehicle;

struct PlayerVehicle {
	Vehicle vehicle;

	float mass;

	float engineForce;
	float maxEngineForce;
	float breakForce;

	float turnAngle;

	Point velocity;

	void Update(float seconds);
};