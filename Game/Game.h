#pragma once

#include "Building.h"
#include "MapElem.h"
#include "Point.h"
#include "PlayerHuman.h"
#include "PlayerVehicle.h"

struct GameState {
	Renderer renderer;
	Map map;
	Intersection* selectedIntersection;

	PathHelper pathHelper;
	WallHelper wallHelper;

	AutoVehicle autoVehicles[100];
	int autoVehicleCount = 100;

	PlayerHuman playerHuman;
	PlayerVehicle playerVehicle;

	bool isPlayerVehicle;
};

void TogglePlayerVehicle(GameState* gameState);

void GameInit(GameState* gameState, int windowWidth, int windowHeight);
void GameUpdate(GameState* gameState, float seconds, Point mousePosition);
void GameDraw(GameState* gameState);