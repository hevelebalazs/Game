#include "Intersection.h"

float Intersection::getRoadWidth() {
	if (leftRoad) return leftRoad->width;
	if (rightRoad) return rightRoad->width;
	if (topRoad) return topRoad->width;
	if (bottomRoad) return bottomRoad->width;

	return 0.0f;
}

void Intersection::highlight(Renderer renderer, Color color) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth = getRoadWidth();

		float stripeWidth = roadWidth / 10.0f;
		float top = coordinate.y - (roadWidth / 2.0f);
		float bottom = coordinate.y + (roadWidth / 2.0f);
		float left = coordinate.x - (roadWidth / 2.0f);
		float right = coordinate.x + (roadWidth / 2.0f);

		renderer.drawRect(top, left, bottom, right, color);
	}
}

void Intersection::draw(Renderer renderer) {
	float roadWidth = getRoadWidth();

	float top = coordinate.y - (roadWidth / 2.0f);
	float bottom = coordinate.y + (roadWidth / 2.0f);
	float left = coordinate.x - (roadWidth / 2.0f);
	float right = coordinate.x + (roadWidth / 2.0f);

	float midX = (left + right) / 2;
	float midY = (top + bottom) / 2;

	Color color = { 0.5f, 0.5f, 0.5f };
	renderer.drawRect(top, left, bottom, right, color);

	float stripeWidth = roadWidth / 20.0f;
	Color stripeColor = { 1.0f, 1.0f, 1.0f };

	int roadCount = 0;

	if (topRoad) roadCount++;
	if (leftRoad) roadCount++;
	if (bottomRoad) roadCount++;
	if (rightRoad) roadCount++;

	if (roadCount > 2) {
		if (topRoad) {
			renderer.drawRect(top, left, top + stripeWidth, right, stripeColor);
		}
		if (leftRoad) {
			renderer.drawRect(top, left, bottom, left + stripeWidth, stripeColor);
		}
		if (bottomRoad) {
			renderer.drawRect(bottom - stripeWidth, left, bottom, right, stripeColor);
		}
		if (rightRoad) {
			renderer.drawRect(top, right - stripeWidth, bottom, right, stripeColor);
		}
	}
	else {
		if (topRoad) {
			renderer.drawRect(top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (leftRoad) {
			renderer.drawRect(midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (bottomRoad) {
			renderer.drawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (rightRoad) {
			renderer.drawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
		}
	}
}