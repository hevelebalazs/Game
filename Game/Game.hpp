#pragma once

#include "Map.hpp"
#include "UserInput.hpp"

#define GameArenaSize (1 * MegaByte)

struct Entity
{
	Vec2 position;
	Vec2 velocity;
};

struct Game
{
	Int8 arenaMemory[GameArenaSize];
	MemArena arena;
	Map map;

	Entity player;
};

static void
func GameInit(Game *game, Canvas *canvas)
{
	game->arena = CreateMemArena(game->arenaMemory, GameArenaSize);

	Camera *camera = canvas->camera;
	camera->unitInPixels = 30;
	camera->center = MakePoint(0.0, 0.0);

	Entity *player = &game->player;
	player->position = MakePoint(0.5f * MapTileSide, 0.5f * MapTileSide);
	player->velocity = MakeVector(0.0f, 0.0f);

	Int8 *mapFile = "Data/Map.data";
	game->map = ReadMapFromFile(mapFile, &game->arena); 
}

#define EntityRadius 0.5f
#define EntitySide (2.0f * EntityRadius)

static Rect
func GetEntityRect(Entity *entity)
{
	Rect rect = MakeSquareRect(entity->position, EntitySide);
	return rect;
}

static Vec2
func GetEntityLeft(Entity *entity)
{
	Vec2 left = entity->position + MakeVector(-EntityRadius, 0.0f);
	return left;
}

static Vec2
func GetEntityRight(Entity *entity)
{
	Vec2 right = entity->position + MakeVector(+EntityRadius, 0.0f);
	return right;
}

static Vec2
func GetEntityTop(Entity *entity)
{
	Vec2 top = entity->position + MakeVector(0.0f, -EntityRadius);
	return top;
}

static Vec2
func GetEntityBottom(Entity *entity)
{
	Vec2 bottom = entity->position + MakeVector(0.0f, +EntityRadius);
	return bottom;
}

static void
func UpdateEntityMovement(Entity *entity, Map *map, Real32 seconds)
{
	Vec2 moveVector = seconds * entity->velocity;
	Vec2 oldPosition = entity->position;
	Vec2 newPosition = entity->position + moveVector;

	IntVec2 topTile = GetContainingTile(map, GetEntityTop(entity));
	topTile.row--;

	IntVec2 bottomTile = GetContainingTile(map, GetEntityBottom(entity));
	bottomTile.row++;

	IntVec2 leftTile = GetContainingTile(map, GetEntityLeft(entity));
	leftTile.col--;

	IntVec2 rightTile = GetContainingTile(map, GetEntityRight(entity));
	rightTile.col++;

	for(Int32 row = topTile.row; row <= bottomTile.row; row++)
	{
		for(Int32 col = leftTile.col; col <= rightTile.col; col++)
		{
			IntVec2 tile = MakeTile(row, col);
			if(IsValidTile(map, tile) && IsTileType(map, tile, NoTileId))
			{
				Poly16 collisionPoly = {};
				Rect tileRect = GetTileRect(map, tile);
				Rect collisionRect = GetExtendedRect(tileRect, EntityRadius);

				if(IsBetween(oldPosition.x, collisionRect.left, collisionRect.right))
				{
					if(oldPosition.y <= collisionRect.top && newPosition.y > collisionRect.top)
					{
						newPosition.y = collisionRect.top;
					}

					if(oldPosition.y >= collisionRect.bottom && newPosition.y < collisionRect.bottom)
					{
						newPosition.y = collisionRect.bottom;
					}
				}

				if(IsBetween(oldPosition.y, collisionRect.top, collisionRect.bottom))
				{
					if(oldPosition.x <= collisionRect.left && newPosition.x > collisionRect.left)
					{
						newPosition.x = collisionRect.left;
					}

					if(oldPosition.x >= collisionRect.right && newPosition.x < collisionRect.right)
					{
						newPosition.x = collisionRect.right;
					}
				}
			}
		}
	}

	Real32 mapWidth  = GetMapWidth(map);
	Real32 mapHeight = GetMapHeight (map);
	newPosition.x = Clip(newPosition.x, EntityRadius, mapWidth  - EntityRadius);
	newPosition.y = Clip(newPosition.y, EntityRadius, mapHeight - EntityRadius);

	entity->position = newPosition;
}

static void
func GameUpdate(Game *game, Canvas *canvas, Real32 seconds, UserInput *userInput)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Entity *player = &game->player;

	Real32 playerMoveSpeed = 10.0f;
	player->velocity.x = 0.0f;
	if(IsKeyDown(userInput, 'A'))
	{
		player->velocity.x -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'D'))
	{
		player->velocity.x += playerMoveSpeed;
	}

	player->velocity.y = 0.0f;
	if(IsKeyDown(userInput, 'W'))
	{
		player->velocity.y -= playerMoveSpeed;
	}
	if(IsKeyDown(userInput, 'S'))
	{
		player->velocity.y += playerMoveSpeed;
	}

	Map *map = &game->map;
	UpdateEntityMovement(player, map, seconds);
	canvas->camera->center = player->position;

	DrawMap(canvas, map);

	Vec4 playerColor = MakeColor(1.0f, 1.0f, 0.0f);
	Rect playerRect = GetEntityRect(player);
	DrawRect(canvas, playerRect, playerColor);
}