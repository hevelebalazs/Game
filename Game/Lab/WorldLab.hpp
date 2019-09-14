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
	camera->unitInPixels = 10.0f;
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
		Map *pushedMap = (Map *)ArenaPushVar(&fileArena, labState->map);
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

	DrawMap(canvas, &labState->map);
}