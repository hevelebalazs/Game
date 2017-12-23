#include "Geometry.h"
#include "Math.h"
#include "Vehicle.h"

void MoveVehicle(Vehicle* vehicle, DirectedPoint point) {
	vehicle->position = point.position;
	vehicle->angle = VectorAngle(point.direction);
}

void DrawVehicle(Renderer renderer, Vehicle vehicle) {
	Point addWidth = PointProd((vehicle.width * 0.5f), RotationVector(vehicle.angle + PI * 0.5f));

	Point side1 = PointSum(vehicle.position, addWidth);
	Point side2 = PointDiff(vehicle.position, addWidth);

	Point addLength = PointProd((vehicle.length * 0.5f), RotationVector(vehicle.angle));

	Point points[4] = {
		PointSum(side1, addLength), 
		PointDiff(side1, addLength),
		PointDiff(side2, addLength), 
		PointSum(side2, addLength)
	};

	DrawQuad(renderer, points, vehicle.color);
}