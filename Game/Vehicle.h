#pragma once

#include "Intersection.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

struct Vehicle {
	Point position;
	
	// TODO: change this to Point "direction"?
	float angle;

	Color color;

	float length;
	float width;

	float maxSpeed;

	Map* map;
};

bool IsVehicleOnPoint(Vehicle* vehicle, Point point);
void MoveVehicle(Vehicle* vehicle, DirectedPoint point);
void DrawVehicle(Renderer renderer, Vehicle vehicle);