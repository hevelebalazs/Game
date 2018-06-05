#pragma once

#include "Bitmap.h"
#include "Map.h"
#include "Memory.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

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
	Bitmap* bitmap;
};

Quad GetVehicleStopArea(Vehicle* vehicle);
bool IsVehicleOnPoint(Vehicle* vehicle, Point point);
void MoveVehicle(Vehicle* vehicle, DirectedPoint point);
void DrawVehicle(Renderer renderer, Vehicle vehicle);

void AllocateCarBitmap(Bitmap* carBitmap);
void GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena);