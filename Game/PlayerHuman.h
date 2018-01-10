#pragma once

#include "Game.h"
#include "Human.h"
#include "Point.h"

struct PlayerHuman {
	Human human;

	bool moveUp;
	bool moveDown;
	bool moveLeft;
	bool moveRight;

	Point moveDirection;

	bool isAiming;
	// TODO: should this be here?
	//       or passed to UpdatePlayerHuman?
	Point aimPosition;

	// TODO: rename this to shootCooldown?
	float aimRedSeconds;
};

struct GameState;

void ShootBullet(PlayerHuman* playerHuman, GameState* gameState);

void UpdatePlayerHuman(PlayerHuman* playerHuman, float seconds);
void DrawPlayerHuman(Renderer renderer, PlayerHuman* playerHuman);