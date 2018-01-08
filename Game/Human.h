#pragma once

#include "Building.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

extern float humanRadius;
extern float humanMoveSpeed;

struct Human {
	Map* map;
	Building* inBuilding;

	Color color;

	Point position;
};

void MoveHuman(Human* human, DirectedPoint point);
void DrawHuman(Renderer renderer, Human human);