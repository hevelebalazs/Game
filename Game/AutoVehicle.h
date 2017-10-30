#pragma once
#include "Vehicle.h"

struct AutoVehicle {
	Vehicle vehicle;

	Intersection *targetIntersection;

	Intersection *onIntersection;
	Road *nextRoad;

	Road *onRoad;
	Intersection *nextIntersection;

	Point startPoint;
	float startAngle;
	Point targetPoint;
	float targetAngle;

	bool rotationMovement;
	float rotationSide;
	float rotationStartAngle;
	float rotationTargetAngle;
	Point rotationPoint = {};

	float spentSeconds;
	float totalSeconds;

	IntersectionPathHelper *pathHelper;

	void update(float seconds);
};