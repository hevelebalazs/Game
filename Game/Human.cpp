#include "Human.h"

float Human::moveSpeed = 10.0f;

void Human::Draw(Renderer renderer) {
	float radius = 1.0f;

	Color color = Color {
		needRed / 100.0f,
		needGreen / 100.0f,
		needBlue / 100.0f
	};

	renderer.DrawRect(
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		color
	);
}