#pragma once

#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

struct Game
{
	Int8 arenaMemory[GameArenaSize];
	MemArena arena;
	Map map;

	Vec2 playerPosition;
	Vec2 playerVelocity;
};

static void
func GameInit(Game *game, Canvas *canvas)
{
	game->arena = CreateMemArena(game->arenaMemory, GameArenaSize);

	Camera *camera = canvas->camera;
	camera->unitInPixels = 30;
	camera->center = MakePoint(0.0, 0.0);

	game->playerPosition = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	game->playerVelocity = MakeVector(0.0f, 0.0f);

	Int8 *mapFile = "Data/Map.data";
	game->map = ReadMapFromFile(mapFile, &game->arena); 
}

static void
func GameUpdate(Game *game, Canvas *canvas, Real32 seconds, UserInput *userInput)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Real32 playerMoveSpeed = 10.0f;
	game->playerVelocity.x = 0.0f;
	if(IsKeyDown(userInput, 'A'))
	{
		game->playerVelocity.x -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'D'))
	{
		game->playerVelocity.x += playerMoveSpeed;
	}

	game->playerVelocity.y = 0.0f;
	if(IsKeyDown(userInput, 'W'))
	{
		game->playerVelocity.y -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'S'))
	{
		game->playerVelocity.y += playerMoveSpeed;
	}

	game->playerPosition += seconds * game->playerVelocity;
	canvas->camera->center = game->playerPosition;

	DrawMap(canvas, &game->map);

	Vec4 playerColor = MakeColor(1.0f, 1.0f, 0.0f);
	Real32 playerSize = 1.0f;
	Rect playerRect = MakeSquareRect(game->playerPosition, playerSize);
	DrawRect(canvas, playerRect, playerColor);
}