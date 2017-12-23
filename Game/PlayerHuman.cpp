#include "Geometry.h"
#include "Human.h"
#include "Math.h"
#include "PlayerHuman.h"
#include "Point.h"

static void MoveHuman(Human* human, Point moveVector) {
	Map* map = human->map;

	float distanceToGo = VectorLength(moveVector);

	// TODO: should there be a limit on the iteration number?
	while (distanceToGo > 0.0f) {
		bool go = true;
		bool isTouchingLine = false;
		BuildingCrossInfo crossInfo = {};
		crossInfo.type = CrossNone;

		Point pointToGo = PointSum(human->position, moveVector);

		if (human->inBuilding) {
			bool isInBuilding = IsPointInExtBuilding(human->position, *human->inBuilding, humanRadius);

			if (isInBuilding)
				crossInfo = ExtBuildingInsideClosestCrossInfo(human->inBuilding, humanRadius, human->position, pointToGo);
			else
				human->inBuilding = 0;
		}
		else {
			crossInfo = ClosestExtBuildingCrossInfo(*human->map, humanRadius, human->position, pointToGo);

			if (crossInfo.type == CrossEntrance)
				human->inBuilding = crossInfo.building;
		}

		if (crossInfo.type == CrossWall) {
			Building* crossedBuilding = crossInfo.building;

			Point crossPoint = crossInfo.crossPoint;

			// TODO: create a proper collision system
			float distanceTaken = Distance(human->position, crossPoint);

			Point moveNormal = PointProd(1.0f / VectorLength(moveVector), moveVector);

			Point wallDirection = PointDiff(crossInfo.corner1, crossInfo.corner2);
			moveVector = ParallelVector(moveVector, wallDirection);

			distanceToGo = VectorLength(moveVector);

			go = false;
		}

		if (go) {
			human->position = pointToGo;
			distanceToGo = 0.0f;
		}
	}
}

void UpdatePlayerHuman(PlayerHuman* playerHuman, float seconds) {
	playerHuman->moveDirection = Point {0.0f, 0.0f};

	bool moveX = false;
	bool moveY = false;

	if (playerHuman->moveLeft) {
		playerHuman->moveDirection.x = -1.0f;
		moveX = true;
	}

	if (playerHuman->moveRight) {
		playerHuman->moveDirection.x = 1.0f;
		moveX = true;
	}

	if (playerHuman->moveUp) {
		playerHuman->moveDirection.y = -1.0f;
		moveY = true;
	}

	if (playerHuman->moveDown) {
		playerHuman->moveDirection.y = 1.0f;
		moveY = true;
	}

	if (moveX && moveY) 
		playerHuman->moveDirection = PointProd(1.0f / Sqrt(2.0f), playerHuman->moveDirection);

	Point moveVector = PointProd(humanMoveSpeed * seconds, playerHuman->moveDirection);

	MoveHuman(&playerHuman->human, moveVector);
}