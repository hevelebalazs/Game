#pragma once

#include "AutoVehicle.hpp"
#include "AutoHuman.hpp"
#include "Building.hpp"
#include "MapElem.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "PlayerHuman.hpp"
#include "PlayerVehicle.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"

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

struct GameState {
	float time;

	Camera camera;

	Renderer renderer;
	Renderer maskRenderer;
	
	Map map;
	Intersection* selectedIntersection;

	AutoVehicle autoVehicles[100];
	int autoVehicleCount = 100;

	AutoHuman autoHumans[300];
	int autoHumanCount = 300;

	PlayerHuman playerHuman;
	PlayerVehicle playerVehicle;

	PathPool pathPool;

	bool isPlayerVehicle;

	Intersection* missionIntersection;
	bool showFullMap;
	bool onMission;
	PathNode* missionPath;
	// TODO: create a MapPosition structure that contains a positionIndex and a MapElem (and a position)?
	MapElem missionElem;
	int missionLaneIndex;
	Point missionStartPoint;

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

void TogglePlayerVehicle(GameState* gameState);

void WinResize(GameState* gameState, int width, int height);
void GameInit(GameStorage* gameStorage, int windowWidth, int windowHeight);
void GameUpdate(GameStorage* gameStorage, float seconds, Point mousePosition);
void GameDraw(GameStorage* gameStorage);