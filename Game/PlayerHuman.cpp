#include "Geometry.h"
#include "Human.h"
#include "PlayerHuman.h"
#include "Point.h"

static void MoveHuman(Human* human, Point moveVector) {
	Map* map = human->map;

	float distanceToGo = VectorLength(moveVector);

	// TODO: should there be a limit on the iteration number?
	while (distanceToGo > 0.0f) {
		bool go = true;

		Point pointToGo = PointSum(human->position, moveVector);

		BuildingCrossInfo crossInfo = ClosestExtBuildingCrossInfo(*human->map, human->radius, human->position, pointToGo);

		Building* crossedBuilding = crossInfo.building;

		if (crossedBuilding) {
			if (crossInfo.entrance == EntranceOut) {
				if (human->inBuilding == crossedBuilding) human->inBuilding = 0;
				else human->inBuilding = crossedBuilding;
			}
			else if (crossInfo.entrance == EntranceIn) {
				go = true;
			}
			else if (crossInfo.entrance == EntranceSide) {
				distanceToGo = 0.0f;
				go = false;
			}
			else {
				Point intersectionPoint = ClosestBuildingCrossPoint(*crossedBuilding, human->position, pointToGo);

				// TODO: create a proper collision system
				float distanceTaken = Distance(human->position, intersectionPoint);

				Point moveNormal = PointProd(1.0f / VectorLength(moveVector), moveVector);

				Point wallDirection = PointDiff(crossInfo.corner1, crossInfo.corner2);
				moveVector = ParallelVector(moveVector, wallDirection);

				distanceToGo = VectorLength(moveVector);

				go = false;
			}
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

	if (moveX && moveY) playerHuman->moveDirection = PointProd(1.0f / sqrtf(2.0f), playerHuman->moveDirection);

	Point moveVector = PointProd(Human::moveSpeed * seconds, playerHuman->moveDirection);

	MoveHuman(&playerHuman->human, moveVector);
}