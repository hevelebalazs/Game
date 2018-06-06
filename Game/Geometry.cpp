#include <math.h>

#include "Debug.hpp"
#include "Geometry.hpp"

// TODO: create a type that always represents a unit vector?

void Poly16Add(Poly16* poly, Point point)
{
	Assert(poly->pointN < 16);
	poly->points[poly->pointN] = point;
	poly->pointN++;
}

float DistanceSquare(Point point1, Point point2) {
	return (point1.x - point2.x) * (point1.x - point2.x) +
		(point1.y - point2.y) * (point1.y - point2.y);
}

float CityDistance(Point point1, Point point2) {
	return fabsf(point1.x - point2.x) + fabsf(point1.y - point2.y);
}

float Distance(Point point1, Point point2) {
	float dx = (point1.x - point2.x);
	float dy = (point1.y - point2.y);

	return sqrtf((dx * dx) + (dy * dy));
}

float VectorLength(Point vector) {
	return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
}

float VectorAngle(Point vector) {
	return atan2f(vector.y, vector.x);
}

Point RotationVector(float angle) {
	return Point{cosf(angle), sinf(angle)};
}

Point PointDirection(Point startPoint, Point endPoint) {
	Point vector = {endPoint.x - startPoint.x, endPoint.y - startPoint.y};
	vector = NormalVector(vector);

	return vector;
}

// TODO: can this be merged with LineIntersection?
bool DoLinesCross(Point line11, Point line12, Point line21, Point line22) {
	bool right1 = TurnsRight(line11, line21, line22);
	bool right2 = TurnsRight(line12, line21, line22);
	if (right1 == right2) 
		return false;

	bool right3 = TurnsRight(line21, line11, line12);
	bool right4 = TurnsRight(line22, line11, line12);
	if (right3 == right4) 
		return false;

	return true;
}

static float Determinant(float a, float b, float c, float d) {
	return (a * d) - (b * c);
}

Point LineIntersection(Point line11, Point line12, Point line21, Point line22) {
	float det1  = Determinant(line11.x, line11.y, line12.x, line12.y);
	float detX1 = Determinant(line11.x,     1.0f, line12.x,     1.0f);
	float detY1 = Determinant(line11.y,     1.0f, line12.y,     1.0f);

	float det2  = Determinant(line21.x, line21.y, line22.x, line22.y);
	float detX2 = Determinant(line21.x,     1.0f, line22.x,     1.0f);
	float detY2 = Determinant(line21.y,     1.0f, line22.y,     1.0f);

	float detXUp = Determinant(det1, detX1, det2, detX2);
	float detYUp = Determinant(det1, detY1, det2, detY2);

	detX1 = Determinant(line11.x, 1.0f, line12.x, 1.0f);
	detY1 = Determinant(line11.y, 1.0f, line12.y, 1.0f);
	detX2 = Determinant(line21.x, 1.0f, line22.x, 1.0f);
	detY2 = Determinant(line21.y, 1.0f, line22.y, 1.0f);

	float detDown = Determinant(detX1, detY1, detX2, detY2);

	Assert(detDown != 0.0f);
	if (detDown == 0.0f) 
		return Point{0.0f, 0.0f};
	else 
		return Point{(detXUp / detDown), (detYUp / detDown)};
}

Point LineIntersection(Line line1, Line line2) {
	Point intersection = LineIntersection(line1.p1, line1.p2, line2.p1, line2.p2);
	return intersection;
}

bool IsPointInPoly(Point point, Point* points, int pointN)
{
	bool isInside = false;
	if (pointN >= 3) {
		isInside = true;
		int prev = pointN - 1;
		for (int i = 0; i < pointN; ++i) {
			if (!TurnsRight(points[prev], points[i], point)) {
				isInside = false;
				break;
			}
			prev = i;
		}
	}
	return isInside;
}

float DotProduct(Point vector1, Point vector2) {
	return ((vector1.x * vector2.x) + (vector1.y * vector2.y));
}

Point NormalVector(Point vector) {
	float length = VectorLength(vector);

	if (length == 0.0f) 
		return vector;

	return 
		PointProd(1.0f / length, vector);
}

// TODO: create a version of this where base is unit length?
Point ParallelVector(Point vector, Point base) {
	Point unitBase = NormalVector(base);

	Point result = PointProd(DotProduct(vector, unitBase), unitBase);

	return result;
}