#include "Point.h"
#include <math.h>

float Point::distanceSquare(Point point1, Point point2) {
	return (point1.x - point2.x) * (point1.x - point2.x) +
		(point1.y - point2.y) * (point1.y - point2.y);
}

float Point::cityDistance(Point point1, Point point2) {
	return fabsf(point1.x - point2.x) + fabsf(point1.y - point2.y);
}

Point Point::operator+(Point otherPoint) {
	return { x + otherPoint.x, y + otherPoint.y };
}

bool Point::operator==(Point otherPoint) {
	return x == otherPoint.x && y == otherPoint.y;
}