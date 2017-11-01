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

Point Point::operator-(Point otherPoint) {
	return { x - otherPoint.x, y - otherPoint.y };
}

Point Point::operator+=(Point otherPoint) {
	*this = (*this + otherPoint);
	return (*this);
}

Point Point::operator-=(Point otherPoint) {
	(*this) = (*this - otherPoint);
	return (*this);
}

bool Point::operator==(Point otherPoint) {
	return x == otherPoint.x && y == otherPoint.y;
}

float Point::length() {
	return sqrtf(x * x + y * y);
}

Point Point::rotation(float angle) {
	return { cosf(angle), sinf(angle) };
}

Point operator*(float multiplier, Point point) {
	return { multiplier * point.x, multiplier * point.y };
}

Point operator*(Point point, float multiplier) {
	return multiplier * point;
}