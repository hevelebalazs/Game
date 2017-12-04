#pragma once

#include "Point.h"
#include "Renderer.h"
#include "Map.h"
#include "Building.h"
#include "Path.h"

struct Human {
	// TODO: remove globals from struct
	static float radius;
	static float moveSpeed;

	float needRed;
	float needGreen;
	float needBlue;

	Map* map;
	Building* inBuilding;

	Point position;
};

void DrawHuman(Renderer renderer, Human human);