#include "PlayerVehicle.h"

#include <stdio.h>
#include <windows.h>

void PlayerVehicle::Update(float seconds) {
	Point direction = Point::Rotation(vehicle.angle);

	Point frontWheel = (vehicle.length / 2.0f) * direction;
	Point rearWheel = (-vehicle.length / 2.0f) * direction;

	Point turnDirection = Point::Rotation(vehicle.angle + turnAngle);

	frontWheel += turnDirection * seconds * velocity.Length();
	rearWheel += direction * seconds * velocity.Length();

	vehicle.angle = atan2f(frontWheel.y - rearWheel.y, frontWheel.x - rearWheel.x);

	float cDrag = 0.4257f;
	float cRR = 12.8f;

	Point fTraction = direction * engineForce;
	Point fDrag = (-cDrag * velocity.Length()) * velocity;
	Point fRR = (-cRR) * velocity;

	Point force = fTraction + fDrag + fRR;

	force = (1.0f / 50.0f) * force;

	velocity += (seconds * force);

	Point parallel = Point::DotProduct(velocity, direction) * direction;
	Point perpendicular = velocity - parallel;

	velocity = parallel + 0.95f * perpendicular;

	vehicle.position += (seconds * velocity);
}