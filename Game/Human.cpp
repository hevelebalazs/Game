#include "Human.h"

float Human::radius = 0.5f;
float Human::moveSpeed = 20.0f;

void DrawHuman(Renderer renderer, Human human) {
	float radius = human.radius;
	Point position = human.position;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		human.color
	);
}