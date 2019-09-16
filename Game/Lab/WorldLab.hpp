#pragma once

#include <Windows.h>

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"
#include "../Map.hpp"
#include "../UserInput.hpp"

#define WorldLabArenaSize (1 * MegaByte)

struct WorldLabState
{
	Int8 arenaMemory[WorldLabArenaSize];
	MemArena arena;
	Map map;
	Bitmap bitmap;
};

static void
func WorldLabInit(WorldLabState *labState, Canvas* canvas)
{
	labState->arena = CreateMemArena(labState->arenaMemory, WorldLabArenaSize);

	Camera *camera = canvas->camera;
	camera->unitInPixels = 100.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static Int8 *mapFile = "Data/Map.data";

static void
func WorldLabUpdate(WorldLabState *labState, Canvas *canvas, Real32 seconds, UserInput *userInput)
{
	Bitmap *bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Real32 cameraMoveSpeed = seconds * 50.0f;
	Camera *camera = canvas->camera;

	if(IsKeyDown(userInput, 'A'))
	{
		camera->center.x -= cameraMoveSpeed;
	}
	if(IsKeyDown(userInput, 'D'))
	{
		camera->center.x += cameraMoveSpeed;
	}

	if(IsKeyDown(userInput, 'W'))
	{
		camera->center.y -= cameraMoveSpeed;
	}
	if(IsKeyDown(userInput, 'S'))
	{
		camera->center.y += cameraMoveSpeed;
	}

	if(IsKeyDown(userInput, 'Q'))
	{
		camera->unitInPixels /= 1.10f;
	}
	if(IsKeyDown(userInput, 'E'))
	{
		camera->unitInPixels *= 1.10f;
	}

	if(WasKeyReleased(userInput, 'M'))
	{
		HANDLE file = CreateFileA(mapFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		Assert(file != INVALID_HANDLE_VALUE);

		MemArena fileArena = CreateSubArena(&labState->arena, 512 * KiloByte);
		Map *map = &labState->map;
		Map *pushedMap = (Map *)ArenaPushVar(&fileArena, *map);
		Int32 tileN = (map->tileRowN * map->tileColN);

		TileId *tileTypes = (TileId *)ArenaPushData(&fileArena, tileN * sizeof(TileId), map->tileTypes);
		pushedMap->tileTypes = (TileId *)GetRelativeAddress(tileTypes, fileArena.baseAddress);

		Int8 *data = fileArena.baseAddress;
		DWORD dataSize = fileArena.usedSize;
		DWORD writtenDataSize = 0;

		BOOL result = WriteFile(file, (LPCVOID)data, (DWORD)dataSize, &writtenDataSize, 0);
		Assert(result);
		Assert(writtenDataSize == dataSize);

		result = CloseHandle(file);
		Assert(result);

		ArenaPopTo(&labState->arena, fileArena.baseAddress);
	}

	if(WasKeyReleased(userInput, 'L'))
	{
		HANDLE file = CreateFileA(mapFile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		Assert(file != INVALID_HANDLE_VALUE);

		labState->arena.usedSize = 0;
		DWORD fileSize = GetFileSize(file, 0);
		Assert(fileSize <= labState->arena.maxSize);

		DWORD readSize = 0;
		bool result = ReadFile(file, labState->arena.baseAddress, fileSize, &readSize, 0);
		Assert(result);
		Assert(fileSize == readSize);

		labState->arena.usedSize += fileSize;

		Int8 *base = labState->arena.baseAddress;
		Map *map = (Map *)base;
		map->tileTypes = (TileId *)GetAbsoluteAddress(map->tileTypes, base);

		labState->map = *map;

		result = CloseHandle(file);
		Assert(result);
	}

	Map* map = &labState->map;
	DrawMap(canvas, &labState->map);

	if(map->tileRowN > 0)
	{
		Assert(map->tileColN > 0);
		Rect rect = {};
		rect.left = 0.0f;
		rect.right = GetMapWidth(map);
		rect.top = 0.0f;
		rect.bottom = GetMapHeight(map);

		Vec4 borderColor = MakeColor(1.0f, 1.0f, 0.0);
		DrawRectOutline(canvas, rect, borderColor);
	}

	Vec2 mousePosition = PixelToUnit(camera, userInput->mousePixelPosition);
	
	IntVec2 tile = {};
	tile.row = Floor(mousePosition.y / MapTileSide);
	tile.col = Floor(mousePosition.x / MapTileSide);

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		if(map->tileRowN == 0)
		{
			Assert(map->tileColN == 0);
			Assert(map->tileTypes == 0);
			map->tileRowN = 1;
			map->tileColN = 1;
			map->tileTypes = ArenaPushArray(&labState->arena, TileId, 1);

			IntVec2 newTile = {};
			newTile.row = 0;
			newTile.col = 0;

			SetTileType(map, newTile, CaveTileId);

			camera->center.y -= tile.row * MapTileSide;
			camera->center.x -= tile.col * MapTileSide;
		}
		else
		{
			Assert(map->tileColN > 0);
			Assert(map->tileRowN > 0);

			Int32 newRowN = map->tileRowN;
			Int32 newColN = map->tileColN;

			if(tile.row < 0)
			{
				newRowN += (-tile.row);
			}
			else if(tile.row >= map->tileRowN)
			{
				newRowN += (tile.row - map->tileRowN + 1);
			}

			if(tile.col < 0)
			{
				newColN += (-tile.col);
			}
			else if(tile.col >= map->tileColN)
			{
				newColN += (tile.col - map->tileColN + 1);
			}

			MemArena *arena = &labState->arena;
			Int8 *tileDataEnd = (Int8 *)&map->tileTypes[map->tileRowN * map->tileColN];
			Int8 *arenaEnd = GetArenaTop(arena);
			Assert(tileDataEnd == arenaEnd);

			TileId *newTiles = ArenaPushArray(arena, TileId, newRowN * newColN);
			for(Int32 index = 0; index < newRowN * newColN; index++)
			{
				newTiles[index] = NoTileId;
			}

			for(Int32 row = 0; row < map->tileRowN; row++)
			{
				for(Int32 col = 0; col < map->tileColN; col++)
				{
					Int32 oldIndex = map->tileColN * row + col;
					Int32 newRow = row;
					if(tile.row < 0)
					{
						newRow += (-tile.row);
					}

					Int32 newCol = col;
					if(tile.col < 0)
					{
						newCol += (-tile.col);
					}

					Int32 newIndex = newColN * newRow + newCol;
					newTiles[newIndex] = map->tileTypes[oldIndex];
				}
			}

			Int32 indexDifference = (map->tileRowN * map->tileColN);
			for(Int32 index = 0; index < newRowN * newColN; index++)
			{
				map->tileTypes[index] = map->tileTypes[index + indexDifference];
			}

			map->tileRowN = newRowN;
			map->tileColN = newColN;

			arena->usedSize -= indexDifference * sizeof(TileId);

			if(tile.row < 0)
			{
				camera->center.y += (-tile.row) * MapTileSide;
			}
			if(tile.col < 0)
			{
				camera->center.x += (-tile.col) * MapTileSide;
			}

			tile.row = IntMax2(tile.row, 0);
			tile.col = IntMax2(tile.col, 0);
			SetTileType(map, tile, CaveTileId);
		}
	}
	
	Rect tileRect = {};
	tileRect.left   = tile.col * MapTileSide;
	tileRect.right  = tileRect.left + MapTileSide;
	tileRect.top    = tile.row * MapTileSide;
	tileRect.bottom = tileRect.top + MapTileSide;

	Vec4 tileColor = MakeColor(0.5f, 0.5f, 0.5f);
	DrawRect(canvas, tileRect, tileColor);
}