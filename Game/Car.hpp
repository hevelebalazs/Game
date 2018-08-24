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

#define CarCornerMass 250000
#define CarMass (4 * CarCornerMass)

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

Quad GetCarCorners(Car* car);
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

	V2 velocity;
	V2 acceleration;

	F32 angularVelocity;
	F32 angularAcceleration;
	F32 inertia;

	F32 engineForce;
	F32 breakForce;

	F32 frontWheelAngle;
	F32 frontWheelAngleTarget;
};

V2 GetCarCorner(Car* car, I32 cornerIndex);
void UpdatePlayerCarWithoutCollision(PlayerCar* car, F32 seconds);

struct CollisionInfo {
	I32 count;
	V2 point;
	V2 normalVector;
};

CollisionInfo operator+(CollisionInfo hit1, CollisionInfo hit2);
CollisionInfo GetCarLineCollisionInfo(Car* oldCar, Car* newCar, V2 point1, V2 point2);
V2 GetCarRelativeCoordinates(Car* car, V2 point);
CollisionInfo GetCarPointCollisionInfo(Car* oldCar, Car* newCar, V2 point);
CollisionInfo GetCarPolyCollisionInfo(Car* oldCar, Car* newCar, V2* points, I32 pointN);
V2 GetCarPointVelocity(PlayerCar* car, V2 point);
void UpdatePlayerCarCollision(PlayerCar* car, PlayerCar* oldCar, F32 seconds, CollisionInfo hit);