#pragma once

#include "Human.hpp"
#include "Math.hpp"
#include "Memory.hpp"
#include "Path.hpp"
#include "Renderer.hpp"

struct AutoHuman {
	Human human;

	Junction* onJunction;

	PathNode* moveNode;
	Junction* moveTargetJunction;

	DirectedPoint moveStartPoint;
	DirectedPoint moveEndPoint;
	float moveTotalSeconds;
	float moveSeconds;

	bool dead;
};

void MoveAutoHumanToJunction(AutoHuman* autoHuman, Junction* junction, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);
void UpdateAutoHuman(AutoHuman* autoHuman, float seconds, MemArena* arena, MemArena* tmpArena, PathPool* pathPool);

void KillAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);
void DamageAutoHuman(AutoHuman* autoHuman, PathPool* pathPool);

inline void DrawAutoHuman(Renderer renderer, AutoHuman* autoHuman) {
	if (autoHuman->dead) {
		// TODO: this messes up other things that use random
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
		InitRandom();
	}
	else {
		DrawHuman(renderer, autoHuman->human);
	}
}