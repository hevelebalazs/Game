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

void DrawAutoHuman(Canvas canvas, AutoHuman* autoHuman); 
