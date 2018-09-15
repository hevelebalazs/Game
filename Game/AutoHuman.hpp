#pragma once

#include "Draw.hpp"
#include "Human.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Type.hpp"

struct AutoHuman
{
	Human human;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	V4 moveStartPoint;
	V4 moveEndPoint;
	F32 moveTotalSeconds;
	F32 moveSeconds;

	B32 dead;
};

void MoveAutoHumanToJunction(AutoHuman* autoHuman, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoHuman(AutoHuman* autoHuman, F32 seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);
void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);

inline void DrawAutoHuman(Canvas canvas, AutoHuman* autoHuman) 
{
	if (autoHuman->dead) 
	{
		// TODO: this messes up other things that use random
		SeedRandom((I32)autoHuman->human.position.x + (I32)autoHuman->human.position.y);
		for (I32 i = 0; i < 5; ++i) 
		{
			F32 x = autoHuman->human.position.x + RandomBetween(-2.0f, 2.0f);
			F32 y = autoHuman->human.position.y + RandomBetween(-2.0f, 2.0f);

			F32 side = 0.5f;
			F32 left   = x - side * 0.5f;
			F32 right  = x + side * 0.5f;
			F32 top    = y - side * 0.5f;
			F32 bottom = y + side * 0.5f;

			// TODO: make this a global variable?
			V4 bloodColor = MakeColor(1.0f, 0.0f, 0.0f);
			DrawRect(canvas, left, right, top, bottom, bloodColor);
		}
		InitRandom();
	} 
	else 
	{
		DrawHuman(canvas, autoHuman->human);
	}
}