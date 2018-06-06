#pragma once

#include "Building.hpp"
#include "Map.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "Renderer.hpp"

extern float humanRadius;
extern int maxHealthPoints;

struct Human {
	Map* map;
	Building* inBuilding;

	int isPolice;

	Point position;
	float moveSpeed;

	int healthPoints;
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

inline void DrawPoliceRadius(Renderer renderer, Human* human, float radius) {
	Color color = {0.0f, 0.0f, 1.0f};
	Point position = human->position;
	float left   = position.x - radius;
	float right  = position.x + radius;
	float top    = position.y - radius;
	float bottom = position.y + radius;
	DrawRectOutline(renderer, top, left, bottom, right, color);
}

void MoveHuman(Human* human, DirectedPoint point);
void DrawHuman(Renderer renderer, Human human);