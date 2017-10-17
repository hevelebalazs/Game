#include "Intersection.h"

float Intersection::getRoadWidth() {
	if (leftRoad) return leftRoad->width;
	if (rightRoad) return rightRoad->width;
	if (topRoad) return topRoad->width;
	if (bottomRoad) return bottomRoad->width;

	return 0.0f;
}

void Intersection::highlight(Bitmap bitmap, Color color) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth = getRoadWidth();

		bitmap.drawRect(
			(int)(coordinate.y - roadWidth / 2), (int)(coordinate.x - roadWidth / 2),
			(int)(coordinate.y + roadWidth / 2), (int)(coordinate.x + roadWidth / 2),
			color
		);
	}
}

void Intersection::draw(Bitmap bitmap) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth;
		if (leftRoad) roadWidth = leftRoad->width;
		else if (rightRoad) roadWidth = rightRoad->width;
		else if (topRoad) roadWidth = topRoad->width;
		else if (bottomRoad) roadWidth = bottomRoad->width;

		Color color = { 0.5f, 0.5f, 0.5f };
		
		bitmap.drawRect(
			(int)(coordinate.y - roadWidth / 2), (int)(coordinate.x - roadWidth / 2),
			(int)(coordinate.y + roadWidth / 2), (int)(coordinate.x + roadWidth / 2),
			color
		);
	}
}