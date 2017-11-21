#include "PlayerHuman.h"

void PlayerHuman::Update(float seconds) {
	moveDirection = Point {0.0f, 0.0f};

	bool moveX = false;
	bool moveY = false;

	if (moveLeft) {
		moveDirection.x = -1.0f;
		moveX = true;
	}

	if (moveRight) {
		moveDirection.x = 1.0f;
		moveX = true;
	}

	if (moveUp) {
		moveDirection.y = -1.0f;
		moveY = true;
	}

	if (moveDown) {
		moveDirection.y = 1.0f;
		moveY = true;
	}

	if (moveX && moveY) moveDirection = moveDirection * (1.0f / sqrtf(2.0f));

	human.position += seconds * Human::moveSpeed * moveDirection;
}