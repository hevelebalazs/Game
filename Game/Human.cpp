#include "Human.h"

extern float humanRadius = 0.5f;
extern float humanMoveSpeed = 10.0f;

void DrawHuman(Renderer renderer, Human human) {
	float radius = humanRadius;
	Point position = human.position;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		human.color
	);
}