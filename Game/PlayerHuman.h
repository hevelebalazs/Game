#pragma once

#include "Game.h"
#include "Human.h"
#include "Point.h"

struct Bullet {
	DirectedPoint position;

	// TODO: 0 means not active
	float secondsRemaining;
};

struct PlayerHuman {
	Human human;

	bool moveUp;
	bool moveDown;
	bool moveLeft;
	bool moveRight;

	Point moveDirection;

	float shootCooldown;
	Bullet bullet;
};

struct GameState;

void ShootBullet(PlayerHuman* playerHuman, Point targetPoint);

void UpdatePlayerHuman(PlayerHuman* playerHuman, float seconds, GameState* gameState);
void DrawPlayerHuman(Renderer renderer, PlayerHuman* playerHuman);