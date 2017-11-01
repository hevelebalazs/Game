#include "PlayerVehicle.h"

#include <stdio.h>
#include <windows.h>

void PlayerVehicle::update(float seconds) {
	vehicle.angle += turnAngle * seconds;

	float cDrag = 0.4257f;
	float cRR = 12.8f;

	Point oldVelocity = velocity;

	Point direction = Point::rotation(vehicle.angle);
	Point fTraction = direction * engineForce;
	Point fDrag = (-cDrag * velocity.length()) * velocity;
	Point fRR = (-cRR) * velocity;

	Point force = fTraction + fDrag + fRR;

	velocity += ((seconds / 100.0f) * force);
	vehicle.position += (seconds * velocity);
}