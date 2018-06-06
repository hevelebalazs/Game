// TODO: use a unity build?

#include "Car.hpp"
#include "Game.hpp"
#include "GridMap.hpp"
#include "Light.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Renderer.hpp"

void TogglePlayerCar(GameState* gameState)
{
	Point playerPosition = {};

	if (gameState->isPlayerCar) 
		playerPosition = gameState->playerCar.car.position;
	else 
		playerPosition = gameState->playerHuman.human.position;

	MapElem playerOnElem = RoadElemAtPoint(gameState->map, playerPosition);
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

static inline void ResizeCamera(Camera* camera, int width, int height)
{
	camera->screenSize = Point{(float)width, (float)height};
	camera->center = PointProd(0.5f, camera->screenSize);
}

static inline void ResizeBitmap(Bitmap* bitmap, int width, int height)
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

	int bytesPerPixel = 4;
	int bitmapMemorySize = (bitmap->width * bitmap->height) * bytesPerPixel;

	bitmap->memory = (void*)(new char[bitmapMemorySize]);
}

void WinResize(GameState* gameState, int width, int height)
{
	if (!gameState)
		return;

	ResizeCamera(&gameState->camera, width, height);

	ResizeBitmap(&gameState->renderer.bitmap, width, height);
	ResizeBitmap(&gameState->maskRenderer.bitmap, width, height);

	gameState->renderer.camera = &gameState->camera;
	gameState->maskRenderer.camera = &gameState->camera;
}

void GameInit(GameStorage* gameStorage, int windowWidth, int windowHeight)
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

	int junctionRowN = 10;
	int junctionColN = 10;
	int junctionN = junctionRowN * junctionColN;
	int roadN = 100;
	Map* map = &gameState->map;
	map->junctions = ArenaPushArray(arena, Junction, junctionN);
	map->roads = ArenaPushArray(arena, Road, roadN);
	GenerateGridMap(map, junctionRowN, junctionColN, roadN, tmpArena);

	int maxPathNodeCount = 10000;
	gameState->pathPool.maxNodeCount = maxPathNodeCount;
	gameState->pathPool.nodes = ArenaPushArray(arena, PathNode, maxPathNodeCount);

	WinResize(gameState, windowWidth, windowHeight);

	PlayerHuman* playerHuman = &gameState->playerHuman;
	Junction* startJunction = RandomJunction(gameState->map);
	playerHuman->human.position = startJunction->position;
	playerHuman->human.map = &gameState->map;
	playerHuman->human.moveSpeed = 5.0f;
	playerHuman->human.healthPoints = maxHealthPoints;

	for (int i = 0; i < CarBitmapN; ++i) {
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
	int carBitmapIndex = IntRandom(0, CarBitmapN - 1);
	car->bitmap = &gameState->carBitmaps[carBitmapIndex];

	for (int i = 0; i < gameState->autoCarCount; ++i) {
		// TODO: create an InitAutoCar function?
		Junction* randomJunction = RandomJunction(gameState->map);

		AutoCar* autoCar = &gameState->autoCars[i];
		Car* car = &autoCar->car;

		car->angle = 0.0f;
		car->position = randomJunction->position;
		car->length = 5.0f;
		car->width = 2.3f;
		car->map = &gameState->map;
		car->maxSpeed = RandomBetween(MinCarSpeed, MaxCarSpeed);
		int carBitmapIndex = IntRandom(0, CarBitmapN - 1);
		car->bitmap = &gameState->carBitmaps[carBitmapIndex];
		autoCar->onJunction = RandomJunction(*map);
	}

	for (int i = 0; i < gameState->autoHumanCount; ++i) {
		// TODO: create an InitAutoHuman function?
		AutoHuman* autoHuman = gameState->autoHumans + i;
		Human* human = &autoHuman->human;

		Junction* junction = RandomJunction(gameState->map);
		int cornerIndex = GetRandomJunctionCornerIndex(junction);
		Point position = GetJunctionCorner(junction, cornerIndex);

		human->position = position;
		human->inBuilding = 0;
		human->isPolice = 0;
		human->map = &gameState->map;
		human->moveSpeed = RandomBetween(2.0f, 10.0f);
		human->healthPoints = maxHealthPoints;
		autoHuman->onJunction = junction;
	}

	Camera* camera = &gameState->camera;
	camera->moveSpeed = 100.0f;
	camera->altitude = 30.0f;
	camera->center = Point{(float)windowWidth * 0.5f, (float)windowHeight * 0.5f};

	gameState->renderer.camera = camera;
	gameState->maskRenderer.camera = camera;

	gameState->missionJunction = RandomJunction(gameState->map);
	gameState->onMission = false;

	MapElem startElem = JunctionElem(startJunction);
	MapElem endElem = JunctionElem(gameState->missionJunction);
	gameState->missionPath = ConnectElems(&gameState->map, startElem, endElem, 
										  &gameStorage->tmpArena, &gameState->pathPool);

	GameAssets* assets = &gameState->assets;
	assets->roadTexture = RandomGreyTexture(6, 100, 127);
	assets->stripeTexture = RandomGreyTexture(6, 200, 255);
	assets->sidewalkTexture = RandomGreyTexture(6, 70, 100);
	assets->grassTexture = GrassTexture(10, &gameStorage->tmpArena);
	assets->roofTextureDown = RoofTexture(6);
	assets->roofTextureUp = CopyTexture(&assets->roofTextureDown);
	RotateTextureUpsideDown(&assets->roofTextureUp);
	assets->roofTextureRight = CopyTexture(&assets->roofTextureUp);
	RotateTextureRight(&assets->roofTextureRight);
	assets->roofTextureLeft = CopyTexture(&assets->roofTextureUp);
	RotateTextureLeft(&assets->roofTextureLeft);
}

// TODO: get rid of the mousePosition parameter?
void GameUpdate(GameStorage* gameStorage, float seconds, Point mousePosition)
{
	GameState* gameState = gameStorage->gameState;
	MemArena* arena = &gameStorage->arena;
	MemArena* tmpArena = &gameStorage->tmpArena;

	for (int i = 0; i < gameState->map.junctionCount; ++i) {
		Junction* junction = &gameState->map.junctions[i];
		UpdateTrafficLights(junction, seconds);
	}

	for (int i = 0; i < gameState->autoCarCount; ++i) {
		AutoCar* autoCar = &gameState->autoCars[i];
		Car* car = &autoCar->car;

		car->moveSpeed = car->maxSpeed;

		// TODO: move this logic to UpdateAutoCar?
		Quad stopArea = GetCarStopArea(car);

		Point playerPosition = {};
		if (gameState->isPlayerCar)
			playerPosition = gameState->playerCar.car.position;
		else
			playerPosition = gameState->playerHuman.human.position;

		if (IsPointInQuad(stopArea, playerPosition))
				car->moveSpeed = 0.0f;

		for (int i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			Human* human = &autoHuman->human;
			if (IsPointInQuad(stopArea, human->position))
				car->moveSpeed = 0.0f;
		}

		for (int i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* testAutoCar = &gameState->autoCars[i];
			if (autoCar == testAutoCar)
				continue;
			Car* testCar = &testAutoCar->car;
			if (IsPointInQuad(stopArea, testCar->position))
				car->moveSpeed = 0.0f;
		}

		UpdateAutoCar(autoCar, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
	}

	for (int i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = &gameState->autoHumans[i];
		UpdateAutoHuman(autoHuman, seconds, &gameStorage->tmpArena, &gameStorage->tmpArena, &gameState->pathPool);
	}

	if (gameState->isPlayerCar) {
		MapElem onElemBefore = RoadElemAtPoint(gameState->map, gameState->playerCar.car.position);

		UpdatePlayerCar(&gameState->playerCar, seconds);

		for (int i = 0; i < gameState->autoHumanCount; ++i) {
			// TODO: use array + index instead of &array[index] everywhere
			AutoHuman* autoHuman = gameState->autoHumans + i;
			Point autoHumanPoint = autoHuman->human.position;

			if (IsCarOnPoint(&gameState->playerCar.car, autoHumanPoint)) {
				KillAutoHuman(autoHuman, &gameState->pathPool);
			}
		}
	}
	else {
		PlayerHuman* playerHuman = &gameState->playerHuman;
		UpdatePlayerHuman(playerHuman, seconds, gameState);
	}
	
	Camera* camera = gameState->renderer.camera;

	if (gameState->isPlayerCar) {
		camera->center = gameState->playerCar.car.position;

		// TODO: create a speed variable in PlayerCar?
		float speed = VectorLength(gameState->playerCar.velocity);

		camera->targetAltitude = 20.0f + (1.0f * speed);
	}
	else {
		camera->center = gameState->playerHuman.human.position;

		if (gameState->playerHuman.human.inBuilding)
			camera->targetAltitude = 15.0f;
		else
			camera->targetAltitude = 15.0f;
	}

	if (gameState->showFullMap)
		camera->targetAltitude = 1000.0f;

	UpdateCamera(camera, seconds);

	// TODO: create StartMission function?
	Color missionHighlightColor = {0.0f, 1.0f, 1.0f};
	Point playerPosition = {};
	if (gameState->isPlayerCar)
		playerPosition = gameState->playerCar.car.position;
	else
		playerPosition = gameState->playerHuman.human.position;

	Point missionStartPoint = {};
	missionStartPoint = playerPosition;

	MapElem playerElem = {};
	if (gameState->isPlayerCar)
		playerElem = RoadElemAtPoint(gameState->map, playerPosition);
	else
		playerElem = PedestrianElemAtPoint(gameState->map, playerPosition);

	if ((playerElem.type == MapElemJunction || playerElem.type == MapElemJunctionSidewalk)
		&& (playerElem.junction == gameState->missionJunction)
	) {
		Junction* startJunction = gameState->missionJunction;
		Junction* endJunction = RandomJunction(gameState->map);

		gameState->missionJunction = endJunction;
		gameState->onMission = !gameState->onMission;

		FreePath(gameState->missionPath, &gameState->pathPool);
	
		if (gameState->isPlayerCar) {
			MapElem startElem = JunctionElem(startJunction);
			MapElem endElem = JunctionElem(endJunction);
			gameState->missionPath = ConnectElems(&gameState->map, startElem, endElem,
												  &gameStorage->tmpArena, &gameState->pathPool);
		}
		else {
			MapElem startElem = JunctionSidewalkElem(startJunction);
			MapElem endElem = JunctionSidewalkElem(endJunction);
			int startCornerIndex = GetClosestJunctionCornerIndex(startJunction, playerPosition);
			int endCornerIndex = GetRandomJunctionCornerIndex(endJunction);
			gameState->missionPath = ConnectPedestrianElems(&gameState->map, startElem, startCornerIndex, endElem, endCornerIndex,
														  &gameStorage->tmpArena, &gameState->pathPool);
		}
	}

	bool recalculatePath = false;
	int missionLaneIndex = gameState->missionLaneIndex;

	if (!MapElemEqual(playerElem, gameState->missionElem))
		recalculatePath = true;

	if (playerElem.type == MapElemRoad) {
		missionLaneIndex = LaneIndex(playerElem.road, playerPosition);
		if (missionLaneIndex != gameState->missionLaneIndex)
			recalculatePath = true;

		missionStartPoint = ClosestLanePoint(playerElem.road, missionLaneIndex, playerPosition);
	}
	else if (playerElem.type == MapElemCrossing) {
		if (gameState->missionElem.type == MapElemRoadSidewalk && playerElem.road == gameState->missionElem.road)
			recalculatePath = false;
	}
	else if (playerElem.type == MapElemNone) {
		if (gameState->missionPath)
			FreePath(gameState->missionPath, &gameState->pathPool);

		gameState->missionPath = 0;
		recalculatePath = false;
	}

	if (recalculatePath) {
		FreePath(gameState->missionPath, &gameState->pathPool);

		if (gameState->isPlayerCar) {
			if (playerElem.type == MapElemJunction) {
				MapElem pathEndElem = JunctionElem(gameState->missionJunction);

				gameState->missionPath = ConnectElems(&gameState->map, playerElem, pathEndElem, 
													  &gameStorage->tmpArena, &gameState->pathPool);
			}
			else if (playerElem.type == MapElemRoad) {
				Junction* startJunction = 0;
				if (missionLaneIndex > 0)
					startJunction = playerElem.road->junction2;
				else if (missionLaneIndex < 0)
					startJunction = playerElem.road->junction1;

				MapElem startJunctionElem = JunctionElem(startJunction);
				MapElem pathEndElem = JunctionElem(gameState->missionJunction);

				gameState->missionPath = ConnectElems(&gameState->map, startJunctionElem, pathEndElem,
													  &gameStorage->tmpArena, &gameState->pathPool);

				gameState->missionPath = PrefixPath(playerElem, gameState->missionPath, &gameState->pathPool);
			}
			else {
				gameState->missionPath = 0;
			}
		}
		else {
			if (playerElem.type == MapElemRoadSidewalk 
					 || playerElem.type == MapElemJunctionSidewalk 
					 || playerElem.type == MapElemCrossing
			) {
				MapElem sidewalkElemEnd = JunctionSidewalkElem(gameState->missionJunction);
			}
			else {
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
	Renderer renderer = gameState->renderer;

	Color clearColor = Color{0.0f, 0.0f, 0.0f};
	ClearScreen(renderer, clearColor);

	Point playerPosition = {};
	if (gameState->isPlayerCar)
		playerPosition = gameState->playerCar.car.position;
	else
		playerPosition = gameState->playerHuman.human.position;

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
		DrawGroundElems(renderer, &gameState->map, &gameState->assets);

		Color missionHighlightColor = {0.0f, 1.0f, 1.0f};
		if (gameState->missionJunction)
			// HighlightJunction(gameState->renderer, *gameState->missionJunction, missionHighlightColor);

		if (gameState->missionPath) {
			DirectedPoint startPoint = {};
			startPoint.position = gameState->missionStartPoint;

			// DrawBezierPathFromPoint(gameState->renderer, gameState->missionPath, startPoint, missionHighlightColor, 1.0f);
		}

		for (int i = 0; i < gameState->autoCarCount; ++i) {
			AutoCar* autoCar = &gameState->autoCars[i];
			DrawCar(renderer, autoCar->car);
		}

		for (int i = 0; i < gameState->autoHumanCount; ++i) {
			AutoHuman* autoHuman = &gameState->autoHumans[i];
			DrawAutoHuman(renderer, autoHuman);
		}

		DrawBuildings(renderer, &gameState->map, &gameStorage->tmpArena, &gameState->assets);
	}

	if (gameState->isPlayerCar)
		DrawCar(renderer, gameState->playerCar.car);
	else
		DrawPlayerHuman(renderer, &gameState->playerHuman);

	// TODO: put this whole lighting thing in a function
	/*
	Renderer maskRenderer = gameState->maskRenderer;
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

		float brightness = 0.8f;
		float maxDistance = 15.0f;
		float minDistance = 4.0f;
		float minAngle = NormalizeAngle(car->angle - 0.1f);
		float maxAngle = NormalizeAngle(car->angle + 0.1f);
		LightSector(maskRenderer, lightCenter1, minDistance, maxDistance, minAngle, maxAngle, brightness);
		LightSector(maskRenderer, lightCenter2, minDistance, maxDistance, minAngle, maxAngle, brightness);
	}

	for (int i = 0; i < gameState->map.roadCount; ++i) {
		Road* road = gameState->map.roads + i;
		float length = RoadLength(road);
		float roadX = 0.0f;
		while (roadX < length) {
			float roadY = -road->width * 0.5f;
			Point roadCoord = Point{roadX, roadY};
			Point position = PointFromRoadCoord(road, roadCoord, 1);
			LightCircle(maskRenderer, position, 15.0f, 0.8f);
			roadX += 50.0f;
		}
		roadX = 25.0f;
		while (roadX < length) {
			float roadY = road->width * 0.5f;
			Point roadCoord = Point{roadX, roadY};
			Point position = PointFromRoadCoord(road, roadCoord, 1);
			LightCircle(maskRenderer, position, 15.0f, 0.8f);
			roadX += 50.0f;
		}
	}

	ApplyBitmapMask(renderer.bitmap, maskRenderer.bitmap);
	*/

	for (int i = 0; i < gameState->autoHumanCount; ++i) {
		AutoHuman* autoHuman = gameState->autoHumans + i;
		Human* human = &autoHuman->human;
		if (human->isPolice)
			DrawPoliceRadius(renderer, human, 15.0f);
	}
}