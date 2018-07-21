#pragma once

#include "Bezier.hpp"
#include "Bitmap.hpp"
#include "Draw.hpp"
#include "Map.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Type.hpp"

#define MinCarSpeed	10.0f
#define MaxCarSpeed	16.0f
#define MaxPlayerCarSpeed 25.0f
#define MaxCarEngineForce 1000.0f
#define MaxCarBrakeForce 1000.0f

struct Car {
	V2 position;
	F32 angle;
	F32 length;
	F32 width;

	F32 moveSpeed;
	F32 maxSpeed;

	Map* map;
	Bitmap* bitmap;
};

struct AutoCar {
	Car car;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	V4 moveStartPoint;
	V4 moveEndPoint;
	Bezier4 moveBezier4;
	F32 bezierRatio;

	F32 acceleration;
};

Quad GetCarStopArea(Car* car);
B32 IsCarOnPoint(Car* car, V2 point);
void MoveCar(Car* car, V4 point);
void DrawCar(Canvas canvas, Car car);

void AllocateCarBitmap(Bitmap* carBitmap);
void GenerateCarBitmap(Bitmap* carBitmap, MemArena* tmpArena);

void MoveAutoCarToJunction(AutoCar* autoCar, Junction* junction, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoCar(AutoCar* autoCar, F32 seconds, MemArena* tmpArena, PathPool* pathPool);

struct PlayerCar {
	Car car;

	F32 engineForce;
	F32 breakForce;

	F32 turnDirection;

	V2 velocity;
};

void UpdatePlayerCar(PlayerCar* playerCar, F32 seconds);