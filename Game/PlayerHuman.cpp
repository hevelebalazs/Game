#include "Game.hpp"
#include "Geometry.hpp"
#include "Human.hpp"
#include "Math.hpp"
#include "PlayerHuman.hpp"
#include "Point.hpp"
#include "Renderer.hpp"

float aimDistance = 50.0f;
float bulletTotalSeconds = 0.2f;
float shootCooldown = 0.2f;
float bulletSpeed = (aimDistance / bulletTotalSeconds);

// TODO: this doesn't belong here, move it somewhere else
// TODO: add an aimPosition parameter?
void ShootBullet(PlayerHuman* playerHuman, Point targetPoint) {
	Human* human = &playerHuman->human;
	Point aimStartPoint = playerHuman->human.position;
	Point aimEndPoint = targetPoint;
	Line aimLine = Line{aimStartPoint, aimEndPoint};

	playerHuman->shootCooldown = shootCooldown;

	Bullet* bullet = &playerHuman->bullet;

	bullet->secondsRemaining = bulletTotalSeconds;
	bullet->position.position = human->position;
	bullet->position.direction = PointDirection(human->position, targetPoint);
}

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
		} else {
			crossInfo = GetClosestExtBuildingCrossInfo(human->map, humanRadius, human->position, pointToGo);

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

// TODO: the gameState is needed for updating the bullet
void UpdatePlayerHuman(PlayerHuman* playerHuman, float seconds, GameState* gameState) {
	Human* human = &playerHuman->human;

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

	Point moveVector = PointProd(human->moveSpeed * seconds, playerHuman->moveDirection);

	MoveHuman(human, moveVector);

	// TODO: merge update and draw everywhere
	if (playerHuman->shootCooldown >= 0.0f) {
		playerHuman->shootCooldown -= seconds;
		if (playerHuman->shootCooldown <= 0.0f)
			playerHuman->shootCooldown = 0.0f;
	}

	// TODO: create an UpdateBullet function
	//       and move it to another file?
	Bullet* bullet = &playerHuman->bullet;
	if (bullet->secondsRemaining > 0.0f) {
		float secondsToGo = 0.0f;

		if (bullet->secondsRemaining > seconds) {
			secondsToGo = seconds;
			bullet->secondsRemaining -= seconds;
		}
		else {
			secondsToGo = bullet->secondsRemaining;
			bullet->secondsRemaining = 0.0f;
		}

		Point oldPosition = bullet->position.position;
		Point newPosition = PointSum(
			bullet->position.position,
			PointProd(secondsToGo * bulletSpeed, bullet->position.direction)
		);

		bullet->position.position = newPosition;

		// TODO: is it necessary to build this struct?
		Line bulletLine = Line{oldPosition, newPosition};

		for (int i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			Human* human = &autoHuman->human;

			if (!autoHuman->dead && IsHumanCrossedByLine(human, bulletLine)) {
				DamageAutoHuman(autoHuman, &gameState->pathPool);
				bullet->secondsRemaining = 0.0f;
				break;
			}
		}
	}
}

void DrawPlayerHuman(Renderer renderer, PlayerHuman* playerHuman) {
	Human* human = &playerHuman->human;

	Bullet* bullet = &playerHuman->bullet;
	if (bullet->secondsRemaining > 0.0f) {
		float bulletTailLength = 2.0f;

		Point point1 = bullet->position.position;
		Point point2 = PointDiff(
			point1,
			PointProd(bulletTailLength, bullet->position.direction)
		);

		// TODO: make this a global value
		Color bulletColor = Color{1.0f, 1.0f, 0.0f};
		Bresenham(renderer, point1, point2, bulletColor);
	}

	DrawHuman(renderer, playerHuman->human);
}