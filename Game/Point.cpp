#include "Point.hpp"

Point PointSum(Point point1, Point point2) {
	Point result = {};
	result.x = (point1.x + point2.x);
	result.y = (point1.y + point2.y);
	return result;
}

Point PointDiff(Point point1, Point point2) {
	Point result = {};
	result.x = (point1.x - point2.x);
	result.y = (point1.y - point2.y);
	return result;
}

Point PointProd(float times, Point point) {
	Point result = {};
	result.x = (times * point.x);
	result.y = (times * point.y);
	return result;
}

bool PointEqual(Point point1, Point point2) {
	return ((point1.x == point2.x) && (point1.y == point2.y));
}