#pragma once

#include "Vehicle.h"

struct PlayerVehicle {
	Vehicle vehicle;

	float turnAngle = 0.0f;

	float speed;
	Point velocity;

	void update(float seconds);
};