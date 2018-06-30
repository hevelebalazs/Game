#pragma once

#include "Math.hpp"
#include "Type.hpp"

union Line {
	struct {
		V2 p1;
		V2 p2;
	};

	struct {
		F32 x1, y1;
		F32 x2, y2;
	};
};

struct Poly16 {
	V2 points[16];
	I32 pointN;
};

void Poly16Add(Poly16* poly, V2 point);

struct Quad {
	V2 points[4];
};

V2 MakePoint(F32 x, F32 y);
Quad MakeQuad(V2 point1, V2 point2, V2 point3, V2 point4);

F32 DistanceSquare(V2 point1, V2 point2);
F32 CityDistance(V2 point1, V2 point2);
F32 Distance(V2 point1, V2 point2);

F32 VectorLength(V2 vector);
F32 VectorAngle(V2 vector);

inline F32 LineAngle(V2 startPoint, V2 endPoint)
{
	V2 diff = (endPoint - startPoint);
	F32 angle = VectorAngle(diff);
	return angle;
}


inline F32 NormalizeAngle(F32 angle)
{
	while (angle > PI)
		angle -= 2.0f * PI;
	while (angle < -PI)
		angle += 2.0f * PI;
	return angle;
}

inline B32 IsAngleBetween(F32 minAngle, F32 angle, F32 maxAngle)
{
	B32 result = false;
	if (minAngle > maxAngle)
		result = (angle <= maxAngle || angle >= minAngle);
	else
		result = (minAngle <= angle && angle <= maxAngle);
	return result;
}

V2 RotationVector(F32 angle);
V2 PointDirection(V2 startPoint, V2 endPoint);

inline B32 TurnsRight(V2 point1, V2 point2, V2 point3)
{
	F32 dx1 = point2.x - point1.x;
	F32 dy1 = point2.y - point1.y;
	F32 dx2 = point3.x - point2.x;
	F32 dy2 = point3.y - point2.y;

	F32 det = (dx1 * dy2) - (dx2 * dy1);

	return (det > 0.0f);
}

B32 DoLinesCross(V2 line11, V2 line12, V2 line21, V2 line22);
V2 LineIntersection(V2 line11, V2 line12, V2 line21, V2 line22);
V2 LineIntersection(Line line1, Line line2);

F32 DotProduct(V2 vector1, V2 vector2);
V2 NormalVector(V2 vector);
V2 ParallelVector(V2 vector, V2 base);

B32 IsPointInPoly(V2 point, V2* points, I32 pointN);

// TODO: create a Quad struct?
inline B32 IsPointInQuad(Quad quad, V2 point)
{
	V2* points = quad.points;
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

inline V2 TurnVectorToRight(V2 vector)
{
	V2 result = {};
	result.x = -vector.y;
	result.y = vector.x;
	return result;
}

inline V2 XYToBase(V2 point, V2 baseUnit)
{
	F32 cosa = baseUnit.x;
	F32 sina = baseUnit.y;

	V2 result = {};
	result.x = point.x * cosa + point.y * sina;
	result.y = -point.x * sina + point.y * cosa;

	return result;
}