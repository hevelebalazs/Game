#pragma once

#include "Building.h"
#include "Map.h"
#include "Path.h"
#include "Point.h"
#include "Renderer.h"

extern float humanRadius;

struct Human {
	Map* map;
	Building* inBuilding;

	Color color;

	Point position;
	float moveSpeed;
};

inline bool IsHumanCrossedByLine(Human* human, Line line) {
	float left   = (human->position.x - humanRadius);
	float right  = (human->position.x + humanRadius);
	float top    = (human->position.y - humanRadius);
	float bottom = (human->position.y + humanRadius);

	if (line.x1 < left && line.x2 < left)
		return false;
	if (line.x1 > right && line.x2 > right)
		return false;
	if (line.y1 < top && line.y2 < top)
		return false;
	if (line.y1 > bottom && line.y2 > bottom)
		return false;

	Point topLeft     = Point{left,  top};
	Point topRight    = Point{right, top};
	Point bottomLeft  = Point{left,  bottom};
	Point bottomRight = Point{right, bottom};

	if (DoLinesCross(line.p1, line.p2, topLeft, topRight))
		return true;
	else if (DoLinesCross(line.p1, line.p2, topRight, bottomRight))
		return true;
	else if (DoLinesCross(line.p1, line.p2, bottomRight, bottomLeft))
		return true;
	else if (DoLinesCross(line.p1, line.p2, bottomLeft, topLeft))
		return true;
	else
		return false;
}

void MoveHuman(Human* human, DirectedPoint point);
void DrawHuman(Renderer renderer, Human human);