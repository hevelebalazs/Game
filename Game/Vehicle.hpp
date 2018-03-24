#pragma once

#include "Intersection.hpp"
#include "Map.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "Renderer.hpp"

// TODO: capitalize all globals
extern float MinVehicleSpeed;
extern float MaxVehicleSpeed;

struct Vehicle {
	Point position;
	
	// TODO: change this to Point "direction"?
	float angle;

	Color color;

	float length;
	float width;

	float moveSpeed;
	float maxSpeed;

	Map* map;
};

Quad GetVehicleStopArea(Vehicle* vehicle);
bool IsVehicleOnPoint(Vehicle* vehicle, Point point);
void MoveVehicle(Vehicle* vehicle, DirectedPoint point);
void DrawVehicle(Renderer renderer, Vehicle vehicle);