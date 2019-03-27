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

static void func WorldLabInit(WorldLabState* labState, Canvas* canvas)
{
	labState->arena = CreateMemArena(labState->arenaMemory, WorldLabArenaSize);
	labState->map = GenerateForestMap(&labState->arena);

	Camera* camera = canvas->camera;
	camera->unitInPixels = 10.0f;
	camera->center = MakePoint(0.0f, 0.0f);
}

static void func WorldLabUpdate(WorldLabState* labState, Canvas* canvas, Real32 seconds, UserInput* userInput)
{
	Bitmap* bitmap = &canvas->bitmap;
	Vec4 backgroundColor = MakeColor(0.0f, 0.0f, 0.0f);
	FillBitmapWithColor(bitmap, backgroundColor);

	Real32 cameraMoveSpeed = seconds * 50.0f;
	Camera* camera = canvas->camera;

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

	DrawMap(canvas, &labState->map);
}