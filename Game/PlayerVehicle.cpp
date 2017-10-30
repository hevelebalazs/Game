#include "PlayerVehicle.h"

void PlayerVehicle::update(float seconds) {
	vehicle.angle += turnAngle * seconds;

	velocity = speed * Point::rotation(vehicle.angle);

	vehicle.position += (seconds * velocity);

	if (speed != 0.0f) {
		int x = 10;
	}
}