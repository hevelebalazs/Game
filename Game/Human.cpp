#include "Human.h"

extern float humanRadius = 0.5f;
extern Color humanColor = Color{1.0f, 0.0f, 0.0f};
extern int maxHealthPoints = 3;

static float healthPointPadding = 0.1f;
static Color fullHealthColor = {1.0f, 0.0f, 0.0f};
static Color emptyHealthColor = {0.0f, 0.0f, 0.0f};

void MoveHuman(Human* human, DirectedPoint point) {
	human->position = point.position;
}

static inline void DrawHealthPoints(Renderer renderer, Human* human) {
	int healthPoints = maxHealthPoints;

	float healthPointRadius = (2.0f * humanRadius + healthPointPadding) / ((float)maxHealthPoints);
	healthPointRadius -= healthPointPadding;

	float top    = human->position.y + humanRadius + healthPointPadding;
	float bottom = top + healthPointRadius;
	float left   = human->position.x - humanRadius;
	for (int i = 0; i < maxHealthPoints; ++i) {
		float right = left + healthPointRadius;

		Color color = {};
		if (i < human->healthPoints)
			color = fullHealthColor;
		else
			color = emptyHealthColor;

		DrawRect(renderer, top, left, bottom, right, color);

		left = right + healthPointPadding;
	}
}

// TODO: pass pointers to structs wherever possible and makes sense
void DrawHuman(Renderer renderer, Human human) {
	float radius = humanRadius;
	Point position = human.position;

	DrawRect(
		renderer,
		position.y - radius, position.x - radius,
		position.y + radius, position.x + radius,
		human.color
	);

	if (human.healthPoints < maxHealthPoints)
		DrawHealthPoints(renderer, &human);
}