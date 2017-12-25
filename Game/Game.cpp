#include "AutoVehicle.h"
#include "Building.h"
#include "Game.h"
#include "GridMap.h"
#include "Math.h"
#include "Memory.h"
#include "Path.h"
#include "PlayerVehicle.h"
#include "Renderer.h"
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

static inline void ResizeCamera(Camera* camera, int width, int height) {
	camera->screenSize = Point{(float)width, (float)height};
	camera->center = PointProd(0.5f, camera->screenSize);
}

static inline void ResizeBitmap(Bitmap* bitmap, int width, int height) {
	if (bitmap->memory)
		delete bitmap->memory;

	bitmap->width = width;
	bitmap->height = height;

	bitmap->info.bmiHeader.biSize = sizeof(bitmap->info.bmiHeader);
	bitmap->info.bmiHeader.biWidth = bitmap->width;
	bitmap->info.bmiHeader.biHeight = -bitmap->height;
	bitmap->info.bmiHeader.biPlanes = 1;
	bitmap->info.bmiHeader.biBitCount = 32;
	bitmap->info.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmap->width * bitmap->height) * bytesPerPixel;

	bitmap->memory = (void*)(new char[bitmapMemorySize]);
}

void WinResize(GameState* gameState, int width, int height) {
	if (!gameState)
		return;

	ResizeCamera(&gameState->camera, width, height);

	ResizeBitmap(&gameState->renderer.bitmap, width, height);
	ResizeBitmap(&gameState->maskRenderer.bitmap, width, height);

	gameState->renderer.camera = &gameState->camera;
	gameState->maskRenderer.camera = &gameState->camera;
}

void GameInit(GameStorage* gameStorage, int windowWidth, int windowHeight) {
	gameStorage->arena = CreateMemArena(10u * 1024u * 1024u);
	gameStorage->tmpArena = CreateMemArena(10u * 1024u * 1024u);
	MemArena* arena = &gameStorage->arena;
	MemArena* tmpArena = &gameStorage->tmpArena;

	gameStorage->gameState = ArenaPushType(arena, GameState);
	GameState* gameState = gameStorage->gameState;
	*gameState = GameState{};

	gameState->map = CreateGridMap((float)windowWidth, (float)windowHeight, 100, arena, tmpArena);

	int maxPathNodeCount = 10000;
	gameState->pathPool.maxNodeCount = maxPathNodeCount;
	gameState->pathPool.nodes = ArenaPushArray(arena, PathNode, maxPathNodeCount);

	WinResize(gameState, windowWidth, windowHeight);

	Intersection* intersection = RandomIntersection(gameState->map);
	gameState->playerHuman.human.position = intersection->position;
	gameState->playerHuman.human.map = &gameState->map;

	PlayerVehicle* playerVehicle = &gameState->playerVehicle;
	Vehicle* vehicle = &playerVehicle->vehicle;;

	playerVehicle->defaultColor   = Color{0.0f, 0.0f, 1.0f};
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
	}

	Camera* camera = &gameState->camera;
	camera->zoomSpeed = 10.0f;
	camera->pixelCoordRatio = 10.0f;
	camera->center = Point{(float)windowWidth * 0.5f, (float)windowHeight * 0.5f};

	gameState->renderer.camera = camera;
	gameState->maskRenderer.camera = camera;

	gameState->missionIntersection = RandomIntersection(gameState->map);
	gameState->onMission = false;
}

// TODO: get rid of the mousePosition parameter?
void GameUpdate(GameStorage* gameStorage, float seconds, Point mousePosition) {
	GameState* gameState = gameStorage->gameState;
	MemArena* arena = &gameStorage->arena;
	MemArena* tmpArena = &gameStorage->tmpArena;

	for (int i = 0; i < gameState->map.intersectionCount; ++i) {
		Intersection* intersection = &gameState->map.intersections[i];
		UpdateTrafficLights(intersection, seconds);
	}

	for (int i = 0; i < gameState->autoVehicleCount; ++i) {
		AutoVehicle* autoVehicle = &gameState->autoVehicles[i];
		UpdateAutoVehicle(autoVehicle, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
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
	if (gameState->showFullMap) {
		camera->center = Point{
			gameState->map.width * 0.5f,
			gameState->map.height * 0.5f
		};

		camera->zoomTargetRatio = 1.0f;
	}
	else if (gameState->isPlayerVehicle) {
		camera->center = gameState->playerVehicle.vehicle.position;

		// TODO: create a speed variable in PlayerVehicle?
		float speed = VectorLength(gameState->playerVehicle.velocity);

		camera->zoomTargetRatio = 13.0f - (speed / 4.0f);
	}
	else {
		camera->center = gameState->playerHuman.human.position;

		if (gameState->playerHuman.human.inBuilding)
			camera->zoomTargetRatio = 20.0f;
		else
			camera->zoomTargetRatio = 10.0f;
	}

	UpdateCamera(camera, seconds);

	// TODO: create StartMission function?
	Color missionHighlightColor = {1.0f, 1.0f, 0.0f};
	Point playerPosition = {};
	if (gameState->isPlayerVehicle)
		playerPosition = gameState->playerVehicle.vehicle.position;
	else
		playerPosition = gameState->playerHuman.human.position;

	MapElem playerElem = MapElemAtPoint(gameState->map, playerPosition);
	if (playerElem.type == MapElemIntersection && playerElem.intersection == gameState->missionIntersection) {
		gameState->missionIntersection = RandomIntersection(gameState->map);
		gameState->onMission = !gameState->onMission;

		if (gameState->onMission) {
			gameState->playerVehicle.defaultColor = missionHighlightColor;
			gameState->playerVehicle.vehicle.color = missionHighlightColor;
			gameState->playerHuman.human.color = missionHighlightColor;
		}
		else {
			gameState->playerVehicle.defaultColor = Color{0.0f, 0.0f, 1.0f};
			gameState->playerVehicle.vehicle.color = Color{0.0f, 0.0f, 1.0f};
			gameState->playerHuman.human.color = Color{0.0f, 0.0f, 0.0f};
		}
	}
}

void GameDraw(GameStorage* gameStorage) {
	GameState* gameState = gameStorage->gameState;
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

			DrawVisibleAreaInBuilding(maskRenderer, *inBuilding, gameState->playerHuman.human.position, &gameStorage->tmpArena);

			ApplyBitmapMask(renderer.bitmap, maskRenderer.bitmap);
		}
	}
	else {
		DrawMap(renderer, gameState->map);

		Color missionHighlightColor = {1.0f, 1.0f, 0.0f};
		if (gameState->missionIntersection)
			HighlightIntersection(gameState->renderer, *gameState->missionIntersection, missionHighlightColor);

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