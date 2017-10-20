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

				if (nextRoad->intersection1 == onIntersection) targetPoint = nextRoad->enterPoint(1);
				else targetPoint = nextRoad->enterPoint(2);

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

				if (startAngle == targetAngle) {
					rotationMovement = false;
					totalSeconds = Point::cityDistance(startPoint, targetPoint) / maxSpeed;
				}
				else if (startPoint.x == targetPoint.x) {
					rotationMovement = true;

					rotationPoint.x = startPoint.x;
					rotationPoint.y = (startPoint.y + targetPoint.y) / 2.0f;

					rotationSide = fabsf(startPoint.y - targetPoint.y) / 2.0f;

					if (startPoint.y > targetPoint.y) {
						rotationStartAngle = PI / 2.0f;
						rotationTargetAngle = -PI / 2.0f;

						startAngle = 0.0f;
						targetAngle = -PI;
					}
					else {
						rotationStartAngle = -PI / 2.0f;
						rotationTargetAngle = -3.0f * PI / 2.0f;

						startAngle = PI;
						targetAngle = 0.0f;
					}

					totalSeconds = (rotationSide * PI) / maxSpeed;
				}
				else if (startPoint.y == targetPoint.y) {
					rotationMovement = true;

					rotationPoint.y = startPoint.y;
					rotationPoint.x = (startPoint.x + targetPoint.x) / 2.0f;

					rotationSide = fabsf(startPoint.x - targetPoint.x) / 2.0f;

					if (startPoint.x > targetPoint.x) {
						rotationStartAngle = 0.0f;
						rotationTargetAngle = -PI;

						startAngle = 3.0f * PI / 2.0f;
						targetAngle = PI / 2.0f;
					}
					else {
						rotationStartAngle = PI;
						rotationTargetAngle = 0.0f;

						startAngle = PI / 2.0f;
						targetAngle = -PI / 2.0f;
					}

					totalSeconds = (rotationSide * PI) / maxSpeed;
				}
				else {
					rotationMovement = true;
					rotationSide = fabsf(startPoint.x - targetPoint.x);

					rotationTargetAngle = startAngle;
					rotationStartAngle = targetAngle + PI;

					if (rotationStartAngle - rotationTargetAngle > PI) rotationStartAngle -= 2 * PI;
					if (rotationStartAngle - rotationTargetAngle < -PI) rotationStartAngle += 2 * PI;

					rotationPoint = {
						startPoint.x + rotationSide * cosf(targetAngle),
						startPoint.y + rotationSide * sinf(targetAngle)
					};

					totalSeconds = (rotationSide * PI / 2.0f) / maxSpeed;
				}

				spentSeconds = 0.0f;
			}

			if (totalSeconds == spentSeconds) {
				onRoad = nextRoad;

				startAngle = targetAngle;
				targetAngle = targetAngle;

				if (onRoad->intersection1 == onIntersection) {
					startPoint = onRoad->enterPoint(1);

					targetPoint = onRoad->leavePoint(2);
					nextIntersection = onRoad->intersection2;
				}
				else if (onRoad->intersection2 == onIntersection) {
					startPoint = onRoad->enterPoint(2);

					targetPoint = onRoad->leavePoint(1);
					nextIntersection = onRoad->intersection1;
				}

				onIntersection = 0;
				nextRoad = 0;

				totalSeconds = Point::cityDistance(startPoint, targetPoint) / maxSpeed;
				spentSeconds = 0.0f;

				rotationMovement = false;
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

		float timeRatio = 0.0f;

		if (totalSeconds > 0.0f) timeRatio = (spentSeconds / totalSeconds);

		angle = startAngle + (targetAngle - startAngle) * (timeRatio);

		if (rotationMovement == false) {
			position.x = startPoint.x + (targetPoint.x - startPoint.x) * (timeRatio);
			position.y = startPoint.y + (targetPoint.y - startPoint.y) * (timeRatio);
		}
		else {
			float rotationAngle = rotationStartAngle + (rotationTargetAngle - rotationStartAngle) * (timeRatio);

			position.x = rotationPoint.x + rotationSide * cosf(rotationAngle);
			position.y = rotationPoint.y + rotationSide * sinf(rotationAngle);
		}
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