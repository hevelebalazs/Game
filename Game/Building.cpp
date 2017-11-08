#include "Building.h"

float Building::connectRoadWidth;

// TODO: move these to a math file?
float Min2(float x, float y) {
	if (x < y) return x;
	else return y;
}

float Max2(float x, float y) {
	if (x > y) return x;
	else return y;
}

bool Building::IsCrossed(Point point1, Point point2) {
	if (point1.x < left && point2.x < left) return false;
	if (point1.x > right && point2.x > right) return false;
	if (point1.y < top && point2.y < top) return false;
	if (point1.y > bottom && point2.y > bottom) return false;
	return true;
}

Point Building::ClosestCrossPoint(Point closePoint, Point farPoint) {
	Point result = {};

	if (closePoint.x < farPoint.x) {
		result.x = left;
		result.y = closePoint.y;
	}
	else if (closePoint.x > farPoint.x) {
		result.x = right;
		result.y = closePoint.y;
	}
	else if (closePoint.y < farPoint.y) {
		result.x = closePoint.x;
		result.y = top;
	}
	else if (closePoint.y > farPoint.y) {
		result.x = closePoint.x;
		result.y = bottom;
	}

	return result;
}

void Building::Draw(Renderer renderer) {
	// TODO: make this a static member of Road?
	Color roadColor = Color{0.5f, 0.5f, 0.5f};
	
	if (roadAround) {
		renderer.DrawRect(
			top - connectRoadWidth, left - connectRoadWidth,
			bottom + connectRoadWidth, right + connectRoadWidth,
			roadColor
		);
	}

	float connectPadding = connectRoadWidth * 0.5f;

	Point point1 = connectBuilding;
	Point point2 = connectRoad;

	if (connectBuilding.x == connectRoad.x) {
		point1.x -= connectPadding;
		point2.x += connectPadding;
	}
	else if (connectBuilding.y == connectRoad.y) {
		point1.y -= connectPadding;
		point2.y += connectPadding;
	}

	renderer.DrawRect(
		point1.y, point1.x, point2.y, point2.x,
		roadColor
	);

	renderer.DrawRect(
		top, left, bottom, right,
		color
	);
}