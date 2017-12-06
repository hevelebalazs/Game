#include "Human.h"

float Human::radius = 0.5f;
float Human::moveSpeed = 30.0f;

void DrawHuman(Renderer renderer, Human human) {
	Color color = Color {
		human.needRed / 100.0f,
		human.needGreen / 100.0f,
		human.needBlue / 100.0f
	};

	float radius = human.radius;
	Point position = human.position;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		color
	);
}