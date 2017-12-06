#include "AutoVehicle.h"
#include "Building.h"
#include "Game.h"
#include "GridMapCreator.h"
#include "Path.h"
#include "Vehicle.h"

void GameInit(GameState* gameState, int windowWidth, int windowHeight) {
	gameState->map = CreateGridMap((float)windowWidth, (float)windowHeight, 100);
	gameState->pathHelper = PathHelperForMap(&gameState->map);

	Intersection* intersection = RandomIntersection(gameState->map);
	gameState->playerHuman.human.position = intersection->coordinate;
	gameState->playerHuman.human.map = &gameState->map;

	for (int i = 0; i < gameState->autoVehicleCount; ++i) {
		Building* randomBuilding = RandomBuilding(gameState->map);

		AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
		Vehicle* vehicle = &autoVehicle->vehicle;

		vehicle->angle = 0.0f;
		vehicle->color = Color{0.0f, 0.0f, 1.0f};
		vehicle->position = randomBuilding->connectPointClose;
		vehicle->length = 7.5f;
		vehicle->width = 5.0f;
		vehicle->map = &gameState->map;
		vehicle->maxSpeed = 30.0f;
		autoVehicle->inBuilding = randomBuilding;
		autoVehicle->moveHelper = &gameState->pathHelper;
	}

	gameState->renderer.camera.zoomSpeed = 10.0f;
	gameState->renderer.camera.pixelCoordRatio = 10.0f;
}

// TODO: get rid of the mousePosition parameter?
void GameUpdate(GameState* gameState, float seconds, Point mousePosition) {
	for (int i = 0; i < gameState->map.intersectionCount; ++i) {
		Intersection* intersection = &gameState->map.intersections[i];

		UpdateTrafficLights(intersection, seconds);
	}

	for (int i = 0; i < gameState->autoVehicleCount; ++i) {
		AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
		UpdateAutoVehicle(autoVehicle, seconds);
	}

	UpdatePlayerHuman(&gameState->playerHuman, seconds);

	gameState->highlightedBuilding = BuildingAtPoint(gameState->map, mousePosition);

	if (gameState->selectedBuilding && gameState->highlightedBuilding && gameState->selectedBuilding != gameState->highlightedBuilding) {
		ClearPath(&gameState->buildingPath);

		// TODO: create a function for these?
		MapElem selectedBuildingElem = {};
		selectedBuildingElem.type = MapElemBuilding;
		selectedBuildingElem.building = gameState->selectedBuilding;

		MapElem highlightedBuildingElem = {};
		highlightedBuildingElem.type = MapElemBuilding;
		highlightedBuildingElem.building = gameState->highlightedBuilding;

		gameState->buildingPath = ConnectElems(&gameState->map, selectedBuildingElem, highlightedBuildingElem, &gameState->pathHelper);
	}

	Camera* camera = &gameState->renderer.camera;
	camera->center = gameState->playerHuman.human.position;

	if (gameState->playerHuman.human.inBuilding) {
		camera->zoomTargetRatio = 20.0f;
	}
	else {
		camera->zoomTargetRatio = 10.0f;
	}

	UpdateCamera(camera, seconds);
}

void GameDraw(GameState* gameState) {
	Renderer renderer = gameState->renderer;

	Color clearColor = Color{0.0f, 0.0f, 0.0f};
	ClearScreen(renderer, clearColor);

	Building* inBuilding = gameState->playerHuman.human.inBuilding;
	if (inBuilding) {
		DrawBuildingInside(renderer, *inBuilding);
	}
	else {
		DrawMap(renderer, gameState->map);

		if (gameState->selectedBuilding) {
			Color highlightColor = {0.0f, 1.0f, 1.0f};
			HighLightBuilding(gameState->renderer, *gameState->selectedBuilding, highlightColor);
		}

		if (gameState->highlightedBuilding) {
			Color highlightColor = {0.0f, 1.0f, 1.0f};
			HighLightBuilding(gameState->renderer, *gameState->highlightedBuilding, highlightColor);
		}

		if (gameState->buildingPath.nodeCount > 0 && gameState->selectedBuilding && gameState->highlightedBuilding
			&& gameState->selectedBuilding != gameState->highlightedBuilding) {
			Color color = {0.0f, 0.8f, 0.8f};
			DrawBezierPath(&gameState->buildingPath, gameState->renderer, color, 3.0f);
		}

		for (int i = 0; i < gameState->autoVehicleCount; ++i) {
			AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
			DrawVehicle(renderer, autoVehicle->vehicle);
		}
	}

	DrawHuman(gameState->renderer, gameState->playerHuman.human);
}