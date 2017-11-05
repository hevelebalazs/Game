#include "PlayerVehicle.h"

#include <stdio.h>
#include <windows.h>

void PlayerVehicle::update(float seconds) {
	// vehicle.angle += turnAngle * seconds;
	Point direction = Point::rotation(vehicle.angle);

	Point frontWheel = (vehicle.length / 2.0f) * direction;
	Point rearWheel = (-vehicle.length / 2.0f) * direction;

	Point turnDirection = Point::rotation(vehicle.angle + turnAngle);

	frontWheel += turnDirection * seconds * velocity.length();
	rearWheel += direction * seconds * velocity.length();

	vehicle.angle = atan2f(frontWheel.y - rearWheel.y, frontWheel.x - rearWheel.x);

	float cDrag = 0.4257f;
	float cRR = 12.8f;

	Point fTraction = direction * engineForce;
	Point fDrag = (-cDrag * velocity.length()) * velocity;
	Point fRR = (-cRR) * velocity;

	Point force = fTraction + fDrag + fRR;

	force = (1.0f / 50.0f) * force;

	velocity += (seconds * force);

	Point parallel = Point::dotProduct(velocity, direction) * direction;
	Point perpendicular = velocity - parallel;

	velocity = parallel + 0.95f * perpendicular;

	vehicle.position += (seconds * velocity);
}