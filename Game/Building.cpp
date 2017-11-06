#include "Building.h"

float min2(float x, float y) {
	if (x < y) return x;
	else return y;
}

float max2(float x, float y) {
	if (x > y) return x;
	else return y;
}

void Building::draw(Renderer renderer) {
	renderer.drawRect(
		top, left, bottom, right,
		color
	);

	float boxPadding = 2.0f;
	Color boxColor = Color{ 0.0f, 0.0f, 1.0f };

	Point center = {
		(left + right) * 0.5f,
		(top + bottom) * 0.5f
	};

	float boxTop = min2(center.y, connectRoad.y) - boxPadding;
	float boxLeft = min2(center.x, connectRoad.x) - boxPadding;
	float boxBottom = max2(center.y, connectRoad.y) + boxPadding;
	float boxRight = max2(center.x, connectRoad.x) + boxPadding;

	renderer.drawRect(
		boxTop, boxLeft, boxBottom, boxRight,
		boxColor
	);
}