#pragma once

#include "Human.h"
#include "Math.h"
#include "Memory.h"
#include "Path.h"
#include "Renderer.h"

struct AutoHuman {
	Human human;

	Intersection* onIntersection;

	PathNode* moveNode;
	Intersection* moveTargetIntersection;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;

	bool dead;
};

void MoveAutoHumanToIntersection(AutoHuman* autoHuman, Intersection* intersection, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);
void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);

inline void DrawAutoHuman(Renderer renderer, AutoHuman* autoHuman) {
	if (autoHuman->dead) {
		SeedRandom((int)autoHuman->human.position.x + (int)autoHuman->human.position.y);
		for (int i = 0; i < 5; ++i) {
			float x = autoHuman->human.position.x + RandomBetween(-2.0f, 2.0f);
			float y = autoHuman->human.position.y + RandomBetween(-2.0f, 2.0f);

			float side = 0.5f;
			float left   = x - side * 0.5f;
			float right  = x + side * 0.5f;
			float top    = y - side * 0.5f;
			float bottom = y + side * 0.5f;

			// TODO: make this a global variable?
			Color bloodColor = Color{1.0f, 0.0f, 0.0f};

			DrawRect(renderer, top, left, bottom, right, bloodColor);
		}
	}
	else {
		DrawHuman(renderer, autoHuman->human);
	}
}