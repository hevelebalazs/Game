#include "Vehicle.h"
#include <math.h>
#include <stdio.h>

static float PI = 3.14159265358979323f;

void Vehicle::update(float seconds) {
	while (seconds > 0.0f) {
		if (targetIntersection == 0 || targetIntersection == onIntersection) {
			seconds = 0.0f;
			startPoint = position;
			targetPoint = position;

			totalSeconds = 0.0f;

			break;
		}
		
		if (onIntersection) {
			if (nextRoad == 0) {
				nextRoad = nextRoadOnPath(*map, onIntersection, targetIntersection, pathHelper);

				if (nextRoad->intersection1 == onIntersection) targetPoint = nextRoad->endPoint1;
				else targetPoint = nextRoad->endPoint2;

				targetAngle = 0.0f;
				if (onIntersection->leftRoad == nextRoad) targetAngle = -PI;
				else if (onIntersection->rightRoad == nextRoad) targetAngle = 0.0f;
				else if (onIntersection->topRoad == nextRoad) targetAngle = -PI / 2.0f;
				else if (onIntersection->bottomRoad == nextRoad) targetAngle = PI / 2.0f;

				startAngle = angle;
				startPoint = position;

				while (startAngle - targetAngle > PI) {
					startAngle -= 2 * PI;
				}

				while (startAngle - targetAngle < -PI) {
					startAngle += 2 * PI;
				}

				totalSeconds = Point::cityDistance(startPoint, targetPoint) / maxSpeed;
				spentSeconds = 0.0f;
			}

			if (totalSeconds == spentSeconds) {
				onRoad = nextRoad;

				startAngle = targetAngle;
				targetAngle = targetAngle;

				if (onRoad->intersection1 == onIntersection) {
					startPoint = onRoad->endPoint1;
					targetPoint = onRoad->endPoint2;
					nextIntersection = onRoad->intersection2;
				}
				else if (onRoad->intersection2 == onIntersection) {
					startPoint = onRoad->endPoint2;
					targetPoint = onRoad->endPoint1;
					nextIntersection = onRoad->intersection1;
				}
				else {
					*(char*)0 = 0;
				}

				onIntersection = 0;
				nextRoad = 0;

				totalSeconds = Point::cityDistance(startPoint, targetPoint) / maxSpeed;
				spentSeconds = 0.0f;
			}
		}
		else if (onRoad) {
			if (totalSeconds == spentSeconds) {
				onIntersection = nextIntersection;
				nextRoad = 0;

				startPoint = targetPoint;
				startAngle = targetAngle;

				onRoad = 0;
				nextIntersection = 0;
			}
		}

		if (spentSeconds + seconds > totalSeconds) {
			seconds -= (totalSeconds - spentSeconds);
			spentSeconds = totalSeconds;
		}
		else {
			spentSeconds += seconds;
			seconds = 0.0;
		}

		angle = startAngle + (targetAngle - startAngle) * (spentSeconds / totalSeconds);
		position.x = startPoint.x + (targetPoint.x - startPoint.x) * (spentSeconds / totalSeconds);
		position.y = startPoint.y + (targetPoint.y - startPoint.y) * (spentSeconds / totalSeconds);
	}
}

void Vehicle::draw(Bitmap bitmap) {
	float addWidthX = (width / 2.0f) * cosf(angle + PI / 2.0f);
	float addWidthY = (width / 2.0f) * sinf(angle + PI / 2.0f);

	Point side1 = { position.x + addWidthX, position.y + addWidthY };
	Point side2 = { position.x - addWidthX, position.y - addWidthY };

	float addLengthX = (length / 2.0f) * cosf(angle);
	float addLengthY = (length / 2.0f) * sinf(angle);

	Point points[4] = {
		{ side1.x + addLengthX, side1.y + addLengthY },
		{ side1.x - addLengthX, side1.y - addLengthY },
		{ side2.x - addLengthX, side2.y - addLengthY },
		{ side2.x + addLengthX, side2.y + addLengthY }
	};

	bitmap.drawQuad(points, color);
}