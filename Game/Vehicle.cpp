#include "Geometry.h"
#include "Math.h"
#include "Vehicle.h"

extern float MinVehicleSpeed = 15.0f;
extern float MaxVehicleSpeed = 30.0f;

// TODO: use a vehicle coordinate system?
static inline Quad GetVehicleCorners(Vehicle* vehicle) {
	Quad result = {};

	Point addWidth = PointProd((vehicle->width * 0.5f), RotationVector(vehicle->angle + PI * 0.5f));

	Point side1 = PointSum(vehicle->position, addWidth);
	Point side2 = PointDiff(vehicle->position, addWidth);

	Point addLength = PointProd((vehicle->length * 0.5f), RotationVector(vehicle->angle));

	result.points[0] = PointSum(side1, addLength);
	result.points[1] = PointDiff(side1, addLength);
	result.points[2] = PointDiff(side2, addLength); 
	result.points[3] = PointSum(side2, addLength);
	return result;
}

Quad GetVehicleStopArea(Vehicle* vehicle) {
	Point addWidth = PointProd((vehicle->width * 0.5f), RotationVector(vehicle->angle + PI * 0.5f));

	Point side1 = PointSum(vehicle->position, addWidth);
	Point side2 = PointDiff(vehicle->position, addWidth);

	float stopDistance = 5.0f;

	Point addClose = PointProd((vehicle->length * 0.5f), RotationVector(vehicle->angle));
	Point addFar = PointProd((vehicle->length * 0.5f + stopDistance), RotationVector(vehicle->angle));

	Quad result = {};
	result.points[0] = PointSum(side1, addClose);
	result.points[1] = PointSum(side2, addClose);
	result.points[2] = PointSum(side2, addFar); 
	result.points[3] = PointSum(side1, addFar);

	return result;
}

bool IsVehicleOnPoint(Vehicle* vehicle, Point point) {
	Quad vehicleCorners = GetVehicleCorners(vehicle);
	bool result = IsPointInQuad(vehicleCorners, point);
	return result;
}

void MoveVehicle(Vehicle* vehicle, DirectedPoint point) {
	vehicle->position = point.position;
	vehicle->angle = VectorAngle(point.direction);
}

void DrawVehicle(Renderer renderer, Vehicle vehicle) {
	Quad vehicleCorners = GetVehicleCorners(&vehicle);
	DrawQuad(renderer, vehicleCorners, vehicle.color);
}