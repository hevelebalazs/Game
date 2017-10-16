#include "Vehicle.h"
#include <math.h>

void Vehicle::update(float seconds) {
	if (onIntersection) {
		position = onIntersection->coordinate;

		if (!targetIntersection || onIntersection == targetIntersection) {
			speed = 0.0f;
		}
		else {
			Intersection *nextIntersection = nextIntersectionOnPath(*map, onIntersection, targetIntersection, pathHelper);

			onRoad = 0;

			if (onIntersection->leftRoad && onIntersection->leftRoad->otherIntersection(onIntersection) == nextIntersection) {
				onRoad = onIntersection->leftRoad;
				orientationx = -1;
				orientationy = 0;
			}
			else if (onIntersection->rightRoad && onIntersection->rightRoad->otherIntersection(onIntersection) == nextIntersection) {
				onRoad = onIntersection->rightRoad;
				orientationx = 1;
				orientationy = 0;
			}
			else if (onIntersection->topRoad && onIntersection->topRoad->otherIntersection(onIntersection) == nextIntersection) {
				onRoad = onIntersection->topRoad;
				orientationx = 0;
				orientationy = -1;
			}
			else if (onIntersection->bottomRoad && onIntersection->bottomRoad->otherIntersection(onIntersection) == nextIntersection) {
				onRoad = onIntersection->bottomRoad;
				orientationx = 0;
				orientationy = 1;
			}

			if (onRoad->intersection1 == nextIntersection) onRoadTarget = 1;
			else onRoadTarget = 2;

			onRoadLength = 0.0f;

			onIntersection = 0;
		}
	}
	else if (onRoad) {
		speed = maxSpeed;
	}
	else {
		speed = 0.0f;
	}

	position.x += seconds * speed * orientationx;
	position.y += seconds * speed * orientationy;

	if (onRoad) {
		onRoadLength += seconds * speed;

		float totalRoadLength;

		if (onRoad->endPoint1.x == onRoad->endPoint2.x) {
			totalRoadLength = fabsf(onRoad->endPoint1.y - onRoad->endPoint2.y);
		}
		else {
			totalRoadLength = fabsf(onRoad->endPoint1.x - onRoad->endPoint2.x);
		}

		if (onRoadLength >= totalRoadLength) {
			Intersection *nextIntersection = 0;
			if (onRoadTarget == 1) nextIntersection = onRoad->intersection1;
			else nextIntersection = onRoad->intersection2;

			onIntersection = nextIntersection;
			position = onIntersection->coordinate;
			onRoad = 0;
		}
	}
}

void Vehicle::draw(Bitmap bitmap) {
	if (onRoad) {
		float pathWidth = 5.0f;

		Intersection *nextIntersection = 0;

		if (onRoadTarget == 1) nextIntersection = onRoad->intersection1;
		else nextIntersection = onRoad->intersection2;

		IntersectionPath path = findConnectingPath(*map, nextIntersection, targetIntersection, pathHelper);
		drawIntersectionPath(path, bitmap, pathWidth);
		delete path.intersections;

		Color color = { 1.0f, 0.5f, 0.0f };

		if (onRoad->endPoint1.y == onRoad->endPoint2.y) {
			bitmap.drawRect(
				(int)(position.y - pathWidth / 2), (int)(position.x),
				(int)(nextIntersection->coordinate.y + pathWidth / 2), (int)(nextIntersection->coordinate.x),
				color
			);
		}
		else {
			bitmap.drawRect(
				(int)(position.y), (int)(position.x - pathWidth / 2),
				(int)(nextIntersection->coordinate.y), (int)(nextIntersection->coordinate.x + pathWidth / 2),
				color
			);
		}
	}

	if (orientationx == 0) {
		bitmap.drawRect(
			(int)(position.y - length / 2), (int)(position.x - width / 2),
			(int)(position.y + length / 2), (int)(position.x + width / 2),
			color
		);
	}
	else {
		bitmap.drawRect(
			(int)(position.y - width / 2), (int)(position.x - length / 2),
			(int)(position.y + width / 2), (int)(position.x + length / 2),
			color
		);
	}
}