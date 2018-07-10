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

// TODO: rename this and remove from Game
struct GameAssets {
	Texture roadTexture;
	Texture stripeTexture;
	Texture sidewalkTexture;
	Texture grassTexture;

	Texture roofTextureUp;
	Texture roofTextureDown;
	Texture roofTextureLeft;
	Texture roofTextureRight;
};

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

	Junction* missionJunction;
	B32 showFullMap;
	B32 onMission;
	PathNode* missionPath;
	// TODO: create a MapPosition structure that contains a positionIndex and a MapElem (and a position)?
	MapElem missionElem;
	I32 missionLaneIndex;
	V2 missionStartPoint;

	Bitmap carBitmaps[CarBitmapN];

	GameAssets assets;
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