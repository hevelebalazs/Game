#include "Human.h"

extern float humanRadius = 0.5f;
extern float humanMoveSpeed = 15.0f;
extern Color humanColor = Color{1.0f, 0.0f, 0.0f};

void MoveHuman(Human* human, DirectedPoint point) {
	human->position = point.position;
}

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