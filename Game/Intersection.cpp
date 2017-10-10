#include "Intersection.h"

void Intersection::draw(HDC context) {
	if (leftRoad || rightRoad || topRoad || bottomRoad) {
		HBRUSH brushGrey = CreateSolidBrush(RGB(128, 128, 128));
		int roadWidth;
		if (leftRoad)
			roadWidth = leftRoad->width;
		else if (rightRoad)
			roadWidth = rightRoad->width;
		else if (topRoad)
			roadWidth = topRoad->width;
		else if (bottomRoad)
			roadWidth = bottomRoad->width;
		RECT rect = { (LONG)(coordinate.x - roadWidth / 2), (LONG)(coordinate.y - roadWidth / 2), (LONG)(coordinate.x + roadWidth / 2), (LONG)(coordinate.y + roadWidth / 2) };
		FillRect(context, &rect, brushGrey);
	}
}