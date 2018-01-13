#pragma once
#include "Point.h"

union Line {
	struct {
		Point p1;
		Point p2;
	};

	struct {
		float x1, y1;
		float x2, y2;
	};
};

float DistanceSquare(Point point1, Point point2);
float CityDistance(Point point1, Point point2);
float Distance(Point point1, Point point2);

float VectorLength(Point vector);
float VectorAngle(Point vector);

Point RotationVector(float angle);

Point PointDirection(Point startPoint, Point endPoint);

inline bool TurnsRight(Point point1, Point point2, Point point3) {
	float dx1 = point2.x - point1.x;
	float dy1 = point2.y - point1.y;
	float dx2 = point3.x - point2.x;
	float dy2 = point3.y - point2.y;

	float det = (dx1 * dy2) - (dx2 * dy1);

	return (det > 0.0f);
}

bool DoLinesCross(Point line11, Point line12, Point line21, Point line22);
Point LineIntersection(Point line11, Point line12, Point line21, Point line22);

float DotProduct(Point vector1, Point vector2);
Point NormalVector(Point vector);
Point ParallelVector(Point vector, Point base);

// TODO: create a Rect struct?
// TODO: use this everywhere
inline bool IsPointInRect(Point point, float left, float right, float top, float bottom) {
	bool result = true;

	if (point.x < left || point.x > right) result = false;
	else if (point.y < top || point.y > bottom) result = false;

	return result;
}

// TODO: create a Quad struct?
inline bool IsPointInQuad(Point quad[4], Point point) {
	if (point.x < quad[0].x && point.x < quad[1].x && point.x < quad[2].x && point.x < quad[3].x)
		return false;
	if (point.x > quad[0].x && point.x > quad[1].x && point.x > quad[2].x && point.x > quad[3].x)
		return false;

	if (point.y < quad[0].y && point.y < quad[1].y && point.y < quad[2].y && point.y < quad[3].y)
		return false;
	if (point.y > quad[0].y && point.y > quad[1].y && point.y > quad[2].y && point.y > quad[3].y)
		return false;

	if (!TurnsRight(quad[0], quad[1], point))
		return false;
	else if (!TurnsRight(quad[1], quad[2], point))
		return false;
	else if (!TurnsRight(quad[2], quad[3], point))
		return false;
	else if (!TurnsRight(quad[3], quad[0], point))
		return false;
	else
		return true;
}

inline Point TurnVectorToRight(Point vector) {
	Point result = {};
	result.x = -vector.y;
	result.y = vector.x;
	return result;
}

inline Point XYToBase(Point point, Point baseUnit) {
	float cosa = baseUnit.x;
	float sina = baseUnit.y;

	Point result = {};
	result.x = point.x * cosa + point.y * sina;
	result.y = -point.x * sina + point.y * cosa;

	return result;
}