#include <stdio.h>
#include <windows.h>

#include "Geometry.h"
#include "PlayerVehicle.h"

void PlayerVehicle::Update(float seconds) {
	float speed = VectorLength(velocity);

	Point direction = RotationVector(vehicle.angle);

	Point frontWheel = PointProd(vehicle.length * 0.5f, direction);
	Point rearWheel = PointProd(-vehicle.length * 0.5f, direction);

	Point turnDirection = RotationVector(vehicle.angle + turnAngle);

	frontWheel = PointSum(frontWheel, PointProd(seconds * speed, turnDirection));
	rearWheel = PointSum(rearWheel, PointProd(seconds * speed, direction));

	vehicle.angle = atan2f(frontWheel.y - rearWheel.y, frontWheel.x - rearWheel.x);

	float cDrag = 0.4257f;
	float cRR = 12.8f;

	Point fTraction = PointProd(engineForce, direction);
	Point fDrag = PointProd(-cDrag * speed, velocity);
	Point fRR = PointProd(-cRR, velocity);

	Point force = PointSum(PointSum(fTraction, fDrag), fRR);

	force = PointProd((1.0f / 50.0f), force);

	velocity = PointSum(velocity, PointProd(seconds, force));

	Point parallel = PointProd(DotProduct(velocity, direction), direction);
	Point perpendicular = PointDiff(velocity, parallel);

	velocity = PointSum(parallel, PointProd(0.95f, perpendicular));

	vehicle.position = PointSum(vehicle.position, PointProd(seconds, velocity));
}