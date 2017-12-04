#pragma once
#include "Renderer.h"
#include "Intersection.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"

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

void MoveVehicle(Vehicle* vehicle, DirectedPoint point);
void DrawVehicle(Renderer renderer, Vehicle vehicle);