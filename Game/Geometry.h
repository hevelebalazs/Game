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

struct Quad {
	Point points[4];
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
inline bool IsPointInQuad(Quad quad, Point point) {
	Point* points = (Point*)quad.points;
	if (point.x < points[0].x && point.x < points[1].x && point.x < points[2].x && point.x < points[3].x)
		return false;
	if (point.x > points[0].x && point.x > points[1].x && point.x > points[2].x && point.x > points[3].x)
		return false;

	if (point.y < points[0].y && point.y < points[1].y && point.y < points[2].y && point.y < points[3].y)
		return false;
	if (point.y > points[0].y && point.y > points[1].y && point.y > points[2].y && point.y > points[3].y)
		return false;

	if (!TurnsRight(points[0], points[1], point))
		return false;
	else if (!TurnsRight(points[1], points[2], point))
		return false;
	else if (!TurnsRight(points[2], points[3], point))
		return false;
	else if (!TurnsRight(points[3], points[0], point))
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