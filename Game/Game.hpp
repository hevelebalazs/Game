#pragma once

#include "AutoHuman.hpp"
#include "Car.hpp"
#include "Draw.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "PlayerHuman.hpp"
#include "Texture.hpp"
#include "Type.hpp"

#define CarBitmapN 20

struct GameState {
	F32 time;

	Camera camera;

	Canvas canvas;
	Canvas maskCanvas;
	
	Map map;
	Junction* selectedJunction;

	AutoCar autoCars[100];
	I32 autoCarCount;

	AutoHuman autoHumans[300];
	I32 autoHumanCount;

	PlayerHuman playerHuman;
	PlayerCar playerCar;

	PathPool pathPool;

	B32 isPlayerCar;

	Bitmap carBitmaps[CarBitmapN];

	MapTextures mapTextures;
};

// TODO: remove gameState reference
// TODO: remove arenas, add only pointers
// TODO: add a pointer to a PathPool?
struct GameStorage {
	GameState* gameState;
	MemArena arena;
	MemArena tmpArena;
};

void TogglePlayerCar(GameState* gameState);

void WinResize(GameState* gameState, I32 width, I32 height);
void GameInit(GameStorage* gameStorage, I32 windowWidth, I32 windowHeight);
void GameUpdate(GameStorage* gameStorage, F32 seconds, V2 mousePosition);
void GameDraw(GameStorage* gameStorage);