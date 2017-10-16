#include "Intersection.h"

void Intersection::highlight(Bitmap bitmap) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		float roadWidth;
		if (leftRoad) roadWidth = leftRoad->width;
		else if (rightRoad) roadWidth = rightRoad->width;
		else if (topRoad) roadWidth = topRoad->width;
		else if (bottomRoad) roadWidth = bottomRoad->width;

		Color color = { 1.0f, 0.5f, 0.0f };

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