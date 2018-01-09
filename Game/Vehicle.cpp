#include "Geometry.h"
#include "Math.h"
#include "Vehicle.h"

struct VehicleCorners {
	Point corners[4];
};

static inline VehicleCorners GetVehicleCorners(Vehicle* vehicle) {
	VehicleCorners result = {};

	Point addWidth = PointProd((vehicle->width * 0.5f), RotationVector(vehicle->angle + PI * 0.5f));

	Point side1 = PointSum(vehicle->position, addWidth);
	Point side2 = PointDiff(vehicle->position, addWidth);

	Point addLength = PointProd((vehicle->length * 0.5f), RotationVector(vehicle->angle));

	result.corners[0] = PointSum(side1, addLength);
	result.corners[1] = PointDiff(side1, addLength);
	result.corners[2] = PointDiff(side2, addLength); 
	result.corners[3] = PointSum(side2, addLength);
	return result;
}

bool IsVehicleOnPoint(Vehicle* vehicle, Point point) {
	VehicleCorners vehicleCorners = GetVehicleCorners(vehicle);
	bool result = IsPointInQuad(vehicleCorners.corners, point);
	return result;
}

void MoveVehicle(Vehicle* vehicle, DirectedPoint point) {
	vehicle->position = point.position;
	vehicle->angle = VectorAngle(point.direction);
}

void DrawVehicle(Renderer renderer, Vehicle vehicle) {
	VehicleCorners vehicleCorners = GetVehicleCorners(&vehicle);
	DrawQuad(renderer, vehicleCorners.corners, vehicle.color);
}