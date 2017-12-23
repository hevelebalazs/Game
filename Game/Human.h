#pragma once

#include "Building.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

extern float humanRadius;
extern float humanMoveSpeed;

struct Human {
	float needRed;
	float needGreen;
	float needBlue;

	Map* map;
	Building* inBuilding;

	Color color;

	Point position;
};

void DrawHuman(Renderer renderer, Human human);