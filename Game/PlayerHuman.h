#pragma once

#include "Human.h"
#include "Point.h"

struct PlayerHuman {
	Human human;

	bool moveUp;
	bool moveDown;
	bool moveLeft;
	bool moveRight;

	Point moveDirection;

	void Update(float seconds);
};