#pragma once

#include "Game.hpp"
#include "Human.hpp"
#include "Math.hpp"
#include "Type.hpp"

struct Bullet 
{
	V4 position;

	// TODO: 0 means not active
	F32 secondsRemaining;
};

struct PlayerHuman 
{
	Human human;

	B32 moveUp;
	B32 moveDown;
	B32 moveLeft;
	B32 moveRight;

	V2 moveDirection;

	F32 shootCooldown;
	Bullet bullet;
};

struct GameState;

#define AimDistance        50.0f
#define BulletTotalSeconds 0.2f
#define ShootCooldown      0.2f
#define BulletSpeed        (AimDistance / BulletTotalSeconds)

// TODO: this doesn't belong here, move it somewhere else
// TODO: add an aimPosition parameter?
static void ShootBullet(PlayerHuman* playerHuman, V2 targetPoint)
{
	Human* human = &playerHuman->human;
	V2 aimStartPoint = playerHuman->human.position;
	V2 aimEndPoint = targetPoint;
	Line aimLine = Line{aimStartPoint, aimEndPoint};

	playerHuman->shootCooldown = ShootCooldown;

	Bullet* bullet = &playerHuman->bullet;

	bullet->secondsRemaining = BulletTotalSeconds;
	bullet->position.position = human->position;
	bullet->position.direction = PointDirection(human->position, targetPoint);
}

static void MoveHuman(Human* human, V2 moveVector)
{
	Map* map = human->map;

	F32 distanceToGo = VectorLength(moveVector);

	// TODO: should there be a limit on the iteration number?
	while (distanceToGo > 0.0f) 
	{
		B32 go = true;
		B32 isTouchingLine = false;
		BuildingCrossInfo crossInfo = {};
		crossInfo.type = CrossNone;

		V2 pointToGo = (human->position + moveVector);

		if (human->inBuilding) 
		{
			B32 isInBuilding = IsPointInExtBuilding(human->position, *human->inBuilding, HumanRadius);

			if (isInBuilding)
				crossInfo = ExtBuildingInsideClosestCrossInfo(human->inBuilding, HumanRadius, human->position, pointToGo);
			else
				human->inBuilding = 0;
		} 
		else 
		{
			crossInfo = GetClosestExtBuildingCrossInfo(human->map, HumanRadius, human->position, pointToGo);

			if (crossInfo.type == CrossEntrance)
				human->inBuilding = crossInfo.building;
		}

		if (crossInfo.type == CrossWall) 
		{
			Building* crossedBuilding = crossInfo.building;

			V2 crossPoint = crossInfo.crossPoint;

			// TODO: create a proper collision system
			F32 distanceTaken = Distance(human->position, crossPoint);

			V2 moveNormal = (1.0f / VectorLength(moveVector)) * moveVector;
			V2 wallDirection = (crossInfo.corner1 - crossInfo.corner2);
			moveVector = ParallelVector(moveVector, wallDirection);

			distanceToGo = VectorLength(moveVector);

			go = false;
		}

		if (go) 
		{
			human->position = pointToGo;
			distanceToGo = 0.0f;
		}
	}
}

// TODO: Put back bullet damage!
static void UpdatePlayerHuman(PlayerHuman* playerHuman, F32 seconds/*, GameState* gameState*/)
{
	Human* human = &playerHuman->human;

	playerHuman->moveDirection = MakePoint(0.0f, 0.0f);

	B32 moveX = false;
	B32 moveY = false;

	if (playerHuman->moveLeft) 
	{
		playerHuman->moveDirection.x = -1.0f;
		moveX = true;
	}

	if (playerHuman->moveRight) 
	{
		playerHuman->moveDirection.x = 1.0f;
		moveX = true;
	}

	if (playerHuman->moveUp) 
	{
		playerHuman->moveDirection.y = -1.0f;
		moveY = true;
	}

	if (playerHuman->moveDown) 
	{
		playerHuman->moveDirection.y = 1.0f;
		moveY = true;
	}

	if (moveX && moveY) 
		playerHuman->moveDirection = (1.0f / Sqrt(2.0f)) * playerHuman->moveDirection;

	V2 moveVector = (human->moveSpeed * seconds) * playerHuman->moveDirection;

	MoveHuman(human, moveVector);

	// TODO: merge update and draw everywhere
	if (playerHuman->shootCooldown >= 0.0f) 
	{
		playerHuman->shootCooldown -= seconds;
		if (playerHuman->shootCooldown <= 0.0f)
			playerHuman->shootCooldown = 0.0f;
	}

	// TODO: create an UpdateBullet function
	//       and move it to another file?
	Bullet* bullet = &playerHuman->bullet;
	if (bullet->secondsRemaining > 0.0f) 
	{
		F32 secondsToGo = 0.0f;

		if (bullet->secondsRemaining > seconds) 
		{
			secondsToGo = seconds;
			bullet->secondsRemaining -= seconds;
		} 
		else 
		{
			secondsToGo = bullet->secondsRemaining;
			bullet->secondsRemaining = 0.0f;
		}

		V2 oldPosition = bullet->position.position;
		V2 newPosition = oldPosition + ((secondsToGo * BulletSpeed) * bullet->position.direction);

		bullet->position.position = newPosition;

		// TODO: is it necessary to build this struct?
		Line bulletLine = Line{oldPosition, newPosition};

		/*
		for (I32 i = 0; i < gameState->autoHumanCount; ++i) 
		{
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			Human* human = &autoHuman->human;

			if (!autoHuman->dead && IsHumanCrossedByLine(human, bulletLine)) 
			{
				DamageAutoHuman(autoHuman, &gameState->pathPool);
				bullet->secondsRemaining = 0.0f;
				break;
			}
		}
		*/
	}
}

static void DrawPlayerHuman(Canvas canvas, PlayerHuman* playerHuman)
{
	Human* human = &playerHuman->human;

	Bullet* bullet = &playerHuman->bullet;
	if (bullet->secondsRemaining > 0.0f) 
	{
		F32 bulletTailLength = 2.0f;

		V2 point1 = bullet->position.position;
		V2 point2 = point1 - (bulletTailLength * bullet->position.direction);

		// TODO: make this a global value
		V4 bulletColor = MakeColor(1.0f, 1.0f, 0.0f);
		Bresenham(canvas, point1, point2, bulletColor);
	}

	DrawHuman(canvas, playerHuman->human);
}
