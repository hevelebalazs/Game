#pragma once

#include "Point.h"
#include "PlayerHuman.h"

struct GameState {
	Renderer renderer;
	Map map;
	Intersection* selectedIntersection;

	Building* selectedBuilding;
	Building* highlightedBuilding;

	PathHelper pathHelper;
	Path buildingPath;

	AutoVehicle autoVehicles[100];
	int autoVehicleCount = 100;

	PlayerHuman playerHuman;
};

void GameInit(GameState* gameState, int windowWidth, int windowHeight);
void GameUpdate(GameState* gameState, float seconds, Point mousePosition);
void GameDraw(GameState* gameState);