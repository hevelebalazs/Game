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

void ShootBullet(PlayerHuman* playerHuman, V2 targetPoint);

void UpdatePlayerHuman(PlayerHuman* playerHuman, F32 seconds, GameState* gameState);
void DrawPlayerHuman(Canvas canvas, PlayerHuman* playerHuman);
