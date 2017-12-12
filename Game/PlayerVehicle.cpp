#include <stdio.h>
#include <windows.h>

#include "Geometry.h"
#include "PlayerVehicle.h"

void UpdatePlayerVehicle(PlayerVehicle* playerVehicle, float seconds) {
	Vehicle* vehicle = &playerVehicle->vehicle;

	float speed = VectorLength(playerVehicle->velocity);

	Point direction = RotationVector(vehicle->angle);

	Point frontWheel = PointProd(vehicle->length * 0.5f, direction);
	Point rearWheel  = PointProd(-vehicle->length * 0.5f, direction);

	Point turnDirection = RotationVector(vehicle->angle + playerVehicle->turnAngle);

	frontWheel = PointSum(frontWheel, PointProd(seconds * speed, turnDirection));
	rearWheel  = PointSum(rearWheel, PointProd(seconds * speed, direction));

	vehicle->angle = atan2f(frontWheel.y - rearWheel.y, frontWheel.x - rearWheel.x);

	float cDrag = 0.4257f;
	float cRR = 12.8f;

	Point fTraction = PointProd(playerVehicle->engineForce, direction);
	Point fDrag = PointProd(-cDrag * speed, playerVehicle->velocity);
	Point fRR = PointProd(-cRR, playerVehicle->velocity);

	Point force = PointSum(PointSum(fTraction, fDrag), fRR);

	force = PointProd((1.0f / 50.0f), force);

	playerVehicle->velocity = PointSum(playerVehicle->velocity, PointProd(seconds, force));

	Point parallel = PointProd(DotProduct(playerVehicle->velocity, direction), direction);
	Point perpendicular = PointDiff(playerVehicle->velocity, parallel);

	playerVehicle->velocity = PointSum(parallel, PointProd(0.5f, perpendicular));

	vehicle->position = PointSum(vehicle->position, PointProd(seconds, playerVehicle->velocity));

	if (playerVehicle->secondsRed > 0.0f) {
		vehicle->color = Color{1.0f, 0.0f, 0.0f};

		playerVehicle->secondsRed -= seconds;

		if (playerVehicle->secondsRed < 0.0f) {
			playerVehicle->secondsRed = 0.0f;
			vehicle->color = Color{0.0f, 0.0f, 1.0f};
		}
	}
}