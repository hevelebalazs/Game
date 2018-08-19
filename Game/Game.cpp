// TODO: use a unity build?

#include "Car.hpp"
#include "Draw.hpp"
#include "Game.hpp"
#include "GridMap.hpp"
#include "Light.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Type.hpp"

void TogglePlayerCar(GameState* gameState)
{
	V2 playerPosition = {};

	if (gameState->isPlayerCar) 
		playerPosition = gameState->playerCar.car.position;
	else 
		playerPosition = gameState->playerHuman.human.position;

	MapElem playerOnElem = GetRoadElemAtPoint(&gameState->map, playerPosition);
	if (playerOnElem.type == MapElemRoad || playerOnElem.type == MapElemJunction) {
		if (gameState->isPlayerCar) {
			gameState->isPlayerCar = false;
			gameState->playerHuman.human.position = playerPosition;
		} else {
			gameState->isPlayerCar = true;
			gameState->playerCar.car.position = playerPosition;
		}
	}
}

static void ResizeCamera(Camera* camera, I32 width, I32 height)
{
	camera->screenPixelSize.x = (F32)width; 
	camera->screenPixelSize.y = (F32)height;
	camera->center = (0.5f * camera->screenPixelSize);
}

static void ResizeBitmap(Bitmap* bitmap, I32 width, I32 height)
{
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

	I32 bitmapMemorySize = (bitmap->width * bitmap->height);
	bitmap->memory = new U32[bitmapMemorySize];
}

void WinResize(GameState* gameState, I32 width, I32 height)
{
	if (!gameState)
		return;

	ResizeCamera(&gameState->camera, width, height);

	ResizeBitmap(&gameState->canvas.bitmap, width, height);
	ResizeBitmap(&gameState->maskCanvas.bitmap, width, height);

	gameState->canvas.camera = &gameState->camera;
	gameState->maskCanvas.camera = &gameState->camera;
}

void GameInit(GameStorage* gameStorage, I32 windowWidth, I32 windowHeight)
{
	gameStorage->arena = CreateMemArena(10u * 1024u * 1024u);
	gameStorage->tmpArena = CreateMemArena(10u * 1024u * 1024u);
	MemArena* arena = &gameStorage->arena;
	MemArena* tmpArena = &gameStorage->tmpArena;

	gameStorage->gameState = ArenaPushType(arena, GameState);
	GameState* gameState = gameStorage->gameState;
	*gameState = GameState{};
	gameState->autoHumanCount = 300;
	gameState->autoCarCount = 100;

	I32 junctionRowN = 10;
	I32 junctionColN = 10;
	I32 junctionN = junctionRowN * junctionColN;
	I32 roadN = 100;
	Map* map = &gameState->map;
	map->junctions = ArenaPushArray(arena, Junction, junctionN);
	map->roads = ArenaPushArray(arena, Road, roadN);
	GenerateGridMap(map, junctionRowN, junctionColN, roadN, tmpArena);

	I32 maxPathNodeCount = 10000;
	gameState->pathPool.maxNodeCount = maxPathNodeCount;
	gameState->pathPool.nodes = ArenaPushArray(arena, PathNode, maxPathNodeCount);

	WinResize(gameState, windowWidth, windowHeight);

	PlayerHuman* playerHuman = &gameState->playerHuman;
	Junction* startJunction = GetRandomJunction(&gameState->map);
	playerHuman->human.position = startJunction->position;
	playerHuman->human.map = &gameState->map;
	playerHuman->human.moveSpeed = 5.0f;
	playerHuman->human.healthPoints = MaxHealthPoints;

	for (I32 i = 0; i < CarBitmapN; ++i) {
		Bitmap* carBitmap = &gameState->carBitmaps[i];
		AllocateCarBitmap(carBitmap);
		GenerateCarBitmap(carBitmap, tmpArena);
	}

	PlayerCar* playerCar = &gameState->playerCar;
	Car* car = &playerCar->car;
	
	car->angle    = 0.0f;
	car->length   = 5.0f;
	car->width    = 2.3f;
	car->maxSpeed = MaxPlayerCarSpeed;
	I32 carBitmapIndex = IntRandom(0, CarBitmapN - 1);
	car->bitmap = &gameState->carBitmaps[carBitmapIndex];

	playerCar->inertia = 0.0f;
	for (int i = 0; i < 4; ++i) {
		V2 corner = GetCarCorner(&playerCar->car, i);
		V2 radius = corner - playerCar->car.position;
		F32 radiusLength = VectorLength(radius);
		playerCar->inertia += CarCornerMass * (radiusLength * radiusLength);
	}

	for (I32 i = 0; i < gameState->autoCarCount; ++i) {
		// TODO: create an InitAutoCar function?
		Junction* randomJunction = GetRandomJunction(&gameState->map);

		AutoCar* autoCar = &gameState->autoCars[i];
		Car* car = &autoCar->car;

		car->angle = 0.0f;
		car->position = randomJunction->position;
		car->length = 5.0f;
		car->width = 2.3f;
		car->map = &gameState->map;
		car->maxSpeed = RandomBetween(MinCarSpeed, MaxCarSpeed);
		I32 carBitmapIndex = IntRandom(0, CarBitmapN - 1);
		car->bitmap = &gameState->carBitmaps[carBitmapIndex];
		car->moveSpeed = 0.0f;
		autoCar->onJunction = GetRandomJunction(map);
		autoCar->acceleration = 5.0f;
	}

	for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
		// TODO: create an InitAutoHuman function?
		AutoHuman* autoHuman = gameState->autoHumans + i;
		Human* human = &autoHuman->human;

		Junction* junction = GetRandomJunction(&gameState->map);
		I32 cornerIndex = GetRandomJunctionCornerIndex(junction);
		V2 position = GetJunctionCorner(junction, cornerIndex);

		human->position = position;
		human->inBuilding = 0;
		human->isPolice = 0;
		human->map = &gameState->map;
		human->moveSpeed = RandomBetween(2.0f, 10.0f);
		human->healthPoints = MaxHealthPoints;
		autoHuman->onJunction = junction;
	}

	Camera* camera = &gameState->camera;
	camera->unitInPixels = 50.0f;
	camera->center = 0.5f * MakePoint((F32)windowWidth, (F32)windowHeight);

	gameState->canvas.camera = camera;
	gameState->maskCanvas.camera = camera;

	map->generateTileWorkList.semaphore = CreateSemaphore(0, 0, MaxGenerateMapTileWorkListN, 0);
	for (I32 i = 0; i < GenerateMapTileWorkThreadN; ++i)
		CreateThread(0, 0, GenerateMapTileWorkProc, &map->generateTileWorkList, 0, 0);

	map->drawTileWorkList.semaphore = CreateSemaphore(0, 0, MaxDrawMapTileWorkListN, 0);
	map->drawTileWorkList.semaphoreDone = CreateSemaphore(0, 0, MaxDrawMapTileWorkListN, 0);
	for (I32 i = 0; i < DrawMapTileWorkThreadN; ++i)
		CreateThread(0, 0, DrawMapTileWorkProc, &map->drawTileWorkList, 0, 0);

	GenerateMapTextures(&gameState->mapTextures, &gameStorage->tmpArena);
}

CollisionInfo GetCarCollisionInfoWithAutoCars(Car* oldCar, Car* newCar, GameState* gameState)
{
	CollisionInfo hit = {};

	for (int i = 0; i < gameState->autoCarCount; ++i) {
		Car* testCar = &gameState->autoCars[i].car;
		Quad corners = GetCarCorners(testCar);
		CollisionInfo testHit = GetCarPolyCollisionInfo(oldCar, newCar, corners.points, 4);
		hit = hit + testHit;
	}

	return hit;
}

// TODO: get rid of the mousePosition parameter?
void GameUpdate(GameStorage* gameStorage, F32 seconds, V2 mousePosition)
{
	GameState* gameState = gameStorage->gameState;
	MemArena* arena = &gameStorage->arena;
	MemArena* tmpArena = &gameStorage->tmpArena;

	for (I32 i = 0; i < gameState->map.junctionN; ++i) {
		Junction* junction = &gameState->map.junctions[i];
		UpdateTrafficLights(junction, seconds);
	}

	for (I32 i = 0; i < gameState->autoCarCount; ++i) {
		AutoCar* autoCar = &gameState->autoCars[i];
		Car* car = &autoCar->car;

		bool shouldBrake = false;

		Quad stopArea = GetCarStopArea(car);

		V2 playerPosition = {};
		if (gameState->isPlayerCar)
			playerPosition = gameState->playerCar.car.position;
		else
			playerPosition = gameState->playerHuman.human.position;

		if (IsPointInQuad(stopArea, playerPosition))
				shouldBrake = true;

		for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			if (autoHuman->dead)
				continue;

			Human* human = &autoHuman->human;
			if (IsPointInQuad(stopArea, human->position))
				shouldBrake = true;
		}

		for (I32 i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* testAutoCar = &gameState->autoCars[i];
			if (autoCar == testAutoCar)
				continue;
			Car* testCar = &testAutoCar->car;
			if (IsPointInQuad(stopArea, testCar->position))
				shouldBrake = true;
		}

		if (shouldBrake)
			autoCar->acceleration = -25.0f;
		else
			autoCar->acceleration = +5.0f;
		UpdateAutoCar(autoCar, seconds, &gameStorage->tmpArena, &gameState->pathPool);
	}

	for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = &gameState->autoHumans[i];
		UpdateAutoHuman(autoHuman, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
	}

	if (gameState->isPlayerCar) {
		MapElem onElemBefore = GetRoadElemAtPoint(&gameState->map, gameState->playerCar.car.position);

		PlayerCar oldCar = gameState->playerCar;
		UpdatePlayerCarWithoutCollision(&gameState->playerCar, seconds);

		CollisionInfo hit = GetCarCollisionInfoWithAutoCars(&oldCar.car, &gameState->playerCar.car, gameState);
		UpdatePlayerCarCollision(&gameState->playerCar, &oldCar, seconds, hit);

		for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			V2 autoHumanPoint = autoHuman->human.position;

			if (IsCarOnPoint(&gameState->playerCar.car, autoHumanPoint)) {
				KillAutoHuman(autoHuman, &gameState->pathPool);
			}
		}
	} else {
		PlayerHuman* playerHuman = &gameState->playerHuman;
		UpdatePlayerHuman(playerHuman, seconds, gameState);
	}
	
	Camera* camera = gameState->canvas.camera;

	if (gameState->isPlayerCar) {
		camera->center = gameState->playerCar.car.position;

		// TODO: create a speed variable in PlayerCar?
		F32 speed = VectorLength(gameState->playerCar.velocity);

		camera->targetUnitInPixels = 50.0f - (1.0f * speed);
	} else {
		camera->center = gameState->playerHuman.human.position;

		if (gameState->playerHuman.human.inBuilding)
			camera->targetUnitInPixels = 50.0f;
		else
			camera->targetUnitInPixels = 50.0f;
	}

	UpdateCamera(camera, seconds);
}

// TODO: many things are recalculated, merge GameUpdate with GameDraw?
void GameDraw(GameStorage* gameStorage)
{
	GameState* gameState = gameStorage->gameState;
	Canvas canvas = gameState->canvas;

	V4 clearColor = MakeColor(0.0f, 0.0f, 0.0f);
	ClearScreen(canvas, clearColor);

	V2 playerPosition = {};
	if (gameState->isPlayerCar)
		playerPosition = gameState->playerCar.car.position;
	else
		playerPosition = gameState->playerHuman.human.position;

	Building* inBuilding = gameState->playerHuman.human.inBuilding;
	if (inBuilding && IsPointInBuilding(gameState->playerHuman.human.position, *inBuilding)) {
		DrawBuildingInside(canvas, *inBuilding);

		if (inBuilding && IsPointInBuilding(gameState->playerHuman.human.position, *inBuilding)) {
			V4 black = MakeColor(0.0f, 0.0f, 0.0f);

			Canvas maskData = gameState->maskCanvas;
			ClearScreen(maskData, black);

			DrawVisibleAreaInBuilding(maskData, *inBuilding, gameState->playerHuman.human.position, &gameStorage->tmpArena);

			ApplyBitmapMask(canvas.bitmap, maskData.bitmap);
		}
	} else {
		Map* map = &gameState->map;
		Camera* camera = canvas.camera;
		float visibleRadius = 50.0f;
		F32 left   = CameraLeftSide(camera)   - visibleRadius;
		F32 right  = CameraRightSide(camera)  + visibleRadius;
		F32 top    = CameraTopSide(camera)    - visibleRadius;
		F32 bottom = CameraBottomSide(camera) + visibleRadius;
		DrawVisibleMapTiles(canvas, map, left, right, top, bottom, &gameState->mapTextures);
		DrawAllTrafficLights(canvas, map);

		for (I32 i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* autoCar = &gameState->autoCars[i];
			DrawCar(canvas, autoCar->car);
		}

		for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			DrawAutoHuman(canvas, autoHuman);
		}
	}

	if (gameState->isPlayerCar)
		DrawCar(canvas, gameState->playerCar.car);
	else
		DrawPlayerHuman(canvas, &gameState->playerHuman);

	for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = gameState->autoHumans + i;
		Human* human = &autoHuman->human;
		if (human->isPolice)
			DrawPoliceRadius(canvas, human, 15.0f);
	}
}