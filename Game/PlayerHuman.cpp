#include "Geometry.h"
#include "Human.h"
#include "PlayerHuman.h"
#include "Point.h"

static void MoveHuman(Human* human, Point moveVector) {
	Map* map = human->map;

	float distanceToGo = moveVector.Length();

	// TODO: should there be a limit on the iteration number?
	while (distanceToGo > 0.0f) {
		Point pointToGo = human->position + moveVector;

		// TODO: create a function that returns all collision info (building, wall, intersection point)
		BuildingCrossInfo crossInfo = human->map->ClosestExtBuildingCrossInfo(human->position, pointToGo, human->radius);

		Building* crossedBuilding = crossInfo.building;

		if (crossedBuilding) {
			Point intersectionPoint = crossedBuilding->ClosestCrossPoint(human->position, pointToGo);

			// TODO: create a proper collision system
			float distanceTaken = PointDistance(human->position, intersectionPoint);

			Point moveNormal = moveVector * (1.0f / moveVector.Length());

			Point wallDirection = crossInfo.corner1 - crossInfo.corner2;
			moveVector = ParallelVector(moveVector, wallDirection);

			distanceToGo = moveVector.Length();
		}
		else {
			human->position = pointToGo;
			distanceToGo = 0.0f;
		}
	}
}

void PlayerHuman::Update(float seconds) {
	moveDirection = Point {0.0f, 0.0f};

	bool moveX = false;
	bool moveY = false;

	if (moveLeft) {
		moveDirection.x = -1.0f;
		moveX = true;
	}

	if (moveRight) {
		moveDirection.x = 1.0f;
		moveX = true;
	}

	if (moveUp) {
		moveDirection.y = -1.0f;
		moveY = true;
	}

	if (moveDown) {
		moveDirection.y = 1.0f;
		moveY = true;
	}

	if (moveX && moveY) moveDirection = moveDirection * (1.0f / sqrtf(2.0f));

	Point moveVector = (Human::moveSpeed * seconds) * moveDirection;

	MoveHuman(&human, moveVector);
}