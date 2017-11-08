#include "Intersection.h"

float Intersection::GetRoadWidth() {
	if (leftRoad) return leftRoad->width;
	if (rightRoad) return rightRoad->width;
	if (topRoad) return topRoad->width;
	if (bottomRoad) return bottomRoad->width;

	return 0.0f;
}

void Intersection::Highlight(Renderer renderer, Color color) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth = GetRoadWidth();

		float stripeWidth = roadWidth / 10.0f;
		float top = coordinate.y - (roadWidth / 2.0f);
		float bottom = coordinate.y + (roadWidth / 2.0f);
		float left = coordinate.x - (roadWidth / 2.0f);
		float right = coordinate.x + (roadWidth / 2.0f);

		renderer.DrawRect(top, left, bottom, right, color);
	}
}

void Intersection::Draw(Renderer renderer) {
	float roadWidth = GetRoadWidth();

	float top = coordinate.y - (roadWidth / 2.0f);
	float bottom = coordinate.y + (roadWidth / 2.0f);
	float left = coordinate.x - (roadWidth / 2.0f);
	float right = coordinate.x + (roadWidth / 2.0f);

	float midX = (left + right) / 2;
	float midY = (top + bottom) / 2;

	Color color = { 0.5f, 0.5f, 0.5f };
	renderer.DrawRect(top, left, bottom, right, color);

	float stripeWidth = roadWidth / 20.0f;
	Color stripeColor = { 1.0f, 1.0f, 1.0f };

	int roadCount = 0;

	if (topRoad) roadCount++;
	if (leftRoad) roadCount++;
	if (bottomRoad) roadCount++;
	if (rightRoad) roadCount++;

	if (roadCount > 2) {
		if (topRoad) {
			renderer.DrawRect(top, left, top + stripeWidth, right, stripeColor);
		}
		if (leftRoad) {
			renderer.DrawRect(top, left, bottom, left + stripeWidth, stripeColor);
		}
		if (bottomRoad) {
			renderer.DrawRect(bottom - stripeWidth, left, bottom, right, stripeColor);
		}
		if (rightRoad) {
			renderer.DrawRect(top, right - stripeWidth, bottom, right, stripeColor);
		}
	}
	else {
		if (topRoad) {
			renderer.DrawRect(top, midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (leftRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), left, midY + (stripeWidth / 2.0f), midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (bottomRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), bottom, midX + (stripeWidth / 2.0f), stripeColor);
		}
		if (rightRoad) {
			renderer.DrawRect(midY - (stripeWidth / 2.0f), midX - (stripeWidth / 2.0f), midY + (stripeWidth / 2.0f), right, stripeColor);
		}
	}
}