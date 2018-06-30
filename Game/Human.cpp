#include "Human.hpp"
#include "Type.hpp"

void MoveHuman(Human* human, V4 point)
{
	human->position = point.position;
}

static void DrawHealthPoints(Canvas canvas, Human* human)
{
	I32 healthPoints = MaxHealthPoints;

	F32 healthPointRadius = (2.0f * HumanRadius + HealthPointPadding) / ((F32)MaxHealthPoints);
	healthPointRadius -= HealthPointPadding;

	F32 top    = human->position.y + HumanRadius + HealthPointPadding;
	F32 bottom = top + healthPointRadius;
	F32 left   = human->position.x - HumanRadius;
	for (I32 i = 0; i < MaxHealthPoints; ++i) {
		F32 right = left + healthPointRadius;

		V4 color = {};
		if (i < human->healthPoints)
			color = FullHealthColor;
		else
			color = EmptyHealthColor;

		DrawRect(canvas, left, right, top, bottom, color);

		left = right + HealthPointPadding;
	}
}

void DrawHuman(Canvas canvas, Human human)
{
	F32 radius = HumanRadius;
	V2 position = human.position;

	V4 color = {};
	if (human.healthPoints == 0)
		color = GetColor(0.0f, 0.0f, 0.0f);
	else if (human.isPolice)
		color = GetColor(0.0f, 0.0f, 1.0f);
	else
		color = GetColor(1.0f, 1.0f, 1.0f);

	DrawRect(
		canvas,
		position.x - radius, position.x + radius,
		position.y - radius, position.y + radius,
		color
	);

	if (human.healthPoints < MaxHealthPoints)
		DrawHealthPoints(canvas, &human);
}