#include "Point.h"

Point Point::operator+(Point otherPoint) {
	return { x + otherPoint.x, y + otherPoint.y };
}

bool Point::operator==(Point otherPoint) {
	return x == otherPoint.x && y == otherPoint.y;
}