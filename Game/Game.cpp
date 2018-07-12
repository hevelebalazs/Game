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
	playerCar->maxEngineForce = 1000.0f;
	playerCar->breakForce     = 1000.0f;
	playerCar->mass           = 200.0f;
	
	car->angle  = 0.0f;
	car->length = 5.0f;
	car->width  = 2.3f;
	I32 carBitmapIndex = IntRandom(0, CarBitmapN - 1);
	car->bitmap = &gameState->carBitmaps[carBitmapIndex];

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
		autoCar->onJunction = GetRandomJunction(map);
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

	gameState->missionJunction = GetRandomJunction(&gameState->map);
	gameState->onMission = false;

	MapElem startElem = GetJunctionElem(startJunction);
	MapElem endElem = GetJunctionElem(gameState->missionJunction);
	gameState->missionPath = ConnectElems(&gameState->map, startElem, endElem, 
										  &gameStorage->tmpArena, &gameState->pathPool);

	map->generateTileWorkList.semaphore = CreateSemaphore(0, 0, MaxGenerateMapTileWorkListN, 0);
	for (I32 i = 0; i < GenerateMapTileWorkThreadN; ++i)
		CreateThread(0, 0, GenerateMapTileWorkProc, &map->generateTileWorkList, 0, 0);

	map->drawTileWorkList.semaphore = CreateSemaphore(0, 0, MaxDrawMapTileWorkListN, 0);
	map->drawTileWorkList.semaphoreDone = CreateSemaphore(0, 0, MaxDrawMapTileWorkListN, 0);
	for (I32 i = 0; i < DrawMapTileWorkThreadN; ++i)
		CreateThread(0, 0, DrawMapTileWorkProc, &map->drawTileWorkList, 0, 0);

	GenerateMapTextures(&gameState->mapTextures, &gameStorage->tmpArena);
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

		car->moveSpeed = car->maxSpeed;

		// TODO: move this logic to UpdateAutoCar?
		Quad stopArea = GetCarStopArea(car);

		V2 playerPosition = {};
		if (gameState->isPlayerCar)
			playerPosition = gameState->playerCar.car.position;
		else
			playerPosition = gameState->playerHuman.human.position;

		if (IsPointInQuad(stopArea, playerPosition))
				car->moveSpeed = 0.0f;

		for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			Human* human = &autoHuman->human;
			if (IsPointInQuad(stopArea, human->position))
				car->moveSpeed = 0.0f;
		}

		for (I32 i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* testAutoCar = &gameState->autoCars[i];
			if (autoCar == testAutoCar)
				continue;
			Car* testCar = &testAutoCar->car;
			if (IsPointInQuad(stopArea, testCar->position))
				car->moveSpeed = 0.0f;
		}

		UpdateAutoCar(autoCar, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
	}

	for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = &gameState->autoHumans[i];
		UpdateAutoHuman(autoHuman, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
	}

	if (gameState->isPlayerCar) {
		MapElem onElemBefore = GetRoadElemAtPoint(&gameState->map, gameState->playerCar.car.position);

		UpdatePlayerCar(&gameState->playerCar, seconds);

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

	if (gameState->showFullMap)
		camera->targetUnitInPixels = 0.1f;

	UpdateCamera(camera, seconds);

	// TODO: create StartMission function?
	V4 missionHighlightColor = MakeColor(0.0f, 1.0f, 1.0f);
	V2 playerPosition = {};
	if (gameState->isPlayerCar)
		playerPosition = gameState->playerCar.car.position;
	else
		playerPosition = gameState->playerHuman.human.position;

	V2 missionStartPoint = {};
	missionStartPoint = playerPosition;

	MapElem playerElem = {};
	if (gameState->isPlayerCar)
		playerElem = GetRoadElemAtPoint(&gameState->map, playerPosition);
	else
		playerElem = GetPedestrianElemAtPoint(&gameState->map, playerPosition);

	if ((playerElem.type == MapElemJunction || playerElem.type == MapElemJunctionSidewalk)
		&& (playerElem.junction == gameState->missionJunction)
	) {
		Junction* startJunction = gameState->missionJunction;
		Junction* endJunction = GetRandomJunction(&gameState->map);

		gameState->missionJunction = endJunction;
		gameState->onMission = !gameState->onMission;

		FreePath(gameState->missionPath, &gameState->pathPool);
	
		if (gameState->isPlayerCar) {
			MapElem startElem = GetJunctionElem(startJunction);
			MapElem endElem = GetJunctionElem(endJunction);
			gameState->missionPath = ConnectElems(&gameState->map, startElem, endElem,
												  &gameStorage->tmpArena, &gameState->pathPool);
		} else {
			MapElem startElem = GetJunctionSidewalkElem(startJunction);
			MapElem endElem = GetJunctionSidewalkElem(endJunction);
			I32 startCornerIndex = GetClosestJunctionCornerIndex(startJunction, playerPosition);
			I32 endCornerIndex = GetRandomJunctionCornerIndex(endJunction);
			gameState->missionPath = ConnectPedestrianElems(&gameState->map, startElem, startCornerIndex, endElem, endCornerIndex,
														  &gameStorage->tmpArena, &gameState->pathPool);
		}
	}

	B32 recalculatePath = false;
	I32 missionLaneIndex = gameState->missionLaneIndex;

	if (!AreMapElemsEqual(playerElem, gameState->missionElem))
		recalculatePath = true;

	if (playerElem.type == MapElemRoad) {
		missionLaneIndex = LaneIndex(playerElem.road, playerPosition);
		if (missionLaneIndex != gameState->missionLaneIndex)
			recalculatePath = true;

		missionStartPoint = ClosestLanePoint(playerElem.road, missionLaneIndex, playerPosition);
	} else if (playerElem.type == MapElemCrossing) {
		if (gameState->missionElem.type == MapElemRoadSidewalk && playerElem.road == gameState->missionElem.road)
			recalculatePath = false;
	} else if (playerElem.type == MapElemNone) {
		if (gameState->missionPath)
			FreePath(gameState->missionPath, &gameState->pathPool);

		gameState->missionPath = 0;
		recalculatePath = false;
	}

	if (recalculatePath) {
		FreePath(gameState->missionPath, &gameState->pathPool);

		if (gameState->isPlayerCar) {
			if (playerElem.type == MapElemJunction) {
				MapElem pathEndElem = GetJunctionElem(gameState->missionJunction);

				gameState->missionPath = ConnectElems(&gameState->map, playerElem, pathEndElem, 
													  &gameStorage->tmpArena, &gameState->pathPool);
			} else if (playerElem.type == MapElemRoad) {
				Junction* startJunction = 0;
				if (missionLaneIndex > 0)
					startJunction = playerElem.road->junction2;
				else if (missionLaneIndex < 0)
					startJunction = playerElem.road->junction1;

				MapElem startJunctionElem = GetJunctionElem(startJunction);
				MapElem pathEndElem = GetJunctionElem(gameState->missionJunction);

				gameState->missionPath = ConnectElems(&gameState->map, startJunctionElem, pathEndElem,
													  &gameStorage->tmpArena, &gameState->pathPool);

				gameState->missionPath = PrefixPath(playerElem, gameState->missionPath, &gameState->pathPool);
			} else {
				gameState->missionPath = 0;
			}
		} else {
			if (playerElem.type == MapElemRoadSidewalk 
					 || playerElem.type == MapElemJunctionSidewalk 
					 || playerElem.type == MapElemCrossing
			) {
				MapElem sidewalkElemEnd = GetJunctionSidewalkElem(gameState->missionJunction);
			} else {
				gameState->missionPath = 0;
			}
		}
	}

	gameState->missionElem = playerElem;
	gameState->missionLaneIndex = missionLaneIndex;
	gameState->missionStartPoint = missionStartPoint;

	gameState->time += seconds;
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

		V4 missionHighlightColor = MakeColor(0.0f, 1.0f, 1.0f);
		if (gameState->missionJunction)
			// HighlightJunction(gameState->canvas, *gameState->missionJunction, missionHighlightColor);

		if (gameState->missionPath) {
			V4 startPoint = {};
			startPoint.position = gameState->missionStartPoint;

			// DrawBezierPathFromPoint(gameState->canvas, gameState->missionPath, startPoint, missionHighlightColor, 1.0f);
		}

		for (I32 i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* autoCar = &gameState->autoCars[i];
			DrawCar(canvas, autoCar->car);
		}

		for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			DrawAutoHuman(canvas, autoHuman);
		}

		DrawBuildings(canvas, &gameState->map, &gameStorage->tmpArena, &gameState->assets);
	}

	if (gameState->isPlayerCar)
		DrawCar(canvas, gameState->playerCar.car);
	else
		DrawPlayerHuman(canvas, &gameState->playerHuman);

	// TODO: put this whole lighting thing in a function
	/*
	Canvas maskCanvas = gameState->maskCanvas;
	Color dark = {0.2f, 0.2f, 0.2f};
	ClearScreen(maskRenderer, dark);
	Bitmap maskBitmap = maskRenderer.bitmap;

	if (gameState->isPlayerCar) {
		Car* car = &gameState->playerCar.car;
		
		Point addWidth = PointProd(
			(car->width * 0.25f), 
			RotationVector(car->angle + 0.5f * PI)
		);
		Point lightCenter1 = PointDiff(car->position, addWidth);
		Point lightCenter2 = PointSum(car->position, addWidth);

		F32 brightness = 0.8f;
		F32 maxDistance = 15.0f;
		F32 minDistance = 4.0f;
		F32 minAngle = NormalizeAngle(car->angle - 0.1f);
		F32 maxAngle = NormalizeAngle(car->angle + 0.1f);
		LightSector(maskRenderer, lightCenter1, minDistance, maxDistance, minAngle, maxAngle, brightness);
		LightSector(maskRenderer, lightCenter2, minDistance, maxDistance, minAngle, maxAngle, brightness);
	}

	for (I32 i = 0; i < gameState->map.roadCount; ++i) {
		Road* road = gameState->map.roads + i;
		F32 length = RoadLength(road);
		F32 roadX = 0.0f;
		while (roadX < length) {
			F32 roadY = -road->width * 0.5f;
			Point roadCoord = Point{roadX, roadY};
			Point position = PointFromRoadCoord(road, roadCoord, 1);
			LightCircle(maskRenderer, position, 15.0f, 0.8f);
			roadX += 50.0f;
		}
		roadX = 25.0f;
		while (roadX < length) {
			F32 roadY = road->width * 0.5f;
			Point roadCoord = Point{roadX, roadY};
			Point position = PointFromRoadCoord(road, roadCoord, 1);
			LightCircle(maskRenderer, position, 15.0f, 0.8f);
			roadX += 50.0f;
		}
	}

	ApplyBitmapMask(canvas.bitmap, maskRenderer.bitmap);
	*/

	for (I32 i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = gameState->autoHumans + i;
		Human* human = &autoHuman->human;
		if (human->isPolice)
			DrawPoliceRadius(canvas, human, 15.0f);
	}
}