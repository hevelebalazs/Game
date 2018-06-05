#pragma once

#include "AutoHuman.h"
#include "Car.h"
#include "MapElem.h"
#include "Memory.h"
#include "Path.h"
#include "PlayerHuman.h"
#include "Point.h"
#include "Renderer.h"
#include "Texture.h"

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
	float time;

	Camera camera;

	Renderer renderer;
	Renderer maskRenderer;
	
	Map map;
	Junction* selectedJunction;

	AutoCar autoCars[100];
	int autoCarCount;

	AutoHuman autoHumans[300];
	int autoHumanCount;

	PlayerHuman playerHuman;
	PlayerCar playerCar;

	PathPool pathPool;

	bool isPlayerCar;

	Junction* missionJunction;
	bool showFullMap;
	bool onMission;
	PathNode* missionPath;
	// TODO: create a MapPosition structure that contains a positionIndex and a MapElem (and a position)?
	MapElem missionElem;
	int missionLaneIndex;
	Point missionStartPoint;

	Bitmap carBitmaps[CarBitmapN];

	GameAssets assets;
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

void WinResize(GameState* gameState, int width, int height);
void GameInit(GameStorage* gameStorage, int windowWidth, int windowHeight);
void GameUpdate(GameStorage* gameStorage, float seconds, Point mousePosition);
void GameDraw(GameStorage* gameStorage);