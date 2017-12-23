#include "AutoVehicle.h"
#include "Building.h"
#include "Game.h"
#include "GridMap.h"
#include "Math.h"
#include "Path.h"
#include "PlayerVehicle.h"
#include "Vehicle.h"

void TogglePlayerVehicle(GameState* gameState) {
	Point playerPosition = {};

	if (gameState->isPlayerVehicle) playerPosition = gameState->playerVehicle.vehicle.position;
	else playerPosition = gameState->playerHuman.human.position;

	MapElem playerOnElem = MapElemAtPoint(gameState->map, playerPosition);

	if (playerOnElem.type == MapElemRoad || playerOnElem.type == MapElemIntersection) {
		if (gameState->isPlayerVehicle) {
			gameState->isPlayerVehicle = false;
			gameState->playerHuman.human.position = playerPosition;
		}
		else {
			gameState->isPlayerVehicle = true;
			gameState->playerVehicle.vehicle.position = playerPosition;
		}
	}
}

void GameInit(GameState* gameState, int windowWidth, int windowHeight) {
	gameState->map = CreateGridMap((float)windowWidth, (float)windowHeight, 100);
	gameState->pathHelper = PathHelperForMap(&gameState->map);

	// TODO: fix this when window is resized (using a memory arena will fix this nicely)
	gameState->fillHelper = FillHelperForBitmap(gameState->renderer.bitmap);

	Intersection* intersection = RandomIntersection(gameState->map);
	gameState->playerHuman.human.position = intersection->position;
	gameState->playerHuman.human.map = &gameState->map;

	PlayerVehicle* playerVehicle = &gameState->playerVehicle;
	Vehicle* vehicle = &playerVehicle->vehicle;;

	playerVehicle->maxEngineForce = 1000.0f;
	playerVehicle->breakForce     = 1000.0f;
	playerVehicle->mass           = 200.0f;
	
	vehicle->angle  = 0.0f;
	vehicle->length = 8.0f;
	vehicle->width  = 5.0f;
	vehicle->color  = Color{0.0f, 0.0f, 1.0f};

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

	Camera* camera = &gameState->camera;
	camera->zoomSpeed = 5.0f;
	camera->pixelCoordRatio = 10.0f;
	camera->center = Point{(float)windowWidth * 0.5f, (float)windowHeight * 0.5f};
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

	if (gameState->isPlayerVehicle) {
		MapElem onElemBefore = MapElemAtPoint(gameState->map, gameState->playerVehicle.vehicle.position);

		UpdatePlayerVehicle(&gameState->playerVehicle, seconds);

		MapElem onElemAfter = MapElemAtPoint(gameState->map, gameState->playerVehicle.vehicle.position);

		if (onElemAfter.type == MapElemNone) {
			TurnPlayerVehicleRed(&gameState->playerVehicle, 0.2f);
		}
		else if (onElemAfter.type == MapElemRoad) {
			Road* road = onElemAfter.road;

			int laneIndex = LaneIndex(*road, gameState->playerVehicle.vehicle.position);
			Point laneDirection = LaneDirection(*road, laneIndex);

			Point vehicleDirection = RotationVector(gameState->playerVehicle.vehicle.angle);
			float angleCos = DotProduct(laneDirection, vehicleDirection);

			float minAngleCos = 0.0f;

			if (angleCos < minAngleCos) 
				TurnPlayerVehicleRed(&gameState->playerVehicle, 0.2f);
		}
		else if (onElemAfter.type == MapElemIntersection) {
			Intersection* intersection = onElemAfter.intersection;

			if (onElemBefore.type == MapElemRoad) {
				Road* road = onElemBefore.road;
				TrafficLight* trafficLight = TrafficLightOfRoad(intersection, road);

				if (trafficLight && trafficLight->color == TrafficLightRed)
					TurnPlayerVehicleRed(&gameState->playerVehicle, 0.5f);
			}
		}
	}
	else {
		UpdatePlayerHuman(&gameState->playerHuman, seconds);
	}
	
	Camera* camera = gameState->renderer.camera;
	camera->center = gameState->playerHuman.human.position;

	if (gameState->isPlayerVehicle) {
		camera->center = gameState->playerVehicle.vehicle.position;

		// TODO: create a speed variable in PlayerVehicle?
		float speed = VectorLength(gameState->playerVehicle.velocity);

		camera->zoomTargetRatio = 13.0f - (speed / 4.0f);
	}
	else {
		camera->center = gameState->playerHuman.human.position;

		if (gameState->playerHuman.human.inBuilding) {
		camera->zoomTargetRatio = 20.0f;
		}
		else {
		camera->zoomTargetRatio = 10.0f;
		}
	}

	UpdateCamera(camera, seconds);
}

void GameDraw(GameState* gameState) {
	Renderer renderer = gameState->renderer;

	Color clearColor = Color{0.0f, 0.0f, 0.0f};
	ClearScreen(renderer, clearColor);

	Building* inBuilding = gameState->playerHuman.human.inBuilding;
	if (inBuilding && IsPointInBuilding(gameState->playerHuman.human.position, *inBuilding)) {
		DrawBuildingInside(renderer, *inBuilding);

		if (inBuilding && IsPointInBuilding(gameState->playerHuman.human.position, *inBuilding)) {
			Color black = Color{0.0f, 0.0f, 0.0f};

			Renderer maskRenderer = gameState->maskRenderer;
			ClearScreen(maskRenderer, black);

			DrawVisibleAreaInBuilding(maskRenderer, *inBuilding, gameState->playerHuman.human.position, gameState->fillHelper);

			ApplyBitmapMask(renderer.bitmap, maskRenderer.bitmap);
		}
	}
	else {
		DrawMap(renderer, gameState->map);

		for (int i = 0; i < gameState->autoVehicleCount; ++i) {
			AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
			DrawVehicle(renderer, autoVehicle->vehicle);
		}
	}

	if (gameState->isPlayerVehicle)
		DrawVehicle(renderer, gameState->playerVehicle.vehicle);
	else
		DrawHuman(renderer, gameState->playerHuman.human);
}