#include "AutoVehicle.h"
#include "Building.h"
#include "Game.h"
#include "GridMapCreator.h"
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

	Intersection* intersection = RandomIntersection(gameState->map);
	gameState->playerHuman.human.position = intersection->coordinate;
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

	gameState->renderer.camera.zoomSpeed = 10.0f;
	gameState->renderer.camera.pixelCoordRatio = 10.0f;
	gameState->renderer.camera.center = Point{(float)windowWidth * 0.5f, (float)windowHeight * 0.5f};
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

			if (angleCos < minAngleCos) TurnPlayerVehicleRed(&gameState->playerVehicle, 0.2f);
		}
		else if (onElemAfter.type == MapElemIntersection) {
			Intersection* intersection = onElemAfter.intersection;

			if (onElemBefore.type == MapElemRoad) {
				Road* road = onElemBefore.road;
				TrafficLight* trafficLight = TrafficLightOfRoad(intersection, road);

				if (trafficLight && trafficLight->color == TrafficLight_Red) {
					TurnPlayerVehicleRed(&gameState->playerVehicle, 0.5f);
				}
			}
		}
	}
	else {
		UpdatePlayerHuman(&gameState->playerHuman, seconds);
	}

	Camera* camera = &gameState->renderer.camera;
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
	if (inBuilding) {
		DrawBuildingInside(renderer, *inBuilding);
	}
	else {
		DrawMap(renderer, gameState->map);

		for (int i = 0; i < gameState->autoVehicleCount; ++i) {
			AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
			DrawVehicle(renderer, autoVehicle->vehicle);
		}
	}

	if (gameState->isPlayerVehicle) DrawVehicle(gameState->renderer, gameState->playerVehicle.vehicle);
	else DrawHuman(gameState->renderer, gameState->playerHuman.human);
}