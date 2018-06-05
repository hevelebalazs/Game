#pragma once

#include "Bezier.h"
#include "Bitmap.h"
#include "Map.h"
#include "Memory.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

extern float MinCarSpeed;
extern float MaxCarSpeed;

struct Car {
	Point position;
	float angle;
	float length;
	float width;

	float moveSpeed;
	float maxSpeed;

	Map* map;
	Bitmap* bitmap;
};

Quad GetCarStopArea(Car* car);
bool IsCarOnPoint(Car* car, Point point);
void MoveCar(Car* car, DirectedPoint point);
void DrawCar(Renderer renderer, Car car);

void AllocateCarBitmap(Bitmap* carBitmap);
void GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena);

struct AutoCar {
	Car car;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	Bezier4 moveBezier4;
	float moveTotalSeconds;
	float moveSeconds;
};

void MoveAutoCarToJunction(AutoCar* autoCar, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoCar(AutoCar* autoCar, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

struct PlayerCar {
	Car car;

	float mass;

	float engineForce;
	float maxEngineForce;
	float breakForce;

	float turnDirection;

	Point velocity;
};

void UpdatePlayerCar(PlayerCar* playerCar, float seconds);