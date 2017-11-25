#pragma once

#include "Point.h"
#include "Renderer.h"
#include "Map.h"
#include "Building.h"
#include "Path.h"

struct Human {
	static float radius;
	static float moveSpeed;

	float needRed;
	float needGreen;
	float needBlue;

	Map* map;

	Point position;

	void Draw(Renderer renderer);
};
