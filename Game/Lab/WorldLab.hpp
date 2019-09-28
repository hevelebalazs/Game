#pragma once

#include <Windows.h>

#include "../Bitmap.hpp"
#include "../Debug.hpp"
#include "../Draw.hpp"
#include "../Geometry.hpp"
#include "../Map.hpp"
#include "../UserInput.hpp"

#define WorldLabArenaSize (1 * MegaByte)

enum WorldEditMode
{
	PlaceTileMode,
	PlaceItemMode
};

struct WorldLabState
{
	Int8 arenaMemory1[WorldLabArenaSize];
	Int8 arenaMemory2[WorldLabArenaSize];
	MemArena arena1;
	MemArena arena2;
	MemArena *arena;
	MemArena *tmpArena;

	Map map;

	WorldEditMode editMode;
};

static void
func WorldLabSwapArenas(WorldLabState *labState)
{
	MemArena *arena = labState->arena;
	labState->arena = labState->tmpArena;
	labState->tmpArena = arena;

	Assert((labState->arena == &labState->arena1) && (labState->tmpArena == &labState->arena2) ||
		   (labState->arena == &labState->arena2) && (labState->tmpArena == &labState->arena1));
}

static void
func WorldLabInit(WorldLabState *labState, Canvas* canvas)
{
	labState->arena1 = CreateMemArena(labState->arenaMemory1, WorldLabArenaSize);
	labState->arena2 = CreateMemArena(labState->arenaMemory2, WorldLabArenaSize);
	labState->arena = &labState->arena1;
	labState->tmpArena = &labState->arena2;

	labState->map = {};

	labState->editMode = PlaceTileMode;

	Camera *camera = canvas->camera;
	camera->unitInPixels = 100.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static void
func HandlePlaceTileMode(WorldLabState *labState, Canvas *canvas, UserInput *userInput)
{
	MemArena *arena = labState->arena;
	MemArena *tmpArena = labState->tmpArena;
	Map *map = &labState->map;

	Camera *camera = canvas->camera;
	Vec2 mousePosition = PixelToUnit(camera, userInput->mousePixelPosition);

	IntVec2 tile = {};
	tile.row = Floor(mousePosition.y / MapTileSide);
	tile.col = Floor(mousePosition.x / MapTileSide);

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		ArenaReset(tmpArena);

		if(map->tileRowN == 0)
		{
			Assert(map->tileColN == 0);
			Assert(map->tileTypes == 0);
			map->tileRowN = 1;
			map->tileColN = 1;
			map->tileTypes = ArenaPushArray(tmpArena, TileId, 1);

			IntVec2 newTile = MakeIntPoint(0, 0);
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

			TileId *newTiles = ArenaPushArray(tmpArena, TileId, newRowN * newColN);
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

			map->tileRowN = newRowN;
			map->tileColN = newColN;
			map->tileTypes = newTiles;

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

		WorldLabSwapArenas(labState);
	}

	MapItem *oldItems = map->items;
	map->items = ArenaPushArray(arena, MapItem, map->itemN);
	for(Int32 i = 0; i < map->itemN; i++)
	{
		map->items[i] = oldItems[i];
	}
	
	Rect tileRect = {};
	tileRect.left   = tile.col * MapTileSide;
	tileRect.right  = tileRect.left + MapTileSide;
	tileRect.top    = tile.row * MapTileSide;
	tileRect.bottom = tileRect.top + MapTileSide;

	Vec4 tileColor = MakeColor(0.5f, 0.5f, 0.5f);
	DrawRect(canvas, tileRect, tileColor);
}

static void
func HandlePlaceItemMode(WorldLabState *labState, Canvas *canvas, UserInput *userInput)
{
	Map *map = &labState->map;
	Camera *camera = canvas->camera;
	Vec2 mousePosition = PixelToUnit(camera, userInput->mousePixelPosition);

	Real32 itemRadius = 0.2f;
	Vec4 itemColor = MakeColor(1.0f, 0.0f, 1.0f);

	DrawCircle(canvas, mousePosition, itemRadius, itemColor);

	if(WasKeyReleased(userInput, VK_LBUTTON))
	{
		MemArena *arena = labState->arena;
		
		Int8 *arenaTop = GetArenaTop(arena);
		Int8 *itemsEnd = (Int8 *)&map->items[map->itemN];
		Assert(arenaTop == itemsEnd);

		ArenaAlloc(arena, sizeof(MapItem));
		MapItem *item = &map->items[map->itemN];
		item->itemId = NoItemId;
		item->position = mousePosition;
		map->itemN++;
	}
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
		camera->unitInPixels /= 1.01f;
	}
	if(IsKeyDown(userInput, 'E'))
	{
		camera->unitInPixels *= 1.01f;
	}

	if(WasKeyPressed(userInput, '1'))
	{
		labState->editMode = PlaceTileMode;
	}
	if(WasKeyPressed(userInput, '2'))
	{
		labState->editMode = PlaceItemMode;
	}

	MemArena *arena = labState->arena;
	MemArena *tmpArena = labState->tmpArena;
	if(WasKeyReleased(userInput, 'M'))
	{
		HANDLE file = CreateFileA(mapFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		Assert(file != INVALID_HANDLE_VALUE);

		MemArena *fileArena = tmpArena;
		Map *map = &labState->map;
		Map *pushedMap = (Map *)ArenaPushVar(fileArena, *map);
		Int32 tileN = (map->tileRowN * map->tileColN);

		TileId *tileTypes = (TileId *)ArenaPushData(fileArena, tileN * sizeof(TileId), map->tileTypes);
		pushedMap->tileTypes = (TileId *)GetRelativeAddress(tileTypes, fileArena->baseAddress);

		MapItem *items = (MapItem *)ArenaPushData(fileArena, map->itemN * sizeof(MapItem), map->items);
		pushedMap->items = (MapItem *)GetRelativeAddress(items, fileArena->baseAddress);

		Int8 *data = fileArena->baseAddress;
		DWORD dataSize = fileArena->usedSize;
		DWORD writtenDataSize = 0;

		BOOL result = WriteFile(file, (LPCVOID)data, (DWORD)dataSize, &writtenDataSize, 0);
		Assert(result);
		Assert(writtenDataSize == dataSize);

		result = CloseHandle(file);
		Assert(result);

		ArenaReset(tmpArena);
	}

	if(WasKeyReleased(userInput, 'L'))
	{
		labState->map = ReadMapFromFile(mapFile, arena);
	}

	Map *map = &labState->map;
	DrawMap(canvas, map);

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
	
	if(labState->editMode == PlaceTileMode)
	{
		HandlePlaceTileMode(labState, canvas, userInput);
	}
	else if(labState->editMode == PlaceItemMode)
	{
		HandlePlaceItemMode(labState, canvas, userInput);
	}
}