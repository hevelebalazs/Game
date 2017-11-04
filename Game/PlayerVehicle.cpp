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

	force = (1.0f / 50.0f) * force;

	velocity += (seconds * force);

	Point angleRot = Point::rotation(vehicle.angle);
	Point parallel = dotProduct(velocity, angleRot) * angleRot;
	Point perpendicular = velocity - parallel;

	velocity = parallel + 0.5f * perpendicular;

	vehicle.position += (seconds * velocity);
}