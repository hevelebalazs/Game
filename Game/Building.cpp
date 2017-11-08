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

Road* Building::GetConnectedRoad() {
	Building* building = this;

	while (building->connectBuilding) building = building->connectBuilding;

	return building->connectRoad;
}

bool Building::IsPointInside(Point point) {
	if (point.x < left || point.x > right) return false;
	if (point.y < top || point.y > bottom) return false;
	return true;
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

void Building::HighLight(Renderer renderer, Color color) {
	renderer.DrawRect(
		top, left, bottom, right,
		color
	);
}

void Building::Draw(Renderer renderer) {
	if (top > bottom || left > right) {
		color = Color{0.0f, 0.0f, 1.0f};
	}

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

	Point point1 = connectPointClose;
	Point point2 = connectPointFar;

	if (connectPointClose.x == connectPointFar.x) {
		point1.x -= connectPadding;
		point2.x += connectPadding;
	}
	else if (connectPointClose.y == connectPointFar.y) {
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